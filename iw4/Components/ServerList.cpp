#include "..\STDInclude.hpp"

namespace Components
{
	unsigned int ServerList::CurrentServer = 0;
	ServerList::Container ServerList::RefreshContainer;
	std::vector<ServerList::ServerInfo> ServerList::OnlineList;

	int ServerList::GetServerCount()
	{
		return ServerList::OnlineList.size();
	}

	const char* ServerList::GetServerText(int index, int column)
	{
		if ((unsigned int)index >= ServerList::OnlineList.size()) return "";

		ServerList::ServerInfo* Server = &ServerList::OnlineList[index];

		switch (column)
		{
			case Column::Password:
			{
				return (Server->Password ? "X" : "");
			}

			case Column::Hostname:
			{
				return Server->Hostname.data();
			}

			case Column::Mapname:
			{
				return  Game::UI_LocalizeMapName(Server->Mapname.data());
			}

			case Column::Players:
			{
				return Utils::VA("%i (%i)", Server->Clients, Server->MaxClients);
			}

			case Column::Gametype:
			{
				if (Server->Mod != "")
				{
					return (Server->Mod.data() + 5);
				}

				return Game::UI_LocalizeGameType(Server->Gametype.data());
			}

			case Column::Ping:
			{
				return Utils::VA("%i", Server->Ping);
			}
		}

		return "";
	}

	void ServerList::SelectServer(int index)
	{
		ServerList::CurrentServer = (unsigned int)index;
	}

	void ServerList::Refresh()
	{
		ServerList::OnlineList.clear();

		ServerList::RefreshContainer.Mutex.lock();
		ServerList::RefreshContainer.Servers.clear();
		ServerList::RefreshContainer.Mutex.unlock();

		ServerList::RefreshContainer.SendCount = 0;
		ServerList::RefreshContainer.SentCount = 0;

		ServerList::RefreshContainer.AwatingList = true;
		ServerList::RefreshContainer.AwaitTime = Game::Com_Milliseconds();

		int masterPort = Dvar::Var("masterPort").Get<int>();
		const char* masterServerName = Dvar::Var("masterServerName").Get<const char*>();

		ServerList::RefreshContainer.Host = Network::Address(Utils::VA("%s:%u", masterServerName, masterPort));

		Logger::Print("Sending serverlist request to master: %s:%u\n", masterServerName, masterPort);

		//Network::Send(ServerList::RefreshContainer.Host, "getservers IW4 145 full empty");
		Network::Send(ServerList::RefreshContainer.Host, "getservers 0 full empty\n");
	}

	void ServerList::Insert(Network::Address address, Utils::InfoString info)
	{
		ServerList::RefreshContainer.Mutex.lock();

		for (auto i = ServerList::RefreshContainer.Servers.begin(); i != ServerList::RefreshContainer.Servers.end(); i++)
		{
			// Our desired server
			if (i->Target == address && i->Sent)
			{
				// Challenge did not match
				if (i->Challenge != info.Get("challenge"))
				{
					// Shall we remove the server from the queue?
					// Better not, it might send a second response with the correct challenge.
					// This might happen when users refresh twice (or more often) in a short period of time
					break;
				}

				// TODO: Implement deeper check like version and game
				ServerInfo server;
				server.Hostname = info.Get("hostname");
				server.Mapname = info.Get("mapname");
				server.Gametype = info.Get("gametype");
				server.Mod = info.Get("fs_game");
				server.MatchType = atoi(info.Get("matchtype").data());
				server.Clients = atoi(info.Get("clients").data());
				server.MaxClients = atoi(info.Get("sv_maxclients").data());
				server.Password = 0; // No info yet
				server.Ping = (Game::Com_Milliseconds() - i->SendTime);
				server.Addr = address;

				// Check if already inserted and remove
				for (auto j = ServerList::OnlineList.begin(); j != ServerList::OnlineList.end(); j++)
				{
					if (j->Addr == address)
					{
						ServerList::OnlineList.erase(j);
						break;
					}
				}

				ServerList::OnlineList.push_back(server);

				ServerList::RefreshContainer.Servers.erase(i);

				break;
			}
		}

		ServerList::RefreshContainer.Mutex.unlock();
	}

	void ServerList::Frame()
	{
		ServerList::RefreshContainer.Mutex.lock();

		if (ServerList::RefreshContainer.AwatingList)
		{
			// Check if we haven't got a response within 10 seconds
			if (Game::Com_Milliseconds() - ServerList::RefreshContainer.AwaitTime > 5000)
			{
				ServerList::RefreshContainer.AwatingList = false;

				Logger::Print("We haven't received a response from the master within %d seconds!\n", (Game::Com_Milliseconds() - ServerList::RefreshContainer.AwaitTime) / 1000);
			}
		}

		// Send requests to 10 servers each frame
		int SendServers = 10;
		
		for (unsigned int i = 0; i < ServerList::RefreshContainer.Servers.size(); i++)
		{
			ServerList::Container::ServerContainer* server = &ServerList::RefreshContainer.Servers[i];
			if (server->Sent) continue;

			// Found server we can send a request to
			server->Sent = true;
			SendServers--;

			server->SendTime = Game::Com_Milliseconds();
			server->Challenge = Utils::VA("%d", server->SendTime);

			ServerList::RefreshContainer.SentCount++;

			Network::Send(server->Target, Utils::VA("getinfo %s\n", server->Challenge.data()));

			// Display in the menu, like in COD4
			//Logger::Print("Sent %d/%d\n", ServerList::RefreshContainer.SentCount, ServerList::RefreshContainer.SendCount);

			if (SendServers <= 0) break;
		}

		ServerList::RefreshContainer.Mutex.unlock();
	}

	ServerList::ServerList()
	{
		ServerList::OnlineList.clear();

		Network::Handle("getServersResponse", [] (Network::Address address, std::string data)
		{
			if (ServerList::RefreshContainer.Host != address) return; // Only parse from host we sent to

			ServerList::RefreshContainer.AwatingList = false;

			ServerList::RefreshContainer.Mutex.lock();

			int offset = 0;
			int count = ServerList::RefreshContainer.Servers.size();
			ServerList::MasterEntry* entry = nullptr;

			// Find first entry
			do 
			{
				entry = (ServerList::MasterEntry*)(data.data() + offset++);
			}
			while (!entry->HasSeparator() && !entry->IsEndToken());

			for (int i = 0; !entry[i].IsEndToken() && entry[i].HasSeparator(); i++)
			{
				Network::Address serverAddr = address;
				serverAddr.SetIP(entry[i].IP);
				serverAddr.SetPort(entry[i].Port);
				serverAddr.Get()->type = Game::NA_IP;

				ServerList::Container::ServerContainer container;
				container.Sent = false;
				container.Target = serverAddr;

				bool alreadyInserted = false;
				for (auto &server : ServerList::RefreshContainer.Servers)
				{
					if (server.Target == container.Target)
					{
						alreadyInserted = true;
						break;
					}
				}

				if (!alreadyInserted)
				{
					ServerList::RefreshContainer.Servers.push_back(container);
					ServerList::RefreshContainer.SendCount++;
				}
			}

			Logger::Print("Parsed %d servers from master\n", ServerList::RefreshContainer.Servers.size() - count);

			ServerList::RefreshContainer.Mutex.unlock();
		});

		// Set default masterServerName + port and save it 
		Utils::Hook::Set<const char*>(0x60AD92, "localhost");
		Utils::Hook::Set<BYTE>(0x60AD90, Game::dvar_flag::DVAR_FLAG_SAVED); // masterServerName
		Utils::Hook::Set<BYTE>(0x60ADC6, Game::dvar_flag::DVAR_FLAG_SAVED); // masterPort

		// Add server list feeder
		UIFeeder::Add(2.0f, ServerList::GetServerCount, ServerList::GetServerText, ServerList::SelectServer);

		// Add required UIScripts
		UIScript::Add("RefreshServers", ServerList::Refresh);
		UIScript::Add("JoinServer", [] ()
		{
			if (ServerList::OnlineList.size() > ServerList::CurrentServer)
			{
				Party::Connect(ServerList::OnlineList[ServerList::CurrentServer].Addr);
			}
		});
		UIScript::Add("ServerSort", [] (UIScript::Token token)
		{
			Logger::Print("Server list sorting by token: %d\n", token.Get<int>());
		});

		// Add frame callback
		Renderer::OnFrame(ServerList::Frame);
	}

	ServerList::~ServerList()
	{
		ServerList::OnlineList.clear();
	}
}
