#include "STDInclude.hpp"

namespace Components
{
	std::mutex Bans::AccessMutex;

	bool Bans::IsBanned(Bans::Entry entry)
	{
		Bans::BanList list;
		Bans::LoadBans(&list);

		if (entry.first.Bits)
		{
			for (auto& idEntry : list.IDList)
			{
				if (idEntry.Bits == entry.first.Bits)
				{
					return true;
				}
			}
		}

		if (entry.second.full)
		{
			for (auto& ipEntry : list.IPList)
			{
				if (ipEntry.full == entry.second.full)
				{
					return true;
				}
			}
		}

		return false;
	}

	void Bans::InsertBan(Bans::Entry entry)
	{
		Bans::BanList list;
		Bans::LoadBans(&list);

		Bans::AccessMutex.lock();

		if (entry.first.Bits)
		{
			bool found = false;
			for (auto& idEntry : list.IDList)
			{
				if (idEntry.Bits == entry.first.Bits)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				list.IDList.push_back(entry.first);
			}
		}

		if (entry.second.full)
		{
			bool found = false;
			for (auto& ipEntry : list.IPList)
			{
				if (ipEntry.full == entry.second.full)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				list.IPList.push_back(entry.second);
			}
		}

		std::vector<std::string> idVector;
		std::vector<std::string> ipVector;

		for (auto& idEntry : list.IDList)
		{
			idVector.push_back(fmt::sprintf("%llX", idEntry.Bits));
		}

		for (auto& ipEntry : list.IPList)
		{
			ipVector.push_back(fmt::sprintf("%u.%u.%u.%u",
				ipEntry.bytes[0] & 0xFF,
				ipEntry.bytes[1] & 0xFF, 
				ipEntry.bytes[2] & 0xFF, 
				ipEntry.bytes[3] & 0xFF));
		}

		json11::Json bans = json11::Json::object
		{
			{ "ip", ipVector },
			{ "id", idVector },
		};
		
		FileSystem::FileWriter ban("bans.json");
		ban.Write(bans.dump());

		Bans::AccessMutex.unlock();
	}

	void Bans::LoadBans(Bans::BanList* list)
	{
		Bans::AccessMutex.lock();

		// TODO: Read bans
		FileSystem::File bans("bans.json");

		if (bans.Exists())
		{
			std::string error;
			json11::Json banData = json11::Json::parse(bans.GetBuffer(), error);

			if (!error.empty())
			{
				Logger::Error("Failed to parse bans (bans.json): %s", error.data());
			}

			if (!list)
			{
				Bans::AccessMutex.unlock();
				return;
			}

			if (banData.is_object())
			{
				auto idList = banData["id"];
				auto ipList = banData["ip"];

				if (idList.is_array())
				{
					for (auto &idEntry : idList.array_items())
					{
						if (idEntry.is_string())
						{
							SteamID id;
							id.Bits = strtoull(idEntry.string_value().data(), nullptr, 16);
							
							list->IDList.push_back(id);
						}
					}
				}

				if (ipList.is_array())
				{
					for (auto &ipEntry : ipList.array_items())
					{
						if (ipEntry.is_string())
						{
							Network::Address addr(ipEntry.string_value());

							list->IPList.push_back(addr.GetIP());
						}
					}
				}
			}
		}

		Bans::AccessMutex.unlock();
	}

	void Bans::BanClientNum(int num, std::string reason)
	{
		if (!Dvar::Var("sv_running").Get<bool>())
		{
			Logger::Print("Server is not running.\n");
			return;
		}

		if (*Game::svs_numclients <= num)
		{
			Logger::Print("Player %d is not on the server\n", num);
			return;
		}

		Game::client_t* client = &Game::svs_clients[num];

		SteamID guid;
		guid.Bits = client->steamid;

		Bans::InsertBan({ guid, client->addr.ip });

		Game::SV_KickClientError(client, reason);
	}

	Bans::Bans()
	{
		Command::Add("banclient", [] (Command::Params params)
		{
			if (params.Length() < 2) return;

			std::string reason = "EXE_ERR_BANNED_PERM";
			if (params.Length() >= 3) reason = params[2];

			Bans::BanClientNum(atoi(params[1]), reason);
		});

		// Verify the list on startup
		QuickPatch::Once([] ()
		{
			Bans::BanList list;
			Bans::LoadBans(&list);
		});
	}

	Bans::~Bans()
	{

	}
}
