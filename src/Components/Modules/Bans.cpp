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
			for (auto& idEntry : list.idList)
			{
				if (idEntry.Bits == entry.first.Bits)
				{
					return true;
				}
			}
		}

		if (entry.second.full)
		{
			for (auto& ipEntry : list.ipList)
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

		std::lock_guard<std::mutex> _(Bans::AccessMutex);

		if (entry.first.Bits)
		{
			bool found = false;
			for (auto& idEntry : list.idList)
			{
				if (idEntry.Bits == entry.first.Bits)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				list.idList.push_back(entry.first);
			}
		}

		if (entry.second.full)
		{
			bool found = false;
			for (auto& ipEntry : list.ipList)
			{
				if (ipEntry.full == entry.second.full)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				list.ipList.push_back(entry.second);
			}
		}

		std::vector<std::string> idVector;
		std::vector<std::string> ipVector;

		for (auto& idEntry : list.idList)
		{
			idVector.push_back(Utils::String::VA("%llX", idEntry.Bits));
		}

		for (auto& ipEntry : list.ipList)
		{
			ipVector.push_back(Utils::String::VA("%u.%u.%u.%u",
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
		ban.write(bans.dump());
	}

	void Bans::LoadBans(Bans::BanList* list)
	{
		std::lock_guard<std::mutex> _(Bans::AccessMutex);

		FileSystem::File bans("bans.json");

		if (bans.exists())
		{
			std::string error;
			json11::Json banData = json11::Json::parse(bans.getBuffer(), error);

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

							list->idList.push_back(id);
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

							list->ipList.push_back(addr.getIP());
						}
					}
				}
			}
		}
	}

	void Bans::BanClientNum(int num, std::string reason)
	{
		if (!Dvar::Var("sv_running").get<bool>())
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
		Command::Add("banclient", [] (Command::Params* params)
		{
			if (params->length() < 2) return;

			std::string reason = "EXE_ERR_BANNED_PERM";
			if (params->length() >= 3) reason = params->join(2);

			Bans::BanClientNum(atoi(params->get(1)), reason);
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
