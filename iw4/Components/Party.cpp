#include "..\STDInclude.hpp"

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

	void Party::Connect(Network::Address target)
	{
		Party::Container.Valid = true;
		Party::Container.JoinTime = Game::Com_Milliseconds();
		Party::Container.Target = target;
		Party::Container.Challenge = Utils::VA("%X", Party::Container.JoinTime);

		Network::Send(Game::NS_CLIENT, Party::Container.Target, Utils::VA("getinfo %s\n", Party::Container.Challenge.data()));

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
		Utils::Hook::Nop(0x4CAA5D, 5); 

		// Steam_JoinLobby call causes migration
		Utils::Hook::Nop(0x5AF851, 5);
		Utils::Hook::Set<BYTE>(0x5AF85B, 0xEB);

		Command::Add("connect", [] (Command::Params params)
		{
			if (params.Length() < 2)
			{
				return;
			}

			Party::Connect(Network::Address(params[1]));
		});

		Command::Add("connect2", [] (Command::Params params)
		{
			if (params.Length() < 2)
			{
				return;
			}

			Network::Address address(params[1]);

			SteamID id = Party::GenerateLobbyId();

			Party::LobbyMap[id.Bits] = address;

			OutputDebugStringA(Utils::VA("Mapping %llX -> %s", id.Bits, address.GetString()));

			Game::Steam_JoinLobby(id, 0);

			// Callback not registered on first try
			// TODO: Fix :D
			if (Party::LobbyMap.size() <= 1) Game::Steam_JoinLobby(id, 0); 
		});

		Renderer::OnFrame([] ()
		{
			if (!Party::Container.Valid) return;

			if ((Game::Com_Milliseconds() - Party::Container.JoinTime) > 5000)
			{
				Party::Container.Valid = false;

				Command::Execute("closemenu popup_reconnectingtoparty");
				Dvar::Var("partyend_reason").Set("Server connection timed out.");
				Command::Execute("openmenu menu_xboxlive_partyended");
			}
		});

		// Basic info handler
		Network::Handle("getInfo", [] (Network::Address address, std::string data)
		{
			int clientCount = 0;

			for (int i = 0; i < *Game::svs_numclients; i++)
			{
				if (Game::svs_clients[i].state >= 3)
				{
					clientCount++;
				}
			}

			Utils::InfoString info;
			info.Set("challenge", data.substr(0, data.find_first_of("\n")).data());
			info.Set("gamename", "IW4");
			info.Set("hostname", Dvar::Var("sv_hostname").Get<const char*>());
			info.Set("mapname", Dvar::Var("mapname").Get<const char*>());
			info.Set("gametype", Dvar::Var("g_gametype").Get<const char*>());
			info.Set("fs_game", Dvar::Var("fs_game").Get<const char*>());
			info.Set("xuid", Utils::VA("%llX", Steam::SteamUser()->GetSteamID().Bits));
			info.Set("clients", Utils::VA("%i", clientCount));
			info.Set("sv_maxclients", Utils::VA("%i", *Game::svs_numclients));

			Network::Send(Game::NS_CLIENT, address, Utils::VA("infoResponse\n%s\n", info.Build().data()));
		});

		Network::Handle("infoResponse", [] (Network::Address address, std::string data)
		{
			// Handle connection
			if (Party::Container.Valid)
			{
				if (Party::Container.Target == address)
				{
					// Invalidate handler for future packets
					Party::Container.Valid = false;

					Utils::InfoString info(data);

					OutputDebugStringA(data.data());

					if (info.Get("challenge") != Party::Container.Challenge)
					{
						OutputDebugStringA(Utils::VA("\"%s\" vs. \"%s\"", info.Get("challenge").data(), Party::Container.Challenge.data()));
						Command::Execute("closemenu popup_reconnectingtoparty");
						Dvar::Var("partyend_reason").Set("Invalid join response: Challenge mismatch.");
						Command::Execute("openmenu menu_xboxlive_partyended");
					}
					else if (atoi(info.Get("clients").data()) >= atoi(info.Get("sv_maxclients").data()))
					{
						Command::Execute("closemenu popup_reconnectingtoparty");
						Dvar::Var("partyend_reason").Set("@EXE_SERVERISFULL");
						Command::Execute("openmenu menu_xboxlive_partyended");
					}
					else if (info.Get("mapname") == "" || info.Get("gametype") == "")
					{
						Command::Execute("closemenu popup_reconnectingtoparty");
						Dvar::Var("partyend_reason").Set("Invalid map or gametype.");
						Command::Execute("openmenu menu_xboxlive_partyended");
					}
					else
					{
						Dvar::Var("xblive_privatematch").Set(1);
						Game::Menus_CloseAll(Game::uiContext);

						char xnaddr[32];
						Game::CL_ConnectFromParty(0, xnaddr, *address.Get(), 0, 0, info.Get("mapname").data(), info.Get("gametype").data());
					}
				}
			}
		});
	}

	Party::~Party()
	{
		Party::LobbyMap.clear();
	}
}
