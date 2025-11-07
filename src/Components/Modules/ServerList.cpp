#include <Utils/InfoString.hpp>
#include <Utils/WebIO.hpp>

#include "Discovery.hpp"
#include "Events.hpp"
#include "Node.hpp"
#include "Party.hpp"
#include "ServerList.hpp"
#include "TextRenderer.hpp"
#include "Toast.hpp"
#include "UIFeeder.hpp"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <version.hpp>

namespace Components
{
	bool ServerList::SortAsc = false;
	int ServerList::SortKey = static_cast<std::underlying_type_t<Column>>(Column::Players);

	unsigned int ServerList::CurrentServer = 0;
	ServerList::Container ServerList::RefreshContainer;

	std::vector<ServerList::ServerInfo> ServerList::OnlineList;
	std::vector<ServerList::ServerInfo> ServerList::OfflineList;
	std::vector<ServerList::ServerInfo> ServerList::FavouriteList;

	std::vector<unsigned int> ServerList::VisibleList;

	bool ServerList::UseMasterServer = false;

	Dvar::Var ServerList::UIServerSelected;
	Dvar::Var ServerList::UIServerSelectedMap;
	Dvar::Var ServerList::NETServerQueryLimit;
	Dvar::Var ServerList::NETServerFrames;
	Dvar::Var ServerList::NETServerDeadTimeout;

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
			Refresh();
		}
		else
		{
			std::lock_guard _(RefreshContainer.mutex);

			for (const auto& server : tempList)
			{
				InsertRequest(server.addr);
			}
		}
	}

	void ServerList::RefreshVisibleList([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
	{
		Scheduler::Once([info] ()
		{
			RefreshVisibleListInternal(UIScript::Token (), info);
		}, Scheduler::Pipeline::CLIENT);
	}

	void ServerList::RefreshVisibleListInternal([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info, bool refresh)
	{
		Game::Dvar_SetBoolByName("ui_serverSelected", false);

		VisibleList.clear();

		auto* list = GetList();
		if (!list) return;

		if (refresh)
		{
			Refresh();
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
			if (ui_joinGametype > 0 && (ui_joinGametype - 1) < *Game::gameTypeCount && Game::gameTypes[(ui_joinGametype - 1)].gameType != serverInfo->gametype) continue;

			VisibleList.push_back(i);
		}

		SortList();
	}

	void ServerList::ParseNewMasterServerResponse(const std::string& servers)
	{
		std::lock_guard _(RefreshContainer.mutex);

		rapidjson::Document doc{};
		const rapidjson::ParseResult result = doc.Parse(servers);
		if (!result || !doc.IsObject())
		{
			UseMasterServer = false;
			Logger::Print("Unable to parse JSON response. Using the Node System\n");
			return;
		}

		if (!doc.HasMember("servers"))
		{
			UseMasterServer = false;
			Logger::Print("Unable to parse JSON response: we were unable to find any server. Using the Node System\n");
			return;
		}

		const rapidjson::Value& list = doc["servers"];
		if (!list.IsArray() || list.Empty())
		{
			UseMasterServer = false;
			Logger::Print("Unable to parse JSON response: we were unable to find any server. Using the Node System\n");
			return;
		}

		Logger::Print("Response from the master server contains {} servers\n", list.Size());

		std::size_t count = 0;

		for (const auto& entry : list.GetArray())
		{
			if (!entry.HasMember("ip") || !entry.HasMember("port"))
			{
				continue;
			}

			if (!entry["ip"].IsString() || !entry["port"].IsInt())
			{
				continue;
			}

			if (!entry.HasMember("ip") || !entry["protocol"].IsInt())
			{
				continue;
			}

			const auto protocol = entry["protocol"].GetInt();

			if (protocol != PROTOCOL)
			{
				// We can't connect to it anyway
				continue;
			}

			// Using VA because it's faster
			Network::Address server(Utils::String::VA("%s:%u", entry["ip"].GetString(), entry["port"].GetInt()));
			server.setType(Game::NA_IP); // Just making sure...

			InsertRequest(server);
			++count;
		}

		if (!count)
		{
			UseMasterServer = false;
			Logger::Print("Despite receiving what looked like a valid response from the master server, we got {} servers. Using the Node System\n", count);
			return;
		}

		UseMasterServer = true;
		Logger::Print("Response from the master server was successfully parsed. We got {} servers\n", count);
	}

	void ServerList::Refresh()
	{
		Dvar::Var("ui_serverSelected").set(false);

		auto* list = GetList();

		const bool hasCachedServers = list && !list->empty ();

		if (!hasCachedServers)
			VisibleList.clear ();

		{
			std::lock_guard _(RefreshContainer.mutex);
			RefreshContainer.servers.clear();

			// Record that the visible set must be rebuilt after the first discovery
			// pass when starting without a cache. Note that in this situation the
			// browser has no prior ordering or selection state, so the initial
			// snapshot must drive the first stable view.
			//
			if (!hasCachedServers && IsOnlineList ())
				RefreshContainer.needsInitialRefresh = true;
		}

		if (IsOfflineList())
		{
			Discovery::Perform();
		}
		else if (IsOnlineList())
		{
			// Warms the list early and lets the first discovery cycle reconcile
			// cached state with the actual network view.
			//
			if (hasCachedServers && list)
			{
				for (const auto& server : *list)
				{
					InsertRequest(server.addr);
				}
			}

			const auto masterPort = (*Game::com_masterPort)->current.unsignedInt;
			const auto* masterServerName = (*Game::com_masterServerName)->current.string;

			RefreshContainer.awaitingList = true;
			RefreshContainer.awaitTime = Game::Sys_Milliseconds();

			if (!hasCachedServers)
			{
				Toast::Show("cardicon_headshot", "Server Browser", "Fetching servers...", 3000);
			}

			std::thread([masterServerName, masterPort]()
			{
				const auto host = "master.iw4x.io";
				const auto url = std::format("http://{}/v1/servers/iw4x?protocol={}", host, PROTOCOL);
				const auto reply = Utils::WebIO("IW4x", url).setTimeout(5000)->get();

				Scheduler::Once([reply, masterServerName, masterPort, url]()
				{
					{
						std::lock_guard _(RefreshContainer.mutex);
						RefreshContainer.awaitingList = false;
					}

					if (reply.empty())
					{
						Logger::Print("Response from {} was empty or the request timed out, falling back to node system.\n", url);
						Toast::Show("cardicon_headshot", "^1Error", std::format("Could not get a response from {}, falling back to node system.\n", url), 5000);
						UseMasterServer = false;
						return;
					}

					ParseNewMasterServerResponse(reply);
					RefreshContainer.host = Network::Address(std::format("{}:{}", masterServerName, masterPort));
				}, Scheduler::Pipeline::CLIENT);
			}).detach();
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

		Scheduler::Once([] ()
		{
			RefreshVisibleListInternal(UIScript::Token (), nullptr);
		}, Scheduler::Pipeline::CLIENT);
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

	void ServerList::LoadServerCache()
	{
		if (!IsOnlineList())
			return;

		std::string cache (Utils::IO::ReadFile (ServerCacheFile));
		if (cache.empty ())
			return;

		nlohmann::json root;
		try
		{
			root = nlohmann::json::parse (cache);
		}
		catch (const nlohmann::json::parse_error& e)
		{
			Logger::PrintError(Game::CON_CHANNEL_ERROR,
												"JSON parse error in server cache: {}\n",
												e.what());

			// Treat malformed cache data as non-fatal. The cache is simply ignored
			// and a fresh list will be constructed via the normal discovery path.
			//
			return;
		}

		if (!root.is_object () ||
				!root.contains ("servers") ||
				!root["servers"].is_array ())
		{
			Logger::Print ("server cache file is invalid\n");

			// non-fatal. (see above)
			//
			return;
		}

		const auto& servers = root["servers"];
		auto* list = GetList();
		if (list == nullptr)
			return;

		Logger::Print ("loading {} cached servers...\n", servers.size ());

		for (const nlohmann::json& entry : servers)
		{
			if (!entry.is_object ())
				continue;

			try
			{
				ServerInfo s;

				s.addr          = Network::Address (entry.value ("address", ""));
				s.hostname      = entry.value ("hostname", "");
				s.mapname       = entry.value ("mapname", "");
				s.gametype      = entry.value ("gametype", "");
				s.mod           = entry.value ("mod", "");
				s.version       = entry.value ("version", "");
				s.clients       = entry.value ("clients", 0);
				s.bots          = entry.value ("bots", 0);
				s.maxClients    = entry.value ("maxClients", 0);
				s.password      = entry.value ("password", false);
				s.ping          = entry.value ("ping", 999);
				s.matchType     = entry.value ("matchType", 0);
				s.securityLevel = entry.value ("securityLevel", 0);
				s.protocol      = entry.value ("protocol", PROTOCOL);
				s.hardcore      = entry.value ("hardcore", false);
				s.svRunning     = entry.value ("svRunning", false);
				s.aimassist     = entry.value ("aimassist", false);
				s.voice         = entry.value ("voice", false);
				s.lastSeen      = entry.value ("lastSeen", std::time (nullptr));

				std::hash<ServerInfo> h;
				s.hash = h (s);

				if (!IsServerDuplicate (list, s))
					list->push_back (s);
			}
			catch (const std::exception& e)
			{
				Logger::PrintError (Game::CON_CHANNEL_ERROR,
														"error loading cached server: {}\n",
														e.what ());
			}
		}

		// Recompute visibility after population.
		//
		Scheduler::Once([]()
		{
			RefreshVisibleListInternal(UIScript::Token(), nullptr);
		}, Scheduler::Pipeline::CLIENT);

		Logger::Print ("loaded {} servers from cache\n", list->size ());
	}

	void ServerList::SaveServerCache ()
	{
		if (!IsOnlineList ())
			return;

		auto* list = GetList();
		if (list == nullptr || list->empty ())
			return;

		nlohmann::json::array_t servers;

		for (const ServerInfo& s : *list)
		{
			nlohmann::json e;

			e["address"]       = s.addr.getString ();
			e["hostname"]      = s.hostname;
			e["mapname"]       = s.mapname;
			e["gametype"]      = s.gametype;
			e["mod"]           = s.mod;
			e["version"]       = s.version;
			e["clients"]       = s.clients;
			e["bots"]          = s.bots;
			e["maxClients"]    = s.maxClients;
			e["password"]      = s.password;
			e["ping"]          = s.ping;
			e["matchType"]     = s.matchType;
			e["securityLevel"] = s.securityLevel;
			e["protocol"]      = s.protocol;
			e["hardcore"]      = s.hardcore;
			e["svRunning"]     = s.svRunning;
			e["aimassist"]     = s.aimassist;
			e["voice"]         = s.voice;
			e["lastSeen"]      = s.lastSeen;

			servers.push_back (e);
		}

		nlohmann::json root;

		root["servers"]   = servers;
		root["timestamp"] = std::time (nullptr);
		root["version"]   = REVISION_STR;

		Utils::IO::WriteFile (ServerCacheFile, root.dump ());

		// Note that ephemeral entries are not included, so the cached set may
		// differ from the full set returned by live node queries.
		//
		Logger::Print ("saved {} servers to cache\n", servers.size ());
	}

	void ServerList::RemoveDeadServers ()
	{
		if (!IsOnlineList ())
			return;

		auto* list (GetList ());
		if (list == nullptr || list->empty ())
			return;

		const std::time_t now (std::time (nullptr));
		const std::time_t timeout (NETServerDeadTimeout.get<int> ());

		std::size_t removed (0);

		// Prune entries that have not produced a response within the configured
		// timeout window.
		//
		for (auto i (list->begin ()); i != list->end (); )
		{
			if (now - i->lastSeen > timeout)
			{
				Logger::Print ("removing dead server: {} (last seen {} seconds ago)\n",
											i->addr.getString (), now - i->lastSeen);

				i = list->erase (i);
				++removed;
			}
			else
				++i;
		}

		if (removed > 0)
		{
			Logger::Print ("removed {} dead servers from cache\n", removed);

			// Note that we do not persist the cache here. While it may appear natural
			// to save immediately after pruning, the periodic cache-save interval is
			// aligned with the heartbeat/dead-check cadence. In practice, removal
			// only occurs during those cycles, which guarantees that a scheduled
			// cache write will follow in the same frame or shortly thereafter.
			//
			Scheduler::Once([] ()
			{
				RefreshVisibleListInternal(UIScript::Token (), nullptr);
			}, Scheduler::Pipeline::CLIENT);
		}
	}

	void ServerList::HeartbeatServers()
	{
		if (!IsOnlineList ())
			return;

		auto* list (GetList ());
		if (list == nullptr || list->empty ())
			return;

		Logger::Print ("starting heartbeat check for {} cached servers\n",
									list->size ());

		// Note that we issue getinfo requests for each cached server to refresh
		// without requiring a full discovery sweep.
		//
		std::lock_guard lock (RefreshContainer.mutex);

		for (const ServerInfo& s : *list)
		{
			bool queued (false);

			for (const Container::ServerContainer& c : RefreshContainer.servers)
			{
				if (c.target == s.addr)
				{
					queued = true;
					break;
				}
			}

			if (!queued)
			{
				Container::ServerContainer c;
				c.sent   = false;
				c.target = s.addr;

				RefreshContainer.servers.push_back (c);
			}
		}

		Logger::Print ("queued {} servers for heartbeat ping\n",
									 RefreshContainer.servers.size());
	}

	void ServerList::InsertRequest(Network::Address address)
	{
		std::lock_guard _(RefreshContainer.mutex);

		Container::ServerContainer c;
		c.sent   = false;
		c.target = address;

		auto alreadyInserted = false;
		for (auto& s : RefreshContainer.servers)
		{
			if (s.target == c.target)
			{
				alreadyInserted = true;
				break;
			}
		}

		if (!alreadyInserted)
			RefreshContainer.servers.push_back(c);
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
			server.lastSeen = std::time(nullptr);

			std::hash<ServerInfo> hashFn;
			server.hash = hashFn(server);

			// more secure
			server.hostname = TextRenderer::StripMaterialTextIcons(server.hostname);

			if (server.hostname.empty() || std::all_of(server.hostname.begin(), server.hostname.end(), isspace))
			{
				// Invalid server name containing only emojis
				return;
			}

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

			bool found (false);
			for (auto& s: *list)
			{
				if (s.addr == address)
				{
					// Update entry in-place to retain list position.
					//
					s = server;
					found = true;
					break;
				}
			}

			if (info.get("gamename") == "IW4"s && server.matchType)
			{
				if (auto* l = GetList (); l != nullptr)
				{
					// NOTE: The visible list is not refreshed here during normal
					// operation. Recomputing visibility on each heartbeat causes the
					// browser to re-sort while player counts fluctuate, which makes
					// entries appear to "jump" during normal activity.
					//
					// ... Well, turn out that during initial discovery, the browser is
					// still forming its first stable view, so entries must be allowed to
					// surface incrementally as responses arrive.
					//
					if (!found && !IsServerDuplicate (l, server))
					{
						l->push_back (server);

						if (RefreshContainer.needsInitialRefresh)
						{
							Scheduler::Once([]()
							{
								RefreshVisibleListInternal(UIScript::Token(), nullptr);
							}, Scheduler::Pipeline::CLIENT);
						}
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
		static Utils::Time::Interval cacheSaveInterval;
		static Utils::Time::Interval heartbeatInterval;
		static Utils::Time::Interval deadServerCheckInterval;
		static bool wasOpen = false;

		// Skip update processing when the browser view is inactive.
		//
		if (!IsServerListOpen ())
		{
			std::lock_guard _ (RefreshContainer.mutex);

			if (!RefreshContainer.servers.empty ())
				RefreshContainer.servers.clear ();

			wasOpen = false;
			return;
		}

		// Re-entry into the browser must clears accumulated idle time of existing
		// entries to avoid classifying them as dead upon return.
		//
		//
		if (!wasOpen)
		{
			wasOpen = true;

			auto* l (GetList ());

			if (l != nullptr && !l->empty ())
			{
				const auto now (std::time (nullptr));

				for (auto& s: *l)
					s.lastSeen = now;
			}
		}

		const auto interval = static_cast<int>(1000.0f / static_cast<float>(NETServerFrames.get<int>()));

		if (!frameLimit.elapsed(std::chrono::milliseconds(interval)))
		{
			return;
		}

		frameLimit.update();

		// FIXME: Intervals should come from a dvar (NET...). In practice, they
		// interacts poorly with server frame processing, so their value is kept
		// local for now.

		// Periodically send heartbeat pings to cached servers
		//
		if (heartbeatInterval.elapsed(std::chrono::seconds(30)))
		{
			heartbeatInterval.update();

			if (IsOnlineList())
				HeartbeatServers();
		}

		// Periodically check and remove dead servers
		//
		if (deadServerCheckInterval.elapsed(std::chrono::seconds(30)))
		{
			deadServerCheckInterval.update();

			if (IsOnlineList())
				RemoveDeadServers();
		}

		// Periodically write current online list to on-disk cache
		//
		if (cacheSaveInterval.elapsed(std::chrono::seconds(30)))
		{
			cacheSaveInterval.update ();

			if (IsOnlineList ())
			{
				if (auto* list = GetList (); list != nullptr && !list->empty ())
					SaveServerCache ();
			}
		}

		std::lock_guard _(RefreshContainer.mutex);

		if (RefreshContainer.awaitingList)
		{
			// Stop counting if we are out of the server browser menu
			if (!IsServerListOpen())
			{
				RefreshContainer.awaitingList = false;
			}

			// Check if we haven't got a response within 5 seconds
			if (Game::Sys_Milliseconds() - RefreshContainer.awaitTime > 5000)
			{
				RefreshContainer.awaitingList = false;
				Logger::Print("We haven't received a response from the master within {} seconds!\n", (Game::Sys_Milliseconds() - RefreshContainer.awaitTime) / 1000);

				UseMasterServer = false;
				Node::Synchronize();
			}
		}

		const auto challenge = Utils::Cryptography::Rand::GenerateChallenge();
		auto requestLimit = NETServerQueryLimit.get<int>();

		bool hadPendingRequests = false;

		for (std::size_t i = 0; i < RefreshContainer.servers.size() && requestLimit > 0; ++i)
		{
			auto* server = &RefreshContainer.servers[i];
			if (server->sent) continue;

			// Found server we can send a request to
			server->sent = true;
			requestLimit--;
			hadPendingRequests = true;

			server->sendTime = Game::Sys_Milliseconds();
			server->challenge = challenge;

			Network::SendCommand(server->target, "getinfo", server->challenge);
		}

		// If the list is populated and no requests remain pending, then the
		// discovery phase has produced a stable snapshot. That is, the flag is
		// lowered and the on-disk cache may be written.
		//
		if (RefreshContainer.needsInitialRefresh && !hadPendingRequests)
		{
			auto* l (GetList ());

			if (l != nullptr && !l->empty ())
			{
				RefreshContainer.needsInitialRefresh = false;

				if (IsOnlineList ())
					SaveServerCache ();
			}
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

		Scheduler::Once([]()
		{
			RefreshVisibleListInternal(UIScript::Token(), nullptr);
		}, Scheduler::Pipeline::CLIENT);
	}

	void ServerList::UpdateGameType()
	{
		auto gametype = (*Game::ui_joinGametype)->current.integer;

		if (++gametype > *Game::gameTypeCount)
		{
			gametype = 0;
		}

		Game::Dvar_SetInt(*Game::ui_joinGametype, gametype);

		Scheduler::Once([]()
		{
			RefreshVisibleListInternal(UIScript::Token(), nullptr);
		}, Scheduler::Pipeline::CLIENT);
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
				NETServerDeadTimeout = Dvar::Register<int>("net_serverDeadTimeout", 60,
					1, 604800, Dedicated::IsEnabled() ? Game::DVAR_NONE : Game::DVAR_ARCHIVE, "Seconds after which unresponsive servers are removed from cache");
			});

		// Fix ui_netsource dvar
		Utils::Hook::Nop(0x4CDEEC, 5); // Don't reset the netsource when gametypes aren't loaded

		Localization::Set("MPUI_SERVERQUERIED", "Servers: 0\nPlayers: 0 (0)");

		Network::OnClientPacket("getServersResponse", [](const Network::Address& address, [[maybe_unused]] const std::string& data)
			{
				if (RefreshContainer.host != address) return; // Only parse from host we sent to

				RefreshContainer.awaitingList = false;

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
		Utils::Hook::Set<const char*>(0x60AD92, "dp.iw4x.io");
		Utils::Hook::Set<std::uint8_t>(0x60AD90, Game::DVAR_NONE); // masterServerName
		Utils::Hook::Set<std::uint8_t>(0x60ADC6, Game::DVAR_NONE); // masterPort

		// Add server list feeder
		UIFeeder::Add(2.0f, GetServerCount, GetServerText, SelectServer);

		// Add required UIScripts
		UIScript::Add("UpdateFilter", RefreshVisibleList);
		UIScript::Add("RefreshFilter", UpdateVisibleList);
		UIScript::Add("RefreshServers",
									[](const UIScript::Token&, const Game::uiInfo_s*)
		{
			// Attempt to populate online list from on-disk cache before issuing a
			// network-driven refresh.
			//
			if (IsOnlineList())
			{
				auto* list = GetList();

				if (list != nullptr && list->empty())
				{
					std::thread([]()
					{
						LoadServerCache();
						Scheduler::Once([]() { ServerList::Refresh(); }, Scheduler::Pipeline::CLIENT);
					}).detach();

					return; // defer refresh until after the cache load completes
				}
			}

			ServerList::Refresh();
		});

		UIScript::Add("JoinServer", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
			{
				auto* serverInfo = GetServer(CurrentServer);
				if (serverInfo)
				{
					Party::Connect(serverInfo->addr);
				}
			});

		UIScript::Add("DownloadServerMod", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
			{
				auto* serverInfo = GetServer(CurrentServer);
				if (serverInfo)
				{
					Party::Connect(serverInfo->addr, true);
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
		RefreshContainer.awaitingList = false;
		RefreshContainer.servers.clear();
	}
}
