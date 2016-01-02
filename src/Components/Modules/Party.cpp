#include "..\..\STDInclude.hpp"

namespace Components
{
	Party::JoinContainer Party::Container;
	std::map<uint64_t, Network::Address> Party::LobbyMap;

	SteamID Party::GenerateLobbyId()
	{
		SteamID id;

		id.AccountID = Game::Com_Milliseconds();
		id.Universe = 1;
		id.AccountType = 8;
		id.AccountInstance = 0x40000;

		return id;
	}

	Network::Address Party::Target()
	{
		return Party::Container.Target;
	}

	void Party::Connect(Network::Address target)
	{
		Party::Container.Valid = true;
		Party::Container.AwaitingPlaylist = false;
		Party::Container.JoinTime = Game::Com_Milliseconds();
		Party::Container.Target = target;
		Party::Container.Challenge = Utils::VA("%X", Party::Container.JoinTime);

		Network::Send(Party::Container.Target, Utils::VA("getinfo %s\n", Party::Container.Challenge.data()));

		Command::Execute("openmenu popup_reconnectingtoparty");
	}

	const char* Party::GetLobbyInfo(SteamID lobby, std::string key)
	{
		if (Party::LobbyMap.find(lobby.Bits) != Party::LobbyMap.end())
		{
			Network::Address address = Party::LobbyMap[lobby.Bits];

			if (key == "addr")
			{
				return Utils::VA("%d", address.Get()->ip[0] | (address.Get()->ip[1] << 8) | (address.Get()->ip[2] << 16) | (address.Get()->ip[3] << 24));
			}
			else if (key =="port")
			{
				return Utils::VA("%d", htons(address.GetPort()));
			}
		}

		return "212";
	}

	void Party::RemoveLobby(SteamID lobby)
	{
		if (Party::LobbyMap.find(lobby.Bits) != Party::LobbyMap.end())
		{
			Party::LobbyMap.erase(Party::LobbyMap.find(lobby.Bits));
		}
	}

	void Party::ConnectError(std::string message)
	{
		Command::Execute("closemenu popup_reconnectingtoparty");
		Dvar::Var("partyend_reason").Set(message);
		Command::Execute("openmenu menu_xboxlive_partyended");
	}

	Game::dvar_t* Party::RegisterMinPlayers(const char* name, int value, int min, int max, Game::dvar_flag flag, const char* description)
	{
		return Dvar::Register<int>(name, 1, 1, max, Game::dvar_flag::DVAR_FLAG_WRITEPROTECTED | flag, description).Get<Game::dvar_t*>();
	}

	bool Party::PlaylistAwaiting()
	{
		return Party::Container.AwaitingPlaylist;
	}

	void Party::PlaylistContinue()
	{
		Party::Container.AwaitingPlaylist = false;

		SteamID id = Party::GenerateLobbyId();

		Party::LobbyMap[id.Bits] = Party::Container.Target;

		Game::Steam_JoinLobby(id, 0);

		// Callback not registered on first try
		// TODO: Fix :D
		if (Party::LobbyMap.size() <= 1) Game::Steam_JoinLobby(id, 0);
	}

	Party::Party()
	{
		// various changes to SV_DirectConnect-y stuff to allow non-party joinees
		Utils::Hook::Set<WORD>(0x460D96, 0x90E9);
		Utils::Hook::Set<BYTE>(0x460F0A, 0xEB);
		Utils::Hook::Set<BYTE>(0x401CA4, 0xEB);
		Utils::Hook::Set<BYTE>(0x401C15, 0xEB);

		// disable configstring checksum matching (it's unreliable at most)
		Utils::Hook::Set<BYTE>(0x4A75A7, 0xEB); // SV_SpawnServer
		Utils::Hook::Set<BYTE>(0x5AC2CF, 0xEB); // CL_ParseGamestate
		Utils::Hook::Set<BYTE>(0x5AC2C3, 0xEB); // CL_ParseGamestate

		// AnonymousAddRequest
		Utils::Hook::Set<BYTE>(0x5B5E18, 0xEB);
		Utils::Hook::Set<BYTE>(0x5B5E64, 0xEB);
		Utils::Hook::Nop(0x5B5E5C, 2);

		// HandleClientHandshake
		Utils::Hook::Set<BYTE>(0x5B6EA5, 0xEB);
		Utils::Hook::Set<BYTE>(0x5B6EF3, 0xEB);
		Utils::Hook::Nop(0x5B6EEB, 2);

		// Allow local connections
		Utils::Hook::Set<BYTE>(0x4D43DA, 0xEB);

		// LobbyID mismatch
		Utils::Hook::Nop(0x4E50D6, 2);
		Utils::Hook::Set<BYTE>(0x4E50DA, 0xEB);

		// causes 'does current Steam lobby match' calls in Steam_JoinLobby to be ignored
		Utils::Hook::Set<BYTE>(0x49D007, 0xEB);

		// functions checking party heartbeat timeouts, cause random issues
		Utils::Hook::Nop(0x4E532D, 5);

		// Steam_JoinLobby call causes migration
		Utils::Hook::Nop(0x5AF851, 5);
		Utils::Hook::Set<BYTE>(0x5AF85B, 0xEB);

		// Allow xpartygo in public lobbies
		Utils::Hook::Set<BYTE>(0x5A969E, 0xEB);

		// Patch party_minplayers to 1 and protect it
		//Utils::Hook(0x4D5D51, Party::RegisterMinPlayers, HOOK_CALL).Install()->Quick();

		Command::Add("connect", [] (Command::Params params)
		{
			if (params.Length() < 2)
			{
				return;
			}

			Party::Connect(Network::Address(params[1]));
		});

		Renderer::OnFrame([] ()
		{
			if (Party::Container.Valid)
			{
				if ((Game::Com_Milliseconds() - Party::Container.JoinTime) > 5000)
				{
					Party::Container.Valid = false;
					Party::ConnectError("Server connection timed out.");
				}
			}
			
			if (Party::Container.AwaitingPlaylist)
			{
				if ((Game::Com_Milliseconds() - Party::Container.RequestTime) > 5000)
				{
					Party::Container.AwaitingPlaylist = false;
					Party::ConnectError("Playlist request timed out.");
				}
			}
		});

		// Basic info handler
		Network::Handle("getInfo", [] (Network::Address address, std::string data)
		{
			int clientCount = 0;
			int maxclientCount = *Game::svs_numclients;
			if (!maxclientCount) maxclientCount = Dvar::Var("sv_maxclients").Get<int>();

			for (int i = 0; i < maxclientCount; i++)
			{
				if (Game::svs_clients[i].state >= 3)
				{
					clientCount++;
				}
			}

			// Ensure line break
			data.append("\n");

			Utils::InfoString info;
			info.Set("challenge", data.substr(0, data.find_first_of("\n")).data());
			info.Set("gamename", "IW4");
			info.Set("hostname", Dvar::Var("sv_hostname").Get<const char*>());
			info.Set("gametype", Dvar::Var("g_gametype").Get<const char*>());
			info.Set("fs_game", Dvar::Var("fs_game").Get<const char*>());
			info.Set("xuid", Utils::VA("%llX", Steam::SteamUser()->GetSteamID().Bits));
			info.Set("clients", Utils::VA("%i", clientCount));
			info.Set("sv_maxclients", Utils::VA("%i", maxclientCount));
			info.Set("protocol", Utils::VA("%i", PROTOCOL));
			info.Set("shortversion", VERSION_STR);
			info.Set("checksum", Utils::VA("%d", Game::Com_Milliseconds()));
			info.Set("mapname", Dvar::Var("mapname").Get<const char*>());

			// Ensure mapname is set
			if (!info.Get("mapname").size())
			{
				info.Set("mapname", Dvar::Var("ui_mapname").Get<const char*>());
			}

			// Set matchtype
			// 0 - No match, connecting not possible
			// 1 - Party, use Steam_JoinLobby to connect
			// 2 - Match, use CL_ConnectFromParty to connect

			if (Dvar::Var("party_host").Get<bool>()) // Party hosting
			{
				info.Set("matchtype", "1");
			}
			else if (Dvar::Var("sv_running").Get<bool>()) // Match hosting
			{
				info.Set("matchtype", "2");
			}
			else
			{
				info.Set("matchtype", "0");
			}

			Network::Send(address, Utils::VA("infoResponse\n\\%s\n", info.Build().data()));
		});

		Network::Handle("infoResponse", [] (Network::Address address, std::string data)
		{
			Utils::InfoString info(data);

			// Handle connection
			if (Party::Container.Valid)
			{
				if (Party::Container.Target == address)
				{
					// Invalidate handler for future packets
					Party::Container.Valid = false;

					int matchType = atoi(info.Get("matchtype").data());

					if (info.Get("challenge") != Party::Container.Challenge)
					{
						Party::ConnectError("Invalid join response: Challenge mismatch.");
					}
					else if (!matchType)
					{
						Party::ConnectError("Server is not hosting a match.");
					}
					// Connect 
					else if (matchType == 1) // Party
					{
						// Send playlist request
						Party::Container.RequestTime = Game::Com_Milliseconds();
						Party::Container.AwaitingPlaylist = true;
						Network::Send(address, "getplaylist\n");
					}
					else if (matchType == 2) // Match
					{
						if (atoi(info.Get("clients").data()) >= atoi(info.Get("sv_maxclients").data()))
						{
							Party::ConnectError("@EXE_SERVERISFULL");
						}
						if (info.Get("mapname") == "" || info.Get("gametype") == "")
						{
							Party::ConnectError("Invalid map or gametype.");
						}
						else
						{
							Dvar::Var("xblive_privatematch").Set(1);
							Game::Menus_CloseAll(Game::uiContext);

							char xnaddr[32];
							Game::CL_ConnectFromParty(0, xnaddr, *address.Get(), 0, 0, info.Get("mapname").data(), info.Get("gametype").data());
						}
					}
					else
					{
						Party::ConnectError("Invalid join response: Unknown matchtype");
					}
				}
			}

			ServerList::Insert(address, info);
		});
	}

	Party::~Party()
	{
		Party::LobbyMap.clear();
	}
}
