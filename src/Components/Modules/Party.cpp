#include "STDInclude.hpp"

namespace Components
{
	Party::JoinContainer Party::Container;
	std::map<uint64_t, Network::Address> Party::LobbyMap;

	SteamID Party::GenerateLobbyId()
	{
		SteamID id;

		id.AccountID = Game::Sys_Milliseconds();
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
		Node::AddNode(target);

		Party::Container.Valid = true;
		Party::Container.AwaitingPlaylist = false;
		Party::Container.JoinTime = Game::Sys_Milliseconds();
		Party::Container.Target = target;
		Party::Container.Challenge = fmt::sprintf("%X", Utils::Cryptography::Rand::GenerateInt());

		Network::SendCommand(Party::Container.Target, "getinfo", Party::Container.Challenge);

		Command::Execute("openmenu popup_reconnectingtoparty");
	}

	const char* Party::GetLobbyInfo(SteamID lobby, std::string key)
	{
		if (Party::LobbyMap.find(lobby.Bits) != Party::LobbyMap.end())
		{
			Network::Address address = Party::LobbyMap[lobby.Bits];

			if (key == "addr")
			{
				return Utils::String::VA("%d", address.GetIP().full);
			}
			else if (key =="port")
			{
				return Utils::String::VA("%d", address.GetPort());
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
		Localization::ClearTemp();
		Command::Execute("closemenu popup_reconnectingtoparty");
		Dvar::Var("partyend_reason").Set(message);
		Command::Execute("openmenu menu_xboxlive_partyended");
	}

	Game::dvar_t* Party::RegisterMinPlayers(const char* name, int /*value*/, int /*min*/, int max, Game::dvar_flag flag, const char* description)
	{
		return Dvar::Register<int>(name, 1, 1, max, Game::dvar_flag::DVAR_FLAG_WRITEPROTECTED | flag, description).Get<Game::dvar_t*>();
	}

	bool Party::PlaylistAwaiting()
	{
		return Party::Container.AwaitingPlaylist;
	}

	void Party::PlaylistContinue()
	{
		Dvar::Var("xblive_privateserver").Set(false);

		// Ensure we can join
		*Game::g_lobbyCreateInProgress = false;

		Party::Container.AwaitingPlaylist = false;

		SteamID id = Party::GenerateLobbyId();

		// Temporary workaround
		// TODO: Patch the 127.0.0.1 -> loopback mapping in the party code
		if (Party::Container.Target.IsLoopback())
		{
			if (*Game::numIP)
			{
				Party::Container.Target.SetIP(*Game::localIP);
				Party::Container.Target.SetType(Game::netadrtype_t::NA_IP);

				Logger::Print("Trying to connect to party with loopback address, using a local ip instead: %s\n", Party::Container.Target.GetCString());
			}
			else
			{
				Logger::Print("Trying to connect to party with loopback address, but no local ip was found.\n");
			}
		}

		Party::LobbyMap[id.Bits] = Party::Container.Target;

		Game::Steam_JoinLobby(id, 0);
	}

	void Party::PlaylistError(std::string error)
	{
		Party::Container.Valid = false;
		Party::Container.AwaitingPlaylist = false;

		Party::ConnectError(error);
	}

	DWORD Party::UIDvarIntStub(char* dvar)
	{
		if (!_stricmp(dvar, "onlinegame"))
		{
			return 0x649E660;
		}

		return Utils::Hook::Call<DWORD(char*)>(0x4D5390)(dvar);
	}

	Party::Party()
	{
		static Game::dvar_t* partyEnable = Dvar::Register<bool>("party_enable", Dedicated::IsEnabled(), Game::dvar_flag::DVAR_FLAG_NONE, "Enable party system").Get<Game::dvar_t*>();
		Dvar::Register<bool>("xblive_privatematch", true, Game::dvar_flag::DVAR_FLAG_WRITEPROTECTED, "").Get<Game::dvar_t*>();

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
		Utils::Hook::Nop(0x5A96BE, 2);

		// Always open lobby menu when connecting
		// It's not possible to entirely patch it via code 
		//Utils::Hook::Set<BYTE>(0x5B1698, 0xEB);
		//Utils::Hook::Nop(0x5029F2, 6);
		//Utils::Hook::SetString(0x70573C, "menu_xboxlive_lobby");

		// Disallow selecting team in private match
		//Utils::Hook::Nop(0x5B2BD8, 6);

		// Force teams, even if not private match
		Utils::Hook::Set<BYTE>(0x487BB2, 0xEB);

		// Force xblive_privatematch 0 and rename it
		//Utils::Hook::Set<BYTE>(0x420A6A, 4);
		Utils::Hook::Set<BYTE>(0x420A6C, 0);
		Utils::Hook::Set<char*>(0x420A6E, "xblive_privateserver");

		// Remove migration shutdown, it causes crashes and will be destroyed when erroring anyways
		Utils::Hook::Nop(0x5A8E1C, 12);
		Utils::Hook::Nop(0x5A8E33, 11);

		// Enable XP Bar
		Utils::Hook(0x62A2A7, Party::UIDvarIntStub, HOOK_CALL).Install()->Quick();

		// Set NAT to open
		Utils::Hook::Set<int>(0x79D898, 1);

		// Disable host migration
		Utils::Hook::Set<BYTE>(0x5B58B2, 0xEB);
		Utils::Hook::Set<BYTE>(0x4D6171, 0);

		// Patch playlist stuff for non-party behavior
		Utils::Hook::Set<Game::dvar_t**>(0x4A4093, &partyEnable);
		Utils::Hook::Set<Game::dvar_t**>(0x4573F1, &partyEnable);
		Utils::Hook::Set<Game::dvar_t**>(0x5B1A0C, &partyEnable);

		// Invert corresponding jumps
		Utils::Hook::Xor<BYTE>(0x4A409B, 1);
		Utils::Hook::Xor<BYTE>(0x4573FA, 1);
		Utils::Hook::Xor<BYTE>(0x5B1A17, 1);

		// Fix xstartlobby
		//Utils::Hook::Set<BYTE>(0x5B71CD, 0xEB);

		// Patch party_minplayers to 1 and protect it
		//Utils::Hook(0x4D5D51, Party::RegisterMinPlayers, HOOK_CALL).Install()->Quick();

		// Set ui_maxclients to sv_maxclients
		Utils::Hook::Set<char*>(0x42618F, "sv_maxclients");
		Utils::Hook::Set<char*>(0x4D3756, "sv_maxclients");
		Utils::Hook::Set<char*>(0x5E3772, "sv_maxclients");

		// Unlatch maxclient dvars
		Utils::Hook::Xor<BYTE>(0x426187, Game::dvar_flag::DVAR_FLAG_LATCHED);
		Utils::Hook::Xor<BYTE>(0x4D374E, Game::dvar_flag::DVAR_FLAG_LATCHED);
		Utils::Hook::Xor<BYTE>(0x5E376A, Game::dvar_flag::DVAR_FLAG_LATCHED);
		Utils::Hook::Xor<DWORD>(0x4261A1, Game::dvar_flag::DVAR_FLAG_LATCHED);
		Utils::Hook::Xor<DWORD>(0x4D376D, Game::dvar_flag::DVAR_FLAG_LATCHED);
		Utils::Hook::Xor<DWORD>(0x5E3789, Game::dvar_flag::DVAR_FLAG_LATCHED);

		// Patch Live_PlayerHasLoopbackAddr
		//Utils::Hook::Set<DWORD>(0x418F30, 0x90C3C033);

		Command::Add("connect", [] (Command::Params params)
		{
			if (params.Length() < 2)
			{
				return;
			}

			Party::Connect(Network::Address(params[1]));
		});
		Command::Add("reconnect", [] (Command::Params)
		{
			Party::Connect(Party::Container.Target);
		});

		Renderer::OnFrame([] ()
		{
			if (Party::Container.Valid)
			{
				if ((Game::Sys_Milliseconds() - Party::Container.JoinTime) > 5000)
				{
					Party::Container.Valid = false;
					Party::ConnectError("Server connection timed out.");
				}
			}
			
			if (Party::Container.AwaitingPlaylist)
			{
				if ((Game::Sys_Milliseconds() - Party::Container.RequestTime) > 5000)
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

			if (maxclientCount)
			{
				for (int i = 0; i < maxclientCount; ++i)
				{
					if (Game::svs_clients[i].state >= 3)
					{
						++clientCount;
					}
				}
			}
			else
			{
				//maxclientCount = Dvar::Var("sv_maxclients").Get<int>();
				maxclientCount = Game::Party_GetMaxPlayers(*Game::partyIngame);
				clientCount = Game::PartyHost_CountMembers(reinterpret_cast<Game::PartyData_s*>(0x1081C00));
			}

			Utils::InfoString info;
			info.Set("challenge", Utils::ParseChallenge(data));
			info.Set("gamename", "IW4");
			info.Set("hostname", Dvar::Var("sv_hostname").Get<const char*>());
			info.Set("gametype", Dvar::Var("g_gametype").Get<const char*>());
			info.Set("fs_game", Dvar::Var("fs_game").Get<const char*>());
			info.Set("xuid", fmt::sprintf("%llX", Steam::SteamUser()->GetSteamID().Bits));
			info.Set("clients", fmt::sprintf("%i", clientCount));
			info.Set("sv_maxclients", fmt::sprintf("%i", maxclientCount));
			info.Set("protocol", fmt::sprintf("%i", PROTOCOL));
			info.Set("shortversion", SHORTVERSION);
			info.Set("checksum", fmt::sprintf("%d", Game::Sys_Milliseconds()));
			info.Set("mapname", Dvar::Var("mapname").Get<const char*>());
			info.Set("isPrivate", (Dvar::Var("g_password").Get<std::string>().size() ? "1" : "0"));
			info.Set("hc", (Dvar::Var("g_hardcore").Get<bool>() ? "1" : "0"));
			info.Set("securityLevel", fmt::sprintf("%i", Dvar::Var("sv_securityLevel").Get<int>()));
			info.Set("sv_running", (Dvar::Var("sv_running").Get<bool>() ? "1" : "0"));

			// Ensure mapname is set
			if (info.Get("mapname").empty() || (Dvar::Var("party_enable").Get<bool>() && Dvar::Var("party_host").Get<bool>() && !Dvar::Var("sv_running").Get<bool>()))
			{
				info.Set("mapname", Dvar::Var("ui_mapname").Get<const char*>());
			}

			// Set matchtype
			// 0 - No match, connecting not possible
			// 1 - Party, use Steam_JoinLobby to connect
			// 2 - Match, use CL_ConnectFromParty to connect

			if (Dvar::Var("party_enable").Get<bool>() && Dvar::Var("party_host").Get<bool>()) // Party hosting
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

			Network::SendCommand(address, "infoResponse", "\\" + info.Build());
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
					Party::Container.Info = info;

					Party::Container.MatchType = atoi(info.Get("matchtype").data());
					uint32_t securityLevel = static_cast<uint32_t>(atoi(info.Get("securityLevel").data()));

					if (info.Get("challenge") != Party::Container.Challenge)
					{
						Party::ConnectError("Invalid join response: Challenge mismatch.");
					}
					else if (securityLevel > Auth::GetSecurityLevel())
					{
						//Party::ConnectError(Utils::VA("Your security level (%d) is lower than the server's (%d)", Auth::GetSecurityLevel(), securityLevel));
						Command::Execute("closemenu popup_reconnectingtoparty");
						Auth::IncreaseSecurityLevel(securityLevel, "reconnect");
					}
					else if (!Party::Container.MatchType)
					{
						Party::ConnectError("Server is not hosting a match.");
					}
					else if(Party::Container.MatchType > 2 || Party::Container.MatchType < 0)
					{
						Party::ConnectError("Invalid join response: Unknown matchtype");
					}
					else if(!info.Get("fs_game").empty() && Dvar::Var("fs_game").Get<std::string>() != info.Get("fs_game"))
					{
						Command::Execute("closemenu popup_reconnectingtoparty");
						Download::InitiateClientDownload(info.Get("fs_game"));
					}
					else if (!Dvar::Var("fs_game").Get<std::string>().empty() && info.Get("fs_game").empty())
					{
						Dvar::Var("fs_game").Set("");

						if (Dvar::Var("cl_modVidRestart").Get<bool>())
						{
							Command::Execute("vid_restart", false);
						}

						Command::Execute("reconnect", false);
					}
					else
					{
						if (Party::Container.MatchType == 1) // Party
						{
							// Send playlist request
							Party::Container.RequestTime = Game::Sys_Milliseconds();
							Party::Container.AwaitingPlaylist = true;
							Network::SendCommand(Party::Container.Target, "getplaylist");

							// This is not a safe method
							// TODO: Fix actual error!
							if (Game::CL_IsCgameInitialized())
							{
								Command::Execute("disconnect", true);
							}
						}
						else if (Party::Container.MatchType == 2) // Match
						{
							if (atoi(Party::Container.Info.Get("clients").data()) >= atoi(Party::Container.Info.Get("sv_maxclients").data()))
							{
								Party::ConnectError("@EXE_SERVERISFULL");
							}
							if (Party::Container.Info.Get("mapname") == "" || Party::Container.Info.Get("gametype") == "")
							{
								Party::ConnectError("Invalid map or gametype.");
							}
							else
							{
								Dvar::Var("xblive_privateserver").Set(true);

								Game::Menus_CloseAll(Game::uiContext);

								Game::_XSESSION_INFO hostInfo;
								Game::CL_ConnectFromParty(0, &hostInfo, *Party::Container.Target.Get(), 0, 0, Party::Container.Info.Get("mapname").data(), Party::Container.Info.Get("gametype").data());
							}
						}
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
