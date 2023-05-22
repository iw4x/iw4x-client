#include <STDInclude.hpp>
#include <Utils/InfoString.hpp>

#include "Discovery.hpp"
#include "Events.hpp"
#include "Node.hpp"
#include "Party.hpp"
#include "ServerList.hpp"
#include "TextRenderer.hpp"
#include "Toast.hpp"
#include "UIFeeder.hpp"

namespace Components
{
	bool ServerList::SortAsc = true;
	int ServerList::SortKey = static_cast<std::underlying_type_t<Column>>(Column::Ping);

	unsigned int ServerList::CurrentServer = 0;
	ServerList::Container ServerList::RefreshContainer;

	std::vector<ServerList::ServerInfo> ServerList::OnlineList;
	std::vector<ServerList::ServerInfo> ServerList::OfflineList;
	std::vector<ServerList::ServerInfo> ServerList::FavouriteList;

	std::vector<unsigned int> ServerList::VisibleList;

	Dvar::Var ServerList::UIServerSelected;
	Dvar::Var ServerList::UIServerSelectedMap;
	Dvar::Var ServerList::NETServerQueryLimit;
	Dvar::Var ServerList::NETServerFrames;

	bool ServerList::UseMasterServer = true;

	std::vector<ServerList::ServerInfo>* ServerList::GetList()
	{
		if (IsOnlineList())
		{
			return &OnlineList;
		}

		if (IsOfflineList())
		{
			return &OfflineList;
		}

		if (IsFavouriteList())
		{
			return &FavouriteList;
		}

		return nullptr;
	}

	bool ServerList::IsFavouriteList()
	{
		return (*Game::ui_netSource)->current.integer == 2;
	}

	bool ServerList::IsOfflineList()
	{
		return (*Game::ui_netSource)->current.integer == 0;
	}

	bool ServerList::IsOnlineList()
	{
		return (*Game::ui_netSource)->current.integer == 1;
	}

	unsigned int ServerList::GetServerCount()
	{
		return VisibleList.size();
	}

	const char* ServerList::GetServerText(unsigned int index, int column)
	{
		auto* info = GetServer(index);

		if (info)
		{
			return GetServerInfoText(info, column);
		}

		return "";
	}

	const char* ServerList::GetServerInfoText(ServerInfo* server, int column, bool sorting)
	{
		if (!server) return "";

		switch (static_cast<Column>(column))
		{
		case Column::Password:
		{
			return (server->password ? ":icon_locked:" : "");
		}

		case Column::Matchtype:
		{
			return ((server->matchType == 1) ? "P" : "M");
		}

		case Column::AimAssist:
		{
			return ((server->aimassist == 1) ? ":headshot:" : "");
		}

		case Column::VoiceChat:
		{
			return ((server->voice == 1) ? ":voice_on:" : "");
		}

		case Column::Hostname:
		{
			return server->hostname.data();
		}

		case Column::Mapname:
		{
			if (server->svRunning)
			{
				if (!sorting && !Maps::CheckMapInstalled(server->mapname))
				{
					return Utils::String::VA("^1%s", Localization::LocalizeMapName(server->mapname.data()));
				}

				return Localization::LocalizeMapName(server->mapname.data());
			}

			return Utils::String::VA("^3%s", Localization::LocalizeMapName(server->mapname.data()));
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
			if (Utils::String::StartsWith(server->mod, "mods/"))
			{
				// Can point to '\0' which is fine
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

			if (server->ping < 150) // Below this is a medium ping
			{
				return Utils::String::VA("^3%i", server->ping);
			}

			return Utils::String::VA("^1%i", server->ping);
		}
		default:
		{
			break;
		}
		}

		return "";
	}

	void ServerList::SelectServer(unsigned int index)
	{
		CurrentServer = index;

		auto* serverInfo = GetCurrentServer();

		if (serverInfo)
		{
			UIServerSelected.set(true);
			UIServerSelectedMap.set(serverInfo->mapname);
			Dvar::Var("ui_serverSelectedGametype").set(serverInfo->gametype);
		}
		else
		{
			UIServerSelected.set(false);
		}
	}

	void ServerList::UpdateVisibleList([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
	{
		auto* list = GetList();
		if (!list) return;

		const std::vector tempList(*list);

		if (tempList.empty())
		{
			Refresh(UIScript::Token(), info);
		}
		else
		{
			list->clear();

			std::lock_guard _(RefreshContainer.mutex);

			RefreshContainer.sendCount = 0;
			RefreshContainer.sentCount = 0;

			for (const auto& server : tempList)
			{
				InsertRequest(server.addr);
			}
		}
	}

	void ServerList::RefreshVisibleList([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
	{
		RefreshVisibleListInternal(UIScript::Token(), info);
	}

	void ServerList::RefreshVisibleListInternal([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info, bool refresh)
	{
		Game::Dvar_SetBoolByName("ui_serverSelected", false);

		VisibleList.clear();

		auto* list = GetList();
		if (!list) return;

		if (refresh)
		{
			Refresh(UIScript::Token(), info);
			return;
		}

		auto ui_browserShowFull = Dvar::Var("ui_browserShowFull").get<bool>();
		auto ui_browserShowEmpty = Dvar::Var("ui_browserShowEmpty").get<bool>();
		auto ui_browserShowHardcore = Dvar::Var("ui_browserKillcam").get<int>();
		auto ui_browserShowPassword = Dvar::Var("ui_browserShowPassword").get<int>();
		auto ui_browserMod = Dvar::Var("ui_browserMod").get<int>();
		auto ui_joinGametype = (*Game::ui_joinGametype)->current.integer;

		for (unsigned int i = 0; i < list->size(); ++i)
		{
			auto* serverInfo = &(*list)[i];

			// Filter full servers
			if (!ui_browserShowFull && serverInfo->clients >= serverInfo->maxClients) continue;

			// Filter empty servers
			if (!ui_browserShowEmpty && serverInfo->clients <= 0) continue;

			// Filter hardcore servers
			if ((ui_browserShowHardcore == 0 && serverInfo->hardcore) || (ui_browserShowHardcore == 1 && !serverInfo->hardcore)) continue;

			// Filter servers with password
			if ((ui_browserShowPassword == 0 && serverInfo->password) || (ui_browserShowPassword == 1 && !serverInfo->password)) continue;

			// Don't show modded servers
			if ((ui_browserMod == 0 && static_cast<int>(serverInfo->mod.size())) || (ui_browserMod == 1 && serverInfo->mod.empty())) continue;

			// Filter by gametype
			if (ui_joinGametype > 0 && (ui_joinGametype - 1) < *Game::gameTypeCount  && Game::gameTypes[(ui_joinGametype - 1)].gameType != serverInfo->gametype) continue;

			VisibleList.push_back(i);
		}

		SortList();
	}

	void ServerList::Refresh([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
	{
		Dvar::Var("ui_serverSelected").set(false);

		auto* list = GetList();
		if (list) list->clear();

		VisibleList.clear();

		{
			std::lock_guard _(RefreshContainer.mutex);
			RefreshContainer.servers.clear();
			RefreshContainer.sendCount = 0;
			RefreshContainer.sentCount = 0;
		}

		if (IsOfflineList())
		{
			Discovery::Perform();
		}
		else if (IsOnlineList())
		{
			const auto masterPort = (*Game::com_masterPort)->current.integer;
			const auto* masterServerName = (*Game::com_masterServerName)->current.string;

			// Check if our dvars can properly convert to a address
			Game::netadr_t masterServerAddr;
			if (!GetMasterServer(masterServerName, masterPort, masterServerAddr))
			{
				Logger::Print("Could not resolve address for {}:{}", masterServerName, masterPort);
				Toast::Show("cardicon_headshot", "^1Error", std::format("Could not resolve address for {}:{}", masterServerName, masterPort), 5000);
				UseMasterServer = false;
				return;
			}

			Toast::Show("cardicon_headshot", "Server Browser", "Fetching servers...", 3000);

			UseMasterServer = true;

			RefreshContainer.awatingList = true;
			RefreshContainer.awaitTime = Game::Sys_Milliseconds();
			RefreshContainer.host = Network::Address(std::format("{}:{}", masterServerName, masterPort));

			Logger::Print("Sending server list request to master\n");
			Network::SendCommand(RefreshContainer.host, "getservers", std::format("IW4 {} full empty", PROTOCOL));
		}
		else if (IsFavouriteList())
		{
			LoadFavourties();
		}
	}

	void ServerList::StoreFavourite(const std::string& server)
	{
		std::vector<std::string> servers;
		
		const auto parseData = Utils::IO::ReadFile(FavouriteFile);
		if (!parseData.empty())
		{
			nlohmann::json object;
			try
			{
				object = nlohmann::json::parse(parseData);
			}
			catch (const nlohmann::json::parse_error& ex)
			{
				Logger::PrintError(Game::CON_CHANNEL_ERROR, "JSON Parse Error: {}\n", ex.what());
				return;
			}

			if (!object.is_array())
			{
				Logger::Print("Favourites storage file is invalid!\n");
				Game::ShowMessageBox("Favourites storage file is invalid!", "Error");
				return;
			}

			const nlohmann::json::array_t storedServers = object;
			for (const auto& storedServer : storedServers)
			{
				if (!storedServer.is_string()) continue;
				if (storedServer.get<std::string>() == server)
				{
					Game::ShowMessageBox("Server already marked as favourite.", "Error");
					return;
				}

				servers.push_back(storedServer.get<std::string>());
			}
		}

		servers.push_back(server);

		const auto data = nlohmann::json(servers);
		Utils::IO::WriteFile(FavouriteFile, data.dump());
		Game::ShowMessageBox("Server added to favourites.", "Success");
	}

	void ServerList::RemoveFavourite(const std::string& server)
	{
		std::vector<std::string> servers;

		const auto parseData = Utils::IO::ReadFile(FavouriteFile);
		if (!parseData.empty())
		{
			nlohmann::json object;
			try
			{
				object = nlohmann::json::parse(parseData);
			}
			catch (const nlohmann::json::parse_error& ex)
			{
				Logger::PrintError(Game::CON_CHANNEL_ERROR, "JSON Parse Error: {}\n", ex.what());
				return;
			}

			if (!object.is_array())
			{
				Logger::Print("Favourites storage file is invalid!\n");
				Game::ShowMessageBox("Favourites storage file is invalid!", "Error");
				return;
			}

			const nlohmann::json::array_t arr = object;
			for (auto& storedServer : arr)
			{
				if (storedServer.is_string() && storedServer.get<std::string>() != server)
				{
					servers.push_back(storedServer.get<std::string>());
				}
			}
		}

		const auto data = nlohmann::json(servers);
		Utils::IO::WriteFile(FavouriteFile, data.dump());

		auto* list = GetList();
		if (list) list->clear();
		
		RefreshVisibleListInternal(UIScript::Token(), nullptr);
	}

	void ServerList::LoadFavourties()
	{
		if (!IsFavouriteList())
		{
			return;
		}

		auto* list = GetList();
		if (list) list->clear();

		const auto parseData = Utils::IO::ReadFile(FavouriteFile);
		if (parseData.empty())
		{
			return;
		}

		nlohmann::json object;
		try
		{
			object = nlohmann::json::parse(parseData);
		}
		catch (const nlohmann::json::parse_error& ex)
		{
			Logger::PrintError(Game::CON_CHANNEL_ERROR, "JSON Parse Error: {}\n", ex.what());
			return;
		}

		if (!object.is_array())
		{
			Logger::Print("Favourites storage file is invalid!\n");
			Game::ShowMessageBox("Favourites storage file is invalid!", "Error");
			return;
		}

		const nlohmann::json::array_t servers = object;
		for (const auto& server : servers)
		{
			if (!server.is_string()) continue;
			InsertRequest(server.get<std::string>());
		}
	}

	void ServerList::InsertRequest(Network::Address address)
	{
		std::lock_guard _(RefreshContainer.mutex);

		Container::ServerContainer container;
		container.sent = false;
		container.target = address;

		auto alreadyInserted = false;
		for (auto &server : RefreshContainer.servers)
		{
			if (server.target == container.target)
			{
				alreadyInserted = true;
				break;
			}
		}

		if (!alreadyInserted)
		{
			RefreshContainer.servers.push_back(container);

			auto* list = GetList();
			if (list)
			{
				for (auto& server : *list)
				{
					if (server.addr == container.target)
					{
						--RefreshContainer.sendCount;
						--RefreshContainer.sentCount;
						break;
					}
				}
			}

			++RefreshContainer.sendCount;
		}
	}

	void ServerList::Insert(const Network::Address& address, const Utils::InfoString& info)
	{
		std::lock_guard _(RefreshContainer.mutex);

		for (auto i = RefreshContainer.servers.begin(); i != RefreshContainer.servers.end();)
		{
			// Our desired server
			if ((i->target != address) || !i->sent)
			{
				++i;
				continue;
			}

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
			server.version = info.get("version");
			server.mod = info.get("fs_game");
			server.matchType = std::strtol(info.get("matchtype").data(), nullptr, 10);
			server.clients = std::strtol(info.get("clients").data(), nullptr, 10);
			server.bots = std::strtol(info.get("bots").data(), nullptr, 10);
			server.securityLevel = std::strtol(info.get("securityLevel").data(), nullptr, 10);
			server.maxClients = std::strtol(info.get("sv_maxclients").data(), nullptr, 10);
			server.password = info.get("isPrivate") == "1"s;
			server.aimassist = info.get("aimAssist") == "1";
			server.voice = info.get("voiceChat") == "1"s;
			server.hardcore = info.get("hc") == "1"s;
			server.svRunning = info.get("sv_running") == "1"s;
			server.ping = (Game::Sys_Milliseconds() - i->sendTime);
			server.addr = address;

			std::hash<ServerInfo> hashFn;
			server.hash = hashFn(server);

			server.hostname = TextRenderer::StripMaterialTextIcons(server.hostname);
			server.mapname = TextRenderer::StripMaterialTextIcons(server.mapname);
			server.gametype = TextRenderer::StripMaterialTextIcons(server.gametype);
			server.mod = TextRenderer::StripMaterialTextIcons(server.mod);

			// Remove server from queue
			i = RefreshContainer.servers.erase(i);

			// Servers with more than 18 players or less than 0 players are faking for sure
			// So lets ignore those
			if (static_cast<std::size_t>(server.clients) > Game::MAX_CLIENTS || static_cast<std::size_t>(server.maxClients) > Game::MAX_CLIENTS)
			{
				return;
			}

			// Check if already inserted and remove
			auto* list = GetList();
			if (!list) return;

			std::size_t k = 0;
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
			for (auto j = VisibleList.begin(); j != VisibleList.end();)
			{
				if (*j == k)
				{
					j = VisibleList.erase(j);
				}
				else
				{
					++j;
				}
			}

			if (info.get("gamename") == "IW4"s && server.matchType)
			{
				auto* lList = GetList();
				if (lList)
				{
					if (!IsServerDuplicate(lList, server))
					{
						lList->push_back(server);
						RefreshVisibleListInternal(UIScript::Token(), nullptr);
					}
				}
			}
		}
	}

	bool ServerList::CompareVersion(const std::string& version1, const std::string& version2)
	{
		auto subVersions1 = Utils::String::Split(version1, '.');
		auto subVersions2 = Utils::String::Split(version2, '.');

		while (subVersions1.size() >= 3) subVersions1.pop_back();
		while (subVersions2.size() >= 3) subVersions2.pop_back();
		if (subVersions1.size() != subVersions2.size()) return false;

		for (std::size_t i = 0; i < subVersions1.size(); ++i)
		{
			try
			{
				if (std::stoi(subVersions1[i]) != std::stoi(subVersions2[i]))
				{
					return false;
				}
			}
			catch (const std::exception& ex)
			{
				Logger::PrintError(Game::CON_CHANNEL_ERROR, "{} while performing numeric comparison between {} and {}\n", ex.what(), subVersions1[i], subVersions2[i]);
				return false;
			}
		}

		return true;
	}

	bool ServerList::IsServerDuplicate(const std::vector<ServerInfo>* list, const ServerInfo& server)
	{
		for (auto l = list->begin(); l != list->end(); ++l)
		{
			if (l->hash == server.hash)
			{
				return true;
			}
		}

		return false;
	}

	ServerList::ServerInfo* ServerList::GetCurrentServer()
	{
		return GetServer(CurrentServer);
	}

	void ServerList::SortList()
	{
		// Only sort when the serverlist is open
		if (!IsServerListOpen()) return;

		std::ranges::stable_sort(VisibleList, [](const unsigned int& server1, const unsigned int& server2) -> bool
		{
			ServerInfo* info1 = nullptr;
			ServerInfo* info2 = nullptr;

			auto* list = GetList();
			if (!list) return false;

			if (list->size() > server1) info1 = &(*list)[server1];
			if (list->size() > server2) info2 = &(*list)[server2];

			if (!info1) return false;
			if (!info2) return false;

			// Numerical comparisons
			if (SortKey == static_cast<std::underlying_type_t<Column>>(Column::Ping))
			{
				return info1->ping < info2->ping;
			}

			if (SortKey == static_cast<std::underlying_type_t<Column>>(Column::Players))
			{
				return info1->clients < info2->clients;
			}

			auto text1 = Utils::String::ToLower(TextRenderer::StripColors(GetServerInfoText(info1, SortKey, true)));
			auto text2 = Utils::String::ToLower(TextRenderer::StripColors(GetServerInfoText(info2, SortKey, true)));

			// ASCII-based comparison
			return text1.compare(text2) < 0;
		});

		if (!SortAsc) std::ranges::reverse(VisibleList);
	}

	ServerList::ServerInfo* ServerList::GetServer(unsigned int index)
	{
		if (VisibleList.size() > index)
		{
			auto* list = GetList();
			if (!list) return nullptr;

			if (list->size() > VisibleList[index])
			{
				return &(*list)[VisibleList[index]];
			}
		}

		return nullptr;
	}

	void ServerList::Frame()
	{
		static Utils::Time::Interval frameLimit;
		const auto interval = static_cast<int>(1000.0f / static_cast<float>(NETServerFrames.get<int>()));

		if (!frameLimit.elapsed(std::chrono::milliseconds(interval)))
		{
			return;
		}

		frameLimit.update();

		std::lock_guard _(RefreshContainer.mutex);

		if (RefreshContainer.awatingList)
		{
			// Stop counting if we are out of the server browser menu
			if (!IsServerListOpen())
			{
				RefreshContainer.awatingList = false;
			}

			// Check if we haven't got a response within 5 seconds
			if (Game::Sys_Milliseconds() - RefreshContainer.awaitTime > 5000)
			{
				RefreshContainer.awatingList = false;

				Logger::Print("We haven't received a response from the master within {} seconds!\n", (Game::Sys_Milliseconds() - RefreshContainer.awaitTime) / 1000);
				Toast::Show("net_disconnect", "^2Notice", "Master server could not be reached. Switching to decentralized network", 3000);

				UseMasterServer = false;
				Node::Synchronize();
			}
		}

		const auto challenge = Utils::Cryptography::Rand::GenerateChallenge();
		auto requestLimit = NETServerQueryLimit.get<int>();
		for (std::size_t i = 0; i < RefreshContainer.servers.size() && requestLimit > 0; ++i)
		{
			auto* server = &RefreshContainer.servers[i];
			if (server->sent) continue;

			// Found server we can send a request to
			server->sent = true;
			requestLimit--;

			server->sendTime = Game::Sys_Milliseconds();
			server->challenge = challenge;

			++RefreshContainer.sentCount;

			Network::SendCommand(server->target, "getinfo", server->challenge);
		}

		UpdateVisibleInfo();
	}

	void ServerList::UpdateSource()
	{
		auto source = (*Game::ui_netSource)->current.integer;

		if (++source > (*Game::ui_netSource)->domain.integer.max)
		{
			source = 0;
		}

		Game::Dvar_SetInt(*Game::ui_netSource, source);

		RefreshVisibleListInternal(UIScript::Token(), nullptr, true);
	}

	void ServerList::UpdateGameType()
	{
		auto gametype = (*Game::ui_joinGametype)->current.integer;

		if (++gametype > *Game::gameTypeCount)
		{
			gametype = 0;
		}

		Game::Dvar_SetInt(*Game::ui_joinGametype, gametype);

		RefreshVisibleListInternal(UIScript::Token(), nullptr);
	}

	void ServerList::UpdateVisibleInfo()
	{
		static auto servers = 0;
		static auto players = 0;
		static auto bots = 0;

		auto* list = GetList();

		if (list)
		{
			auto newSevers = static_cast<int>(list->size());
			auto newPlayers = 0;
			auto newBots = 0;

			for (std::size_t i = 0; i < list->size(); ++i)
			{
				newPlayers += list->at(i).clients;
				newBots += list->at(i).bots;
			}

			if (newSevers != servers || newPlayers != players || newBots != bots)
			{
				servers = newSevers;
				players = newPlayers;
				bots = newBots;

				Localization::Set("MPUI_SERVERQUERIED", std::format("Servers: {}\nPlayers: {} ({})", servers, players, bots));
			}
		}
	}

	bool ServerList::GetMasterServer(const char* ip, int port, Game::netadr_t& address)
	{
		return Game::NET_StringToAdr(Utils::String::VA("%s:%u", ip, port), &address);
	}

	bool ServerList::IsServerListOpen()
	{
		auto* menu = Game::Menus_FindByName(Game::uiContext, "pc_join_unranked");
		if (!menu)
		{
			return false;
		}

		return Game::Menu_IsVisible(Game::uiContext, menu);
	}

	ServerList::ServerList()
	{
		OnlineList.clear();
		OfflineList.clear();
		FavouriteList.clear();
		VisibleList.clear();

		Events::OnDvarInit([]
		{
			UIServerSelected = Dvar::Register<bool>("ui_serverSelected", false,
				Game::DVAR_NONE, "Whether a server has been selected in the serverlist");
			UIServerSelectedMap = Dvar::Register<const char*>("ui_serverSelectedMap", "mp_afghan",
				Game::DVAR_NONE, "Map of the selected server");

			NETServerQueryLimit = Dvar::Register<int>("net_serverQueryLimit", 1,
				1, 10, Dedicated::IsEnabled() ? Game::DVAR_NONE : Game::DVAR_ARCHIVE, "Amount of server queries per frame");
			NETServerFrames = Dvar::Register<int>("net_serverFrames", 30,
				1, 60, Dedicated::IsEnabled() ? Game::DVAR_NONE : Game::DVAR_ARCHIVE, "Amount of server query frames per second");
		});

		// Fix ui_netsource dvar
		Utils::Hook::Nop(0x4CDEEC, 5); // Don't reset the netsource when gametypes aren't loaded

		Localization::Set("MPUI_SERVERQUERIED", "Servers: 0\nPlayers: 0 (0)");

		Network::OnClientPacket("getServersResponse", [](const Network::Address& address, [[maybe_unused]] const std::string& data)
		{
			if (RefreshContainer.host != address) return; // Only parse from host we sent to

			RefreshContainer.awatingList = false;

			std::lock_guard _(RefreshContainer.mutex);

			auto offset = 0;
			const auto count = RefreshContainer.servers.size();
			MasterEntry* entry;

			// Find first entry
			do
			{
				entry = reinterpret_cast<MasterEntry*>(const_cast<char*>(data.data()) + offset++);
			} while (!entry->HasSeparator() && !entry->IsEndToken());

			for (int i = 0; !entry[i].IsEndToken() && entry[i].HasSeparator(); ++i)
			{
				Network::Address serverAddr = address;
				serverAddr.setIP(entry[i].ip);
				serverAddr.setPort(ntohs(entry[i].port));
				serverAddr.setType(Game::NA_IP);

				InsertRequest(serverAddr);
			}

			Logger::Print("Parsed {} servers from master\n", RefreshContainer.servers.size() - count);
		});

		// Set default masterServerName + port and save it 
		Utils::Hook::Set<const char*>(0x60AD92, "master.xlabs.dev");
		Utils::Hook::Set<std::uint8_t>(0x60AD90, Game::DVAR_NONE); // masterServerName
		Utils::Hook::Set<std::uint8_t>(0x60ADC6, Game::DVAR_NONE); // masterPort

		// Add server list feeder
		UIFeeder::Add(2.0f, GetServerCount, GetServerText, SelectServer);

		// Add required UIScripts
		UIScript::Add("UpdateFilter", RefreshVisibleList);
		UIScript::Add("RefreshFilter", UpdateVisibleList);

		UIScript::Add("RefreshServers", Refresh);

		UIScript::Add("JoinServer", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
		{
			auto* serverInfo = GetServer(CurrentServer);
			if (serverInfo)
			{
				Party::Connect(serverInfo->addr);
			}
		});

		UIScript::Add("ServerSort", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
		{
			const auto key = token.get<int>();
			if (SortKey == key)
			{
				SortAsc = !SortAsc;
			}
			else
			{
				SortKey = key;
				SortAsc = true;
			}

			Logger::Print("Sorting server list by token: {}\n", SortKey);
			SortList();
		});

		UIScript::Add("CreateListFavorite", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
		{
			auto* serverInfo = GetCurrentServer();
			if (info && serverInfo && serverInfo->addr.isValid())
			{
				StoreFavourite(serverInfo->addr.getString());
			}
		});

		UIScript::Add("CreateFavorite", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
		{
			const auto value = Dvar::Var("ui_favoriteAddress").get<std::string>();
			if (!value.empty())
			{
				StoreFavourite(value);
			}
		});

		UIScript::Add("CreateCurrentServerFavorite", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
		{
			if (Game::CL_IsCgameInitialized())
			{
				const auto addressText = Network::Address(*Game::connectedHost).getString();
				if (addressText != "0.0.0.0:0"s && addressText != "loopback"s)
				{
					StoreFavourite(addressText);
				}
			}
		});

		UIScript::Add("DeleteFavorite", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
		{
			auto* serverInfo = GetCurrentServer();
			if (serverInfo)
			{
				RemoveFavourite(serverInfo->addr.getString());
			}
		});

		// Add required ownerDraws
		UIScript::AddOwnerDraw(220, UpdateSource);
		UIScript::AddOwnerDraw(253, UpdateGameType);

		// Add frame callback
		Scheduler::Loop(Frame, Scheduler::Pipeline::CLIENT);
	}

	void ServerList::preDestroy()
	{
		std::lock_guard _(RefreshContainer.mutex);
		RefreshContainer.awatingList = false;
		RefreshContainer.servers.clear();
	}
}
