#include "..\STDInclude.hpp"

namespace Components
{
	Party::JoinContainer Party::Container;

	void Party::Connect(Network::Address target)
	{
		Party::Container.Valid = true;
		Party::Container.JoinTime = Game::Com_Milliseconds();
		Party::Container.Target = target;
		Party::Container.Challenge = Utils::VA("%X", Party::Container.JoinTime);

		Network::Send(Game::NS_CLIENT, Party::Container.Target, Utils::VA("getinfo %s\n", Party::Container.Challenge.data()));

		Command::Execute("openmenu popup_reconnectingtoparty");
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
			info.Set("xuid", Utils::VA("%llX", Steam::SteamUser()->GetSteamID().m_Bits));
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
						Game::Menus_CloseAll(0x62E2858);

						char xnaddr[32];
						Game::CL_ConnectFromParty(0, xnaddr, *address.Get(), 0, 0, info.Get("mapname").data(), info.Get("gametype").data());
					}
				}
			}
		});
	}

	Party::~Party()
	{

	}
}
