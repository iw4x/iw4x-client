#include "STDInclude.hpp"

namespace Components
{
	bool ServerList::SortAsc = true;
	int ServerList::SortKey = ServerList::Column::Ping;

	unsigned int ServerList::CurrentServer = 0;
	ServerList::Container ServerList::RefreshContainer;

	std::vector<ServerList::ServerInfo> ServerList::OnlineList;
	std::vector<ServerList::ServerInfo> ServerList::OfflineList;
	std::vector<ServerList::ServerInfo> ServerList::FavouriteList;

	std::vector<unsigned int> ServerList::VisibleList;

	std::vector<ServerList::ServerInfo>* ServerList::GetList()
	{
		if (ServerList::IsOnlineList())
		{
			return &ServerList::OnlineList;
		}
		else if (ServerList::IsOfflineList())
		{
			return &ServerList::OfflineList;
		}
		else if (ServerList::IsFavouriteList())
		{
			return &ServerList::FavouriteList;
		}

		return nullptr;
	}

	bool ServerList::IsFavouriteList()
	{
		return (Dvar::Var("ui_netSource").get<int>() == 2);
	}

	bool ServerList::IsOfflineList()
	{
		return (Dvar::Var("ui_netSource").get<int>() == 0);
	}

	bool ServerList::IsOnlineList()
	{
		return (Monitor::IsEnabled() || Dvar::Var("ui_netSource").get<int>() == 1);
	}

	unsigned int ServerList::GetServerCount()
	{
		return ServerList::VisibleList.size();
	}

	const char* ServerList::GetServerText(unsigned int index, int column)
	{
		ServerList::ServerInfo* info = ServerList::GetServer(index);

		if (info)
		{
			return ServerList::GetServerInfoText(info, column);
		}

		return "";
	}

	const char* ServerList::GetServerInfoText(ServerList::ServerInfo* server, int column, bool sorting)
	{
		if (!server) return "";

		switch (column)
		{
		case Column::Password:
		{
			return (server->password ? "X" : "");
		}

		case Column::Matchtype:
		{
			return ((server->matchType == 1) ? "P" : "M");
		}

		case Column::Hostname:
		{
			return server->hostname.data();
		}

		case Column::Mapname:
		{
			if (server->svRunning)
			{
				if (!sorting && !Maps::CheckMapInstalled(server->mapname.data()))
				{
					return Utils::String::VA("^1%s", Game::UI_LocalizeMapName(server->mapname.data()));
				}
				return Game::UI_LocalizeMapName(server->mapname.data());
			}
			else
			{
				return Utils::String::VA("^3%s", Game::UI_LocalizeMapName(server->mapname.data()));
			}
		}

		case Column::Players:
		{
			return Utils::String::VA("%i/%i (%i)", server->clients, server->maxClients, server->bots);
		}

		case Column::Gametype:
		{
			return Game::UI_LocalizeGameType(server->gametype.data());
		}

		case Column::Mod:
		{
			if (server->mod != "")
			{
				return (server->mod.data() + 5);
			}

			return "";
		}

		case Column::Ping:
		{
			if (server->ping < 75) // Below this is a good ping
			{
				return Utils::String::VA("^2%i", server->ping);
			}
			else if (server->ping < 150) // Below this is a medium ping
			{
				return Utils::String::VA("^3%i", server->ping);
			}
			else
			{
				return Utils::String::VA("^1%i", server->ping);
			}
		}

		default:
		{
			break;
		};
		}

		return "";
	}

	void ServerList::SelectServer(unsigned int index)
	{
		ServerList::CurrentServer = index;

		ServerList::ServerInfo* info = ServerList::GetCurrentServer();

		if (info)
		{
			Dvar::Var("ui_serverSelected").set(true);
			Dvar::Var("ui_serverSelectedMap").set(info->mapname);
			Dvar::Var("ui_serverSelectedGametype").set(info->gametype);
		}
		else
		{
			Dvar::Var("ui_serverSelected").set(false);
		}
	}

	void ServerList::UpdateVisibleList(UIScript::Token)
	{
		auto list = ServerList::GetList();
		if (!list) return;

		std::vector<ServerList::ServerInfo> tempList(*list);

		if (tempList.empty())
		{
			ServerList::Refresh(UIScript::Token());
		}
		else
		{
			list->clear();

			std::lock_guard<std::recursive_mutex> _(ServerList::RefreshContainer.mutex);

			ServerList::RefreshContainer.sendCount = 0;
			ServerList::RefreshContainer.sentCount = 0;

			for (auto& server : tempList)
			{
				ServerList::InsertRequest(server.addr);
			}
		}
	}

	void ServerList::RefreshVisibleList(UIScript::Token)
	{
		Dvar::Var("ui_serverSelected").set(false);

		ServerList::VisibleList.clear();

		auto list = ServerList::GetList();
		if (!list) return;

		// Refresh entirely, if there is no entry in the list
		if (list->empty())
		{
			ServerList::Refresh(UIScript::Token());
			return;
		}

		bool ui_browserShowFull     = Dvar::Var("ui_browserShowFull").get<bool>();
		bool ui_browserShowEmpty    = Dvar::Var("ui_browserShowEmpty").get<bool>();
		int ui_browserShowHardcore  = Dvar::Var("ui_browserKillcam").get<int>();
		int ui_browserShowPassword  = Dvar::Var("ui_browserShowPassword").get<int>();
		int ui_browserMod           = Dvar::Var("ui_browserMod").get<int>();
		int ui_joinGametype         = Dvar::Var("ui_joinGametype").get<int>();

		for (unsigned int i = 0; i < list->size(); ++i)
		{
			ServerList::ServerInfo* info = &(*list)[i];

			// Filter full servers
			if (!ui_browserShowFull && info->clients >= info->maxClients) continue;

			// Filter empty servers
			if (!ui_browserShowEmpty && info->clients <= 0) continue;

			// Filter hardcore servers
			if ((ui_browserShowHardcore == 0 && info->hardcore) || (ui_browserShowHardcore == 1 && !info->hardcore)) continue;

			// Filter servers with password
			if ((ui_browserShowPassword == 0 && info->password) || (ui_browserShowPassword == 1 && !info->password)) continue;

			// Don't show modded servers
			if ((ui_browserMod == 0 && info->mod.size()) || (ui_browserMod == 1 && !info->mod.size())) continue;

			// Filter by gametype
			if (ui_joinGametype > 0 && (ui_joinGametype - 1) < *Game::gameTypeCount  && Game::gameTypes[(ui_joinGametype - 1)].gameType != info->gametype) continue;

			ServerList::VisibleList.push_back(i);
		}

		ServerList::SortList();
	}

	void ServerList::Refresh(UIScript::Token)
	{
		Dvar::Var("ui_serverSelected").set(false);
		//Localization::Set("MPUI_SERVERQUERIED", "Sent requests: 0/0");

// 		ServerList::OnlineList.clear();
// 		ServerList::OfflineList.clear();
// 		ServerList::FavouriteList.clear();

		auto list = ServerList::GetList();
		if (list) list->clear();

		ServerList::VisibleList.clear();

		{
			std::lock_guard<std::recursive_mutex> _(ServerList::RefreshContainer.mutex);
			ServerList::RefreshContainer.servers.clear();
			ServerList::RefreshContainer.sendCount = 0;
			ServerList::RefreshContainer.sentCount = 0;
		}

		if (ServerList::IsOfflineList())
		{
			Discovery::Perform();
		}
		else if (ServerList::IsOnlineList())
		{
#ifdef USE_LEGACY_SERVER_LIST
			ServerList::RefreshContainer.awatingList = true;
			ServerList::RefreshContainer.awaitTime = Game::Sys_Milliseconds();

			int masterPort = Dvar::Var("masterPort").get<int>();
			const char* masterServerName = Dvar::Var("masterServerName").get<const char*>();

			ServerList::RefreshContainer.host = Network::Address(Utils::String::VA("%s:%u", masterServerName, masterPort));

			Logger::Print("Sending serverlist request to master: %s:%u\n", masterServerName, masterPort);

			Network::SendCommand(ServerList::RefreshContainer.host, "getservers", Utils::String::VA("IW4 %i full empty", PROTOCOL));
			//Network::SendCommand(ServerList::RefreshContainer.Host, "getservers", "0 full empty");
#else
			Node::Synchronize();
#endif
		}
		else if (ServerList::IsFavouriteList())
		{
			ServerList::LoadFavourties();
		}
	}

	void ServerList::StoreFavourite(const std::string& server)
	{
		//json11::Json::parse()
		std::vector<std::string> servers;

		if (Utils::IO::FileExists("players/favourites.json"))
		{
			std::string data = Utils::IO::ReadFile("players/favourites.json");
			json11::Json object = json11::Json::parse(data, data);

			if (!object.is_array())
			{
				Logger::Print("Favourites storage file is invalid!\n");
				Game::ShowMessageBox("Favourites storage file is invalid!", "Error");
				return;
			}

			auto storedServers = object.array_items();

			for (unsigned int i = 0; i < storedServers.size(); ++i)
			{
				if (!storedServers[i].is_string()) continue;
				if (storedServers[i].string_value() == server)
				{
					Game::ShowMessageBox("Server already marked as favourite.", "Error");
					return;
				}

				servers.push_back(storedServers[i].string_value());
			}
		}

		servers.push_back(server);

		json11::Json data = json11::Json(servers);
		Utils::IO::WriteFile("players/favourites.json", data.dump());
		Game::ShowMessageBox("Server added to favourites.", "Success");
	}

	void ServerList::RemoveFavourite(const std::string& server)
	{
		std::vector<std::string> servers;

		if (Utils::IO::FileExists("players/favourites.json"))
		{
			std::string data = Utils::IO::ReadFile("players/favourites.json");
			json11::Json object = json11::Json::parse(data, data);

			if (!object.is_array())
			{
				Logger::Print("Favourites storage file is invalid!\n");
				Game::ShowMessageBox("Favourites storage file is invalid!", "Error");
				return;
			}

			for (auto& storedServer : object.array_items())
			{
				if (storedServer.is_string() && storedServer.string_value() != server)
				{
					servers.push_back(storedServer.string_value());
				}
			}
		}

		json11::Json data = json11::Json(servers);
		Utils::IO::WriteFile("players/favourites.json", data.dump());

		auto list = ServerList::GetList();
		if (list) list->clear();
		
		ServerList::RefreshVisibleList(UIScript::Token());
		
		Game::ShowMessageBox("Server removed from favourites.", "Success");
	}

	void ServerList::LoadFavourties()
	{
		if (ServerList::IsFavouriteList() && Utils::IO::FileExists("players/favourites.json"))
		{
			auto list = ServerList::GetList();
			if (list) list->clear();

			std::string data = Utils::IO::ReadFile("players/favourites.json");
			json11::Json object = json11::Json::parse(data, data);

			if (!object.is_array())
			{
				Logger::Print("Favourites storage file is invalid!\n");
				Game::ShowMessageBox("Favourites storage file is invalid!", "Error");
				return;
			}

			auto servers = object.array_items();

			for (unsigned int i = 0; i < servers.size(); ++i)
			{
				if (!servers[i].is_string()) continue;
				ServerList::InsertRequest(servers[i].string_value());
			}
		}
	}

	void ServerList::InsertRequest(Network::Address address)
	{
		std::lock_guard<std::recursive_mutex> _(ServerList::RefreshContainer.mutex);

		ServerList::Container::ServerContainer container;
		container.sent = false;
		container.target = address;

		bool alreadyInserted = false;
		for (auto &server : ServerList::RefreshContainer.servers)
		{
			if (server.target == container.target)
			{
				alreadyInserted = true;
				break;
			}
		}

		if (!alreadyInserted)
		{
			ServerList::RefreshContainer.servers.push_back(container);

			auto list = ServerList::GetList();
			if (list)
			{
				for (auto& server : *list)
				{
					if (server.addr == container.target)
					{
						--ServerList::RefreshContainer.sendCount;
						--ServerList::RefreshContainer.sentCount;
						break;
					}
				}
			}

			++ServerList::RefreshContainer.sendCount;
		}
	}

	void ServerList::Insert(Network::Address address, Utils::InfoString info)
	{
		std::lock_guard<std::recursive_mutex> _(ServerList::RefreshContainer.mutex);

		for (auto i = ServerList::RefreshContainer.servers.begin(); i != ServerList::RefreshContainer.servers.end();)
		{
			// Our desired server
			if (i->target == address && i->sent)
			{
				// Challenge did not match
				if (i->challenge != info.get("challenge"))
				{
					// Shall we remove the server from the queue?
					// Better not, it might send a second response with the correct challenge.
					// This might happen when users refresh twice (or more often) in a short period of time
					break;
				}

				ServerInfo server;
				server.hostname = info.get("hostname");
				server.mapname = info.get("mapname");
				server.gametype = info.get("gametype");
				server.shortversion = info.get("shortversion");
				server.mod = info.get("fs_game");
				server.matchType = atoi(info.get("matchtype").data());
				server.clients = atoi(info.get("clients").data());
				server.bots = atoi(info.get("bots").data());
				server.securityLevel = atoi(info.get("securityLevel").data());
				server.maxClients = atoi(info.get("sv_maxclients").data());
				server.password = (atoi(info.get("isPrivate").data()) != 0);
				server.hardcore = (atoi(info.get("hc").data()) != 0);
				server.svRunning = (atoi(info.get("sv_running").data()) != 0);
				server.ping = (Game::Sys_Milliseconds() - i->sendTime);
				server.addr = address;

				server.hostname = TextRenderer::StripMaterialTextIcons(server.hostname);
				server.mapname = TextRenderer::StripMaterialTextIcons(server.mapname);
				server.gametype = TextRenderer::StripMaterialTextIcons(server.gametype);
				server.mod = TextRenderer::StripMaterialTextIcons(server.mod);

				// Remove server from queue
				i = ServerList::RefreshContainer.servers.erase(i);

				// Servers with more than 18 players or less than 0 players are faking for sure
				// So lets ignore those
				if (server.clients > 18 || server.maxClients > 18 || server.clients < 0 || server.maxClients < 0)
					return;

				// Check if already inserted and remove
				auto list = ServerList::GetList();
				if (!list) return;

				unsigned int k = 0;
				for (auto j = list->begin(); j != list->end(); ++k)
				{
					if (j->addr == address)
					{
						j = list->erase(j);
					}
					else
					{
						++j;
					}
				}

				// Also remove from visible list
				for (auto j = ServerList::VisibleList.begin(); j != ServerList::VisibleList.end();)
				{
					if (*j == k)
					{
						j = ServerList::VisibleList.erase(j);
					}
					else
					{
						++j;
					}
				}

				if (info.get("gamename") == "IW4"
					&& server.matchType
#if !defined(DEBUG) && defined(VERSION_FILTER)
					&& ServerList::CompareVersion(server.shortversion, SHORTVERSION)
#endif
					)
				{
					auto lList = ServerList::GetList();

					if (lList)
					{
						lList->push_back(server);
						ServerList::RefreshVisibleList(UIScript::Token());
					}
				}

				break;
			}
			else
			{
				++i;
			}
		}
	}

	bool ServerList::CompareVersion(const std::string& version1, const std::string& version2)
	{
		std::vector<std::string> subVersions1 = Utils::String::Explode(version1, '.');
		std::vector<std::string> subVersions2 = Utils::String::Explode(version2, '.');

		while (subVersions1.size() >= 3) subVersions1.pop_back();
		while (subVersions2.size() >= 3) subVersions2.pop_back();
		if (subVersions1.size() != subVersions2.size()) return false;

		for (unsigned int i = 0; i < subVersions1.size(); ++i)
		{
			if (atoi(subVersions1[i].data()) != atoi(subVersions2[i].data()))
			{
				return false;
			}
		}

		return true;
	}

	ServerList::ServerInfo* ServerList::GetCurrentServer()
	{
		return ServerList::GetServer(ServerList::CurrentServer);
	}

	void ServerList::SortList()
	{
		// Only sort when the serverlist is open
		Game::menuDef_t* menu = Game::Menus_FindByName(Game::uiContext, "pc_join_unranked");
		if (!menu || !Game::Menu_IsVisible(Game::uiContext, menu)) return;

		std::stable_sort(ServerList::VisibleList.begin(), ServerList::VisibleList.end(), [](const unsigned int &server1, const unsigned int &server2) -> bool
		{
			ServerInfo* info1 = nullptr;
			ServerInfo* info2 = nullptr;

			auto list = ServerList::GetList();
			if (!list) return false;

			if (list->size() > server1) info1 = &(*list)[server1];
			if (list->size() > server2) info2 = &(*list)[server2];

			if (!info1) return false;
			if (!info2) return false;

			// Numerical comparisons
			if (ServerList::SortKey == ServerList::Column::Ping)
			{
				return info1->ping < info2->ping;
			}
			else if (ServerList::SortKey == ServerList::Column::Players)
			{
				return info1->clients < info2->clients;
			}

			std::string text1 = Utils::String::ToLower(TextRenderer::StripColors(ServerList::GetServerInfoText(info1, ServerList::SortKey, true)));
			std::string text2 = Utils::String::ToLower(TextRenderer::StripColors(ServerList::GetServerInfoText(info2, ServerList::SortKey, true)));

			// ASCII-based comparison
			return text1.compare(text2) < 0;
		});

		if (!ServerList::SortAsc) std::reverse(ServerList::VisibleList.begin(), ServerList::VisibleList.end());
	}

	ServerList::ServerInfo* ServerList::GetServer(unsigned int index)
	{
		if (ServerList::VisibleList.size() > index)
		{
			auto list = ServerList::GetList();
			if (!list) return nullptr;

			if (list->size() > ServerList::VisibleList[index])
			{
				return &(*list)[ServerList::VisibleList[index]];
			}
		}

		return nullptr;
	}

	void ServerList::Frame()
	{
		static Utils::Time::Interval frameLimit;
		int interval = static_cast<int>(1000.0f / Dvar::Var("net_serverFrames").get<int>());
		if (!frameLimit.elapsed(std::chrono::milliseconds(interval))) return;
		frameLimit.update();

		std::lock_guard<std::recursive_mutex> _(ServerList::RefreshContainer.mutex);

		if (ServerList::RefreshContainer.awatingList)
		{
			// Check if we haven't got a response within 10 seconds
			if (Game::Sys_Milliseconds() - ServerList::RefreshContainer.awaitTime > 5000)
			{
				ServerList::RefreshContainer.awatingList = false;

				Logger::Print("We haven't received a response from the master within %d seconds!\n", (Game::Sys_Milliseconds() - ServerList::RefreshContainer.awaitTime) / 1000);
			}
		}

		int requestLimit = Dvar::Var("net_serverQueryLimit").get<int>();
		for (unsigned int i = 0; i < ServerList::RefreshContainer.servers.size() && requestLimit > 0; ++i)
		{
			ServerList::Container::ServerContainer* server = &ServerList::RefreshContainer.servers[i];
			if (server->sent) continue;

			// Found server we can send a request to
			server->sent = true;
			requestLimit--;

			server->sendTime = Game::Sys_Milliseconds();
			server->challenge = Utils::Cryptography::Rand::GenerateChallenge();

			++ServerList::RefreshContainer.sentCount;

			Network::SendCommand(server->target, "getinfo", server->challenge);

			// Display in the menu, like in COD4
			//Localization::Set("MPUI_SERVERQUERIED", Utils::String::VA("Sent requests: %d/%d", ServerList::RefreshContainer.sentCount, ServerList::RefreshContainer.sendCount));
		}

		ServerList::UpdateVisibleInfo();
	}

	void ServerList::UpdateSource()
	{
		Dvar::Var netSource("ui_netSource");

		int source = netSource.get<int>();

		if (++source > netSource.get<Game::dvar_t*>()->domain.integer.max)
		{
			source = 0;
		}

		netSource.set(source);

		ServerList::RefreshVisibleList(UIScript::Token());
	}

	void ServerList::UpdateGameType()
	{
		Dvar::Var joinGametype("ui_joinGametype");

		int gametype = joinGametype.get<int>();

		if (++gametype > *Game::gameTypeCount)
		{
			gametype = 0;
		}

		joinGametype.set(gametype);

		ServerList::RefreshVisibleList(UIScript::Token());
	}

	void ServerList::UpdateVisibleInfo()
	{
		static int servers = 0;
		static int players = 0;
		static int bots = 0;

		auto list = ServerList::GetList();

		if (list)
		{
			int newSevers = list->size();
			int newPlayers = 0;
			int newBots = 0;

			for (unsigned int i = 0; i < list->size(); ++i)
			{
				newPlayers += list->at(i).clients;
				newBots += list->at(i).bots;
			}

			if (newSevers != servers || newPlayers != players || newBots != bots)
			{
				servers = newSevers;
				players = newPlayers;
				bots = newBots;

				Localization::Set("MPUI_SERVERQUERIED", Utils::String::VA("Servers: %i\nPlayers: %i (%i)", servers, players, bots));
			}
		}
	}

	ServerList::ServerList()
	{
		ServerList::OnlineList.clear();
		ServerList::OfflineList.clear();
		ServerList::FavouriteList.clear();
		ServerList::VisibleList.clear();

		Dvar::OnInit([]()
		{
			Dvar::Register<bool>("ui_serverSelected", false, Game::dvar_flag::DVAR_FLAG_NONE, "Whether a server has been selected in the serverlist");
			Dvar::Register<const char*>("ui_serverSelectedMap", "mp_afghan", Game::dvar_flag::DVAR_FLAG_NONE, "Map of the selected server");

			Dvar::Register<int>("net_serverQueryLimit", 1, 1, 10, Dedicated::IsEnabled() ? 0 : Game::dvar_flag::DVAR_FLAG_SAVED, "Amount of server queries per frame");
			Dvar::Register<int>("net_serverFrames", 30, 1, 60, Dedicated::IsEnabled() ? 0 : Game::dvar_flag::DVAR_FLAG_SAVED, "Amount of server query frames per second");
		});

		// Fix ui_netsource dvar
		Utils::Hook::Nop(0x4CDEEC, 5); // Don't reset the netsource when gametypes aren't loaded
		Dvar::Register<int>("ui_netSource", 1, 0, 2, Game::DVAR_FLAG_SAVED, reinterpret_cast<const char*>(0x6D9F08));

		//Localization::Set("MPUI_SERVERQUERIED", "Sent requests: 0/0");
		Localization::Set("MPUI_SERVERQUERIED", "Servers: 0\nPlayers: 0 (0)");

		Network::Handle("getServersResponse", [](Network::Address address, const std::string& data)
		{
			if (ServerList::RefreshContainer.host != address) return; // Only parse from host we sent to

			ServerList::RefreshContainer.awatingList = false;

			std::lock_guard<std::recursive_mutex> _(ServerList::RefreshContainer.mutex);

			int offset = 0;
			int count = ServerList::RefreshContainer.servers.size();
			ServerList::MasterEntry* entry = nullptr;

			// Find first entry
			do
			{
				entry = reinterpret_cast<ServerList::MasterEntry*>(const_cast<char*>(data.data()) + offset++);
			} while (!entry->HasSeparator() && !entry->IsEndToken());

			for (int i = 0; !entry[i].IsEndToken() && entry[i].HasSeparator(); ++i)
			{
				Network::Address serverAddr = address;
				serverAddr.setIP(entry[i].ip);
				serverAddr.setPort(ntohs(entry[i].port));
				serverAddr.setType(Game::NA_IP);

				ServerList::InsertRequest(serverAddr);
			}

			Logger::Print("Parsed %d servers from master\n", ServerList::RefreshContainer.servers.size() - count);
		});

		// Set default masterServerName + port and save it 
#ifdef USE_LEGACY_SERVER_LIST
		Utils::Hook::Set<char*>(0x60AD92, "127.0.0.1");
		Utils::Hook::Set<BYTE>(0x60AD90, Game::dvar_flag::DVAR_FLAG_SAVED); // masterServerName
		Utils::Hook::Set<BYTE>(0x60ADC6, Game::dvar_flag::DVAR_FLAG_SAVED); // masterPort
#endif

		// Add server list feeder
		UIFeeder::Add(2.0f, ServerList::GetServerCount, ServerList::GetServerText, ServerList::SelectServer);

		// Add required UIScripts
		UIScript::Add("UpdateFilter", ServerList::RefreshVisibleList);
		UIScript::Add("RefreshFilter", ServerList::UpdateVisibleList);

		UIScript::Add("RefreshServers", ServerList::Refresh);
		UIScript::Add("JoinServer", [](UIScript::Token)
		{
			ServerList::ServerInfo* info = ServerList::GetServer(ServerList::CurrentServer);

			if (info)
			{
				Party::Connect(info->addr);
			}
		});
		UIScript::Add("ServerSort", [](UIScript::Token token)
		{
			int key = token.get<int>();

			if (ServerList::SortKey == key)
			{
				ServerList::SortAsc = !ServerList::SortAsc;
			}
			else
			{
				ServerList::SortKey = key;
				ServerList::SortAsc = true;
			}

			Logger::Print("Sorting server list by token: %d\n", ServerList::SortKey);
			ServerList::SortList();
		});
		UIScript::Add("CreateListFavorite", [](UIScript::Token)
		{
			ServerList::ServerInfo* info = ServerList::GetCurrentServer();

			if (info)
			{
				ServerList::StoreFavourite(info->addr.getString());
			}
		});
		UIScript::Add("CreateFavorite", [](UIScript::Token)
		{
			ServerList::StoreFavourite(Dvar::Var("ui_favoriteAddress").get<std::string>());
		});
		UIScript::Add("CreateCurrentServerFavorite", [](UIScript::Token)
		{
			if (Game::CL_IsCgameInitialized())
			{
				std::string addressText = Network::Address(*Game::connectedHost).getString();
				if (addressText != "0.0.0.0:0" && addressText != "loopback")
				{
					ServerList::StoreFavourite(addressText);
				}
			}
		});
		UIScript::Add("DeleteFavorite", [](UIScript::Token)
		{
			ServerList::ServerInfo* info = ServerList::GetCurrentServer();

			if (info)
			{
				ServerList::RemoveFavourite(info->addr.getString());
			};
		});

		Command::Add("playerCount", [](Command::Params*)
		{
			int count = 0;
			for (auto server : ServerList::OnlineList)
			{
				count += server.clients;
			}

			Logger::Print("There are %d players playing.\n", count);
		});

		// Add required ownerDraws
		UIScript::AddOwnerDraw(220, ServerList::UpdateSource);
		UIScript::AddOwnerDraw(253, ServerList::UpdateGameType);

		// Add frame callback
		Scheduler::OnFrame(ServerList::Frame);

		// This is placed here in case the anticheat has been disabled!
#if !defined(DISABLE_ANTICHEAT) && defined(PROCTECT_PROCESS)
		Scheduler::OnFrame(AntiCheat::ReadIntegrityCheck, true);
#endif
	}

	ServerList::~ServerList()
	{
		ServerList::OnlineList.clear();
		ServerList::OfflineList.clear();
		ServerList::FavouriteList.clear();
		ServerList::VisibleList.clear();

		{
			std::lock_guard<std::recursive_mutex> _(ServerList::RefreshContainer.mutex);
			ServerList::RefreshContainer.awatingList = false;
			ServerList::RefreshContainer.servers.clear();
		}
	}
}
