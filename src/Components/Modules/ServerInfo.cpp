#include "STDInclude.hpp"

namespace Components
{
	ServerInfo::Container ServerInfo::PlayerContainer;

	unsigned int ServerInfo::GetPlayerCount()
	{
		return ServerInfo::PlayerContainer.PlayerList.size();
	}

	const char* ServerInfo::GetPlayerText(unsigned int index, int column)
	{
		if (index < ServerInfo::PlayerContainer.PlayerList.size())
		{
			switch (column)
			{
			case 0:
				return Utils::String::VA("%d", index);

			case 1:
				return ServerInfo::PlayerContainer.PlayerList[index].Name.data();

			case 2:
				return Utils::String::VA("%d", ServerInfo::PlayerContainer.PlayerList[index].Score);

			case 3:
				return Utils::String::VA("%d", ServerInfo::PlayerContainer.PlayerList[index].Ping);
			}
		}

		return "";
	}

	void ServerInfo::SelectPlayer(unsigned int index)
	{
		ServerInfo::PlayerContainer.CurrentPlayer = index;
	}

	void ServerInfo::ServerStatus()
	{
		ServerInfo::PlayerContainer.CurrentPlayer = 0;
		ServerInfo::PlayerContainer.PlayerList.clear();

		ServerList::ServerInfo* info = ServerList::GetCurrentServer();

		if(info)
		{
			Dvar::Var("uiSi_ServerName").Set(info->Hostname);
			Dvar::Var("uiSi_MaxClients").Set(info->Clients);
			Dvar::Var("uiSi_Version").Set(info->Shortversion);
			Dvar::Var("uiSi_SecurityLevel").Set(info->SecurityLevel);
			Dvar::Var("uiSi_isPrivate").Set(info->Password ? "@MENU_YES" : "@MENU_NO");
			Dvar::Var("uiSi_Hardcore").Set(info->Hardcore ? "@MENU_ENABLED" : "@MENU_DISABLED");
			Dvar::Var("uiSi_KillCam").Set("@MENU_NO");
			Dvar::Var("uiSi_ffType").Set("@MENU_DISABLED");
			Dvar::Var("uiSi_MapName").Set(info->Mapname);
			Dvar::Var("uiSi_MapNameLoc").Set(Game::UI_LocalizeMapName(info->Mapname.data()));
			Dvar::Var("uiSi_GameType").Set(Game::UI_LocalizeGameType(info->Gametype.data()));
			Dvar::Var("uiSi_ModName").Set("");

			if (info->Mod.size() > 5)
			{
				Dvar::Var("uiSi_ModName").Set(info->Mod.data() + 5);
			}

			ServerInfo::PlayerContainer.Target = info->Addr;
			Network::SendCommand(ServerInfo::PlayerContainer.Target, "getstatus");
		}
	}

	void ServerInfo::DrawScoreboardInfo(void* a1)
	{
		Game::Font* font = Game::R_RegisterFont("fonts/bigfont");
		void* cxt = Game::UI_GetContext(a1);

		std::string addressText = Network::Address(*Game::connectedHost).GetString();
		if (addressText == "0.0.0.0:0" || addressText == "loopback") addressText = "Listen Server";

		// get x positions
		float fontSize = 0.35f;
		float y = (480.0f - Dvar::Var("cg_scoreboardHeight").Get<float>()) * 0.5f;
		y += Dvar::Var("cg_scoreboardHeight").Get<float>() + 6.0f;

		float x = 320.0f - Dvar::Var("cg_scoreboardWidth").Get<float>() * 0.5f;
		float x2 = 320.0f + Dvar::Var("cg_scoreboardWidth").Get<float>() * 0.5f;

		Game::UI_DrawText(cxt, reinterpret_cast<const char*>(0x7ED3F8), 0x7FFFFFFF, font, x, y, 0, 0, fontSize, reinterpret_cast<float*>(0x747F34), 3);
		Game::UI_DrawText(cxt, addressText.data(), 0x7FFFFFFF, font, x2 - Game::UI_TextWidth(addressText.data(), 0, font, fontSize), y, 0, 0, fontSize, reinterpret_cast<float*>(0x747F34), 3);
	}

	__declspec(naked) void ServerInfo::DrawScoreboardStub()
	{
		__asm 
		{
			push eax
			call ServerInfo::DrawScoreboardInfo
			pop eax
			mov ecx, 591B70h
			jmp ecx
		}
	}

	Utils::InfoString ServerInfo::GetInfo()
	{
		int maxclientCount = *Game::svs_numclients;

		if (!maxclientCount)
		{
			//maxclientCount = Dvar::Var("sv_maxclients").Get<int>();
			maxclientCount = Game::Party_GetMaxPlayers(*Game::partyIngame);
		}

		Utils::InfoString info(Game::Dvar_InfoString_Big(1024));
		info.Set("gamename", "IW4");
		info.Set("sv_maxclients", fmt::sprintf("%i", maxclientCount));
		info.Set("protocol", fmt::sprintf("%i", PROTOCOL));
		info.Set("shortversion", VERSION_STR);
		info.Set("mapname", Dvar::Var("mapname").Get<const char*>());
		info.Set("isPrivate", (Dvar::Var("g_password").Get<std::string>().empty() ? "0" : "1"));
		info.Set("checksum", fmt::sprintf("%X", Utils::Cryptography::JenkinsOneAtATime::Compute(fmt::sprintf("%u", Game::Sys_Milliseconds()))));

		// Ensure mapname is set
		if (info.Get("mapname").empty())
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

		return info;
	}

	ServerInfo::ServerInfo()
	{
		ServerInfo::PlayerContainer.CurrentPlayer = 0;
		ServerInfo::PlayerContainer.PlayerList.clear();

		// Draw IP and hostname on the scoreboard
		Utils::Hook(0x4FC6EA, ServerInfo::DrawScoreboardStub, HOOK_CALL).Install()->Quick();

		// Ignore native getStatus implementation
		Utils::Hook::Nop(0x62654E, 6);

		// Add uiscript
		UIScript::Add("ServerStatus", ServerInfo::ServerStatus);

		// Add uifeeder
		UIFeeder::Add(13.0f, ServerInfo::GetPlayerCount, ServerInfo::GetPlayerText, ServerInfo::SelectPlayer);

		Network::Handle("getStatus", [] (Network::Address address, std::string data)
		{
			std::string playerList;

			Utils::InfoString info = ServerInfo::GetInfo();
			info.Set("challenge", Utils::ParseChallenge(data));

			for (int i = 0; i < atoi(info.Get("sv_maxclients").data()); ++i) // Maybe choose 18 here? 
			{
				int score = 0;
				int ping = 0;
				std::string name;

				if (Dvar::Var("sv_running").Get<bool>())
				{
					if (Game::svs_clients[i].state < 3) continue;

					score = Game::SV_GameClientNum_Score(i);
					ping = Game::svs_clients[i].ping;
					name = Game::svs_clients[i].name;
				}
				else
				{
					// Score and ping are irrelevant
					const char* namePtr = Game::PartyHost_GetMemberName(reinterpret_cast<Game::PartyData_t*>(0x1081C00), i);
					if (!namePtr || !namePtr[0]) continue;

					name = namePtr;
				}

				playerList.append(fmt::sprintf("%i %i \"%s\"\n", score, ping, name.data()));
			}

			Network::SendCommand(address, "statusResponse", "\\" + info.Build() + "\n" + playerList + "\n");
		});

		Network::Handle("statusResponse", [] (Network::Address address, std::string data)
		{
			if (ServerInfo::PlayerContainer.Target == address)
			{
				Utils::InfoString info(data.substr(0, data.find_first_of("\n")));

				Dvar::Var("uiSi_ServerName").Set(info.Get("sv_hostname"));
				Dvar::Var("uiSi_MaxClients").Set(info.Get("sv_maxclients"));
				Dvar::Var("uiSi_Version").Set(info.Get("shortversion"));
				Dvar::Var("uiSi_SecurityLevel").Set(info.Get("sv_securityLevel"));
				Dvar::Var("uiSi_isPrivate").Set(info.Get("isPrivate") == "0" ? "@MENU_NO" : "@MENU_YES");
				Dvar::Var("uiSi_Hardcore").Set(info.Get("g_hardcore") == "0" ? "@MENU_DISABLED" : "@MENU_ENABLED");
				Dvar::Var("uiSi_KillCam").Set(info.Get("scr_game_allowkillcam") == "0" ? "@MENU_NO" : "@MENU_YES");
				Dvar::Var("uiSi_MapName").Set(info.Get("mapname"));
				Dvar::Var("uiSi_MapNameLoc").Set(Game::UI_LocalizeMapName(info.Get("mapname").data()));
				Dvar::Var("uiSi_GameType").Set(Game::UI_LocalizeGameType(info.Get("g_gametype").data()));
				Dvar::Var("uiSi_ModName").Set("");

				switch (atoi(info.Get("scr_team_fftype").data()))
				{
				default:
					Dvar::Var("uiSi_ffType").Set("@MENU_DISABLED");
					break;

				case 1:
					Dvar::Var("uiSi_ffType").Set("@MENU_ENABLED");
					break;

				case 2:
					Dvar::Var("uiSi_ffType").Set("@MPUI_RULES_REFLECT");
					break;

				case 3:
					Dvar::Var("uiSi_ffType").Set("@MPUI_RULES_SHARED");
					break;
				}

				if (info.Get("fs_game").size() > 5)
				{
					Dvar::Var("uiSi_ModName").Set(info.Get("fs_game").data() + 5);
				}

				auto lines = Utils::String::Explode(data, '\n');

				if (lines.size() <= 1) return;

				for (unsigned int i = 1; i < lines.size(); ++i)
				{
					ServerInfo::Container::Player player;

					std::string currentData = lines[i];

					if (currentData.size() < 3) continue;

					// Insert score
					player.Score = atoi(currentData.substr(0, currentData.find_first_of(" ")).data());

					// Remove score
					currentData = currentData.substr(currentData.find_first_of(" ") + 1);

					// Insert ping
					player.Ping = atoi(currentData.substr(0, currentData.find_first_of(" ")).data());

					// Remove ping
					currentData = currentData.substr(currentData.find_first_of(" ") + 1);

					if (currentData[0] == '\"')
					{
						currentData = currentData.substr(1);
					}

					if (currentData[currentData.size() - 1] == '\"')
					{
						currentData.pop_back();
					}

					player.Name = currentData;

					ServerInfo::PlayerContainer.PlayerList.push_back(player);
				}
			}
		});
	}

	ServerInfo::~ServerInfo()
	{
		ServerInfo::PlayerContainer.PlayerList.clear();
	}
}
