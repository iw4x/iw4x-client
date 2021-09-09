#include "STDInclude.hpp"

namespace Components
{
	std::recursive_mutex Bans::AccessMutex;

	bool Bans::IsBanned(Bans::Entry entry)
	{
		Bans::BanList list;
		Bans::LoadBans(&list);

		if (entry.first.bits)
		{
			for (auto& idEntry : list.idList)
			{
				if (idEntry.bits == entry.first.bits)
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
		std::lock_guard<std::recursive_mutex> _(Bans::AccessMutex);

		Bans::BanList list;
		Bans::LoadBans(&list);

		if (entry.first.bits)
		{
			bool found = false;
			for (auto& idEntry : list.idList)
			{
				if (idEntry.bits == entry.first.bits)
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

		Bans::SaveBans(&list);
	}

	void Bans::SaveBans(BanList* list)
	{
		std::lock_guard<std::recursive_mutex> _(Bans::AccessMutex);

		std::vector<std::string> idVector;
		std::vector<std::string> ipVector;

		for (auto& idEntry : list->idList)
		{
			idVector.push_back(Utils::String::VA("%llX", idEntry.bits));
		}

		for (auto& ipEntry : list->ipList)
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
		std::lock_guard<std::recursive_mutex> _(Bans::AccessMutex);

		FileSystem::File bans("bans.json");

		if (bans.exists())
		{
			std::string error;
			json11::Json banData = json11::Json::parse(bans.getBuffer(), error);

			if (!error.empty())
			{
				Logger::Error("Failed to parse bans (bans.json): %s", error.data());
			}

			if (!list) return;

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
							id.bits = strtoull(idEntry.string_value().data(), nullptr, 16);

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

	void Bans::BanClientNum(int num, const std::string& reason)
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
		guid.bits = client->steamID;

		Bans::InsertBan({ guid, client->netchan.remoteAddress.ip });

		Game::SV_KickClientError(client, reason);
	}

	void Bans::UnbanClient(SteamID id)
	{
		std::lock_guard<std::recursive_mutex> _(Bans::AccessMutex);

		Bans::BanList list;
		Bans::LoadBans(&list);

		auto entry = std::find_if(list.idList.begin(), list.idList.end(), [&id](SteamID& entry)
		{
			return id.bits == entry.bits;
		});

		if (entry != list.idList.end())
		{
			list.idList.erase(entry);
		}

		Bans::SaveBans(&list);
	}

	void Bans::UnbanClient(Game::netIP_t ip)
	{
		std::lock_guard<std::recursive_mutex> _(Bans::AccessMutex);

		Bans::BanList list;
		Bans::LoadBans(&list);

		auto entry = std::find_if(list.ipList.begin(), list.ipList.end(), [&ip](Game::netIP_t& entry)
		{
			return ip.full == entry.full;
		});

		if (entry != list.ipList.end())
		{
			list.ipList.erase(entry);
		}

		Bans::SaveBans(&list);
	}

	Bans::Bans()
	{
		Command::Add("banclient", [](Command::Params* params)
		{
			if (params->length() < 2) return;

			std::string reason = "EXE_ERR_BANNED_PERM";
			if (params->length() >= 3) reason = params->join(2);

			Bans::BanClientNum(atoi(params->get(1)), reason);
		});

		Command::Add("unbanclient", [](Command::Params* params)
		{
			if (params->length() < 2) return;

			std::string type = params->get(1);

			if (type == "ip"s)
			{
				Network::Address address(params->get(2));
				Bans::UnbanClient(address.getIP());

				Logger::Print("Unbanned IP %s\n", params->get(2));

			}
			else if (type == "guid"s)
			{
				SteamID id;
				id.bits = strtoull(params->get(2), nullptr, 16);

				Bans::UnbanClient(id);

				Logger::Print("Unbanned GUID %s\n", params->get(2));
			}
		});

		// Verify the list on startup
		Scheduler::Once([]()
		{
			Bans::BanList list;
			Bans::LoadBans(&list);
		});
	}

	Bans::~Bans()
	{

	}
}
