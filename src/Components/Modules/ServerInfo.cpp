#include <STDInclude.hpp>
#include <Utils/InfoString.hpp>

#include "Friends.hpp"
#include "Gamepad.hpp"
#include "Party.hpp"
#include "ServerInfo.hpp"
#include "ServerList.hpp"
#include "UIFeeder.hpp"
#include "Voice.hpp"

#include <version.hpp>

namespace Components
{
	ServerInfo::Container ServerInfo::PlayerContainer;

	unsigned int ServerInfo::GetPlayerCount()
	{
		return PlayerContainer.playerList.size();
	}

	const char* ServerInfo::GetPlayerText(unsigned int index, int column)
	{
		if (index < PlayerContainer.playerList.size())
		{
			switch (column)
			{
			case 0:
				return Utils::String::VA("%d", index);
			case 1:
				return PlayerContainer.playerList[index].name.data();
			case 2:
				return Utils::String::VA("%d", PlayerContainer.playerList[index].score);
			case 3:
				return Utils::String::VA("%d", PlayerContainer.playerList[index].ping);
			default:
				break;
			}
		}

		return "";
	}

	void ServerInfo::SelectPlayer(unsigned int index)
	{
		PlayerContainer.currentPlayer = index;
	}

	void ServerInfo::ServerStatus([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
	{
		PlayerContainer.currentPlayer = 0;
		PlayerContainer.playerList.clear();

		auto* serverInfo = ServerList::GetCurrentServer();

		if (info && serverInfo)
		{
			Dvar::Var("uiSi_ServerName").set(serverInfo->hostname);
			Dvar::Var("uiSi_MaxClients").set(serverInfo->clients);
			Dvar::Var("uiSi_Version").set(serverInfo->version);
			Dvar::Var("uiSi_SecurityLevel").set(serverInfo->securityLevel);
			Dvar::Var("uiSi_isPrivate").set(serverInfo->password ? "@MENU_YES" : "@MENU_NO");
			Dvar::Var("uiSi_Hardcore").set(serverInfo->hardcore ? "@MENU_ENABLED" : "@MENU_DISABLED");
			Dvar::Var("uiSi_KillCam").set("@MENU_NO");
			Dvar::Var("uiSi_ffType").set("@MENU_DISABLED");
			Dvar::Var("uiSi_MapName").set(serverInfo->mapname);
			Dvar::Var("uiSi_MapNameLoc").set(Localization::LocalizeMapName(serverInfo->mapname.data()));
			Dvar::Var("uiSi_GameType").set(Game::UI_LocalizeGameType(serverInfo->gametype.data()));
			Dvar::Var("uiSi_ModName").set("");
			Dvar::Var("uiSi_aimAssist").set(serverInfo->aimassist ? "@MENU_YES" : "@MENU_NO");
			Dvar::Var("uiSi_voiceChat").set(serverInfo->voice ? "@MENU_YES" : "@MENU_NO");

			if (serverInfo->mod.size() > 5)
			{
				Dvar::Var("uiSi_ModName").set(serverInfo->mod.data() + 5);
			}

			PlayerContainer.target = serverInfo->addr;
			Network::SendCommand(PlayerContainer.target, "getstatus");
		}
	}

	void ServerInfo::DrawScoreboardInfo(int localClientNum)
	{
		Game::Font_s* font = Game::R_RegisterFont("fonts/bigfont", 0);
		const auto* cxt = Game::ScrPlace_GetActivePlacement(localClientNum);

		auto addressText = Network::Address(*Game::connectedHost).getString();
		if (addressText == "0.0.0.0:0"s || addressText == "loopback"s)
		{
			addressText = "Listen Server"s;
		}

		// Get x positions
		auto y = (480.0f - (*Game::cg_scoreboardHeight)->current.value) * 0.5f;
		y += (*Game::cg_scoreboardHeight)->current.value + 6.0f;

		const auto x = 320.0f - (*Game::cg_scoreboardWidth)->current.value * 0.5f;
		const auto x2 = 320.0f + (*Game::cg_scoreboardWidth)->current.value * 0.5f;

		// Draw only when stream friendly ui is not enabled
		if (!Friends::UIStreamFriendly.get<bool>())
		{
			constexpr auto fontSize = 0.35f;
			Game::UI_DrawText(cxt, reinterpret_cast<const char*>(0x7ED3F8), std::numeric_limits<int>::max(), font, x, y, 0, 0, fontSize, reinterpret_cast<float*>(0x747F34), 3);
			Game::UI_DrawText(cxt, addressText.data(), std::numeric_limits<int>::max(), font, x2 - Game::UI_TextWidth(addressText.data(), 0, font, fontSize), y, 0, 0, fontSize, reinterpret_cast<float*>(0x747F34), 3);
		}
	}

	__declspec(naked) void ServerInfo::DrawScoreboardStub()
	{
		__asm
		{
			pushad
			push eax
			call DrawScoreboardInfo
			pop eax
			popad

			push 591B70h
			retn
		}
	}

	Utils::InfoString ServerInfo::GetHostInfo()
	{
		Utils::InfoString info;

		info.set("admin", Dvar::Var("_Admin").get<const char*>());
		info.set("website", Dvar::Var("_Website").get<const char*>());
		info.set("email", Dvar::Var("_Email").get<const char*>());
		info.set("location", Dvar::Var("_Location").get<const char*>());

		return info;
	}

	Utils::InfoString ServerInfo::GetInfo()
	{
		auto maxClientCount = *Game::svs_clientCount;
		const auto* password = *Game::g_password ? (*Game::g_password)->current.string : "";

		if (!maxClientCount)
		{
			maxClientCount = *Game::party_maxplayers ? (*Game::party_maxplayers)->current.integer : 18;
		}

		Utils::InfoString info(Game::Dvar_InfoString_Big(Game::DVAR_SERVERINFO));
		info.set("gamename", "IW4");
		info.set("sv_maxclients", std::to_string(maxClientCount));
		info.set("protocol", std::to_string(PROTOCOL));
		info.set("version", REVISION_STR);
		info.set("version", (*Game::version)->current.string);
		info.set("mapname", (*Game::sv_mapname)->current.string);
		info.set("isPrivate", *password ? "1" : "0");
		info.set("checksum", Utils::String::VA("%X", Utils::Cryptography::JenkinsOneAtATime::Compute(std::to_string(Game::Sys_Milliseconds()))));
		info.set("aimAssist", (Gamepad::sv_allowAimAssist.get<bool>() ? "1" : "0"));
		info.set("voiceChat", (Voice::SV_VoiceEnabled() ? "1" : "0"));

		// Ensure mapname is set
		if (info.get("mapname").empty())
		{
			info.set("mapname", (*Game::ui_mapname)->current.string);
		}

		// Set matchtype
		// 0 - No match, connecting not possible
		// 1 - Party, use Steam_JoinLobby to connect
		// 2 - Match, use CL_ConnectFromParty to connect

		if (Party::IsEnabled() && Dvar::Var("party_host").get<bool>()) // Party hosting
		{
			info.set("matchtype", "1");
		}
		else if (Dedicated::IsRunning()) // Match hosting
		{
			info.set("matchtype", "2");
		}
		else
		{
			info.set("matchtype", "0");
		}

		return info;
	}

	ServerInfo::ServerInfo()
	{
		PlayerContainer.currentPlayer = 0;

		// Draw IP and hostname on the scoreboard
		Utils::Hook(0x4FC6EA, DrawScoreboardStub, HOOK_CALL).install()->quick();

		// Ignore native getStatus implementation
		Utils::Hook::Nop(0x62654E, 6);

		// Add uiscript
		UIScript::Add("ServerStatus", ServerStatus);

		// Add uifeeder
		UIFeeder::Add(13.0f, GetPlayerCount, GetPlayerText, SelectPlayer);

		Network::OnClientPacket("getStatus", [](const Network::Address& address, [[maybe_unused]] const std::string& data)
		{
			std::string playerList;

			Utils::InfoString info = GetInfo();
			info.set("challenge", Utils::ParseChallenge(data));

			for (std::size_t i = 0; i < Game::MAX_CLIENTS; ++i)
			{
				auto score = 0;
				auto ping = 0;
				std::string name;

				if (Dedicated::IsRunning())
				{
					if (Game::svs_clients[i].header.state < Game::CS_ACTIVE) continue;
					if (!Game::svs_clients[i].gentity || !Game::svs_clients[i].gentity->client) continue;

					const auto* client = Game::svs_clients[i].gentity->client;
					const auto team = client->sess.cs.team;
					if (Game::svs_clients[i].bIsTestClient || team == Game::TEAM_SPECTATOR)
					{
						continue;
					}

					score = Game::SV_GameClientNum_Score(static_cast<int>(i));
					ping = Game::svs_clients[i].ping;
					name = Game::svs_clients[i].name;
				}
				else
				{
					// Score and ping are irrelevant
					const auto* namePtr = Game::PartyHost_GetMemberName(reinterpret_cast<Game::PartyData*>(0x1081C00), i);
					if (!namePtr || !*namePtr) continue;

					name = namePtr;
				}

				playerList.append(std::format("{} {} \"{}\"\n", score, ping, name));
			}

			Network::SendCommand(address, "statusResponse", info.build() + "\n"s + playerList + "\n"s);
		});

		Network::OnClientPacket("statusResponse", [](const Network::Address& address, [[maybe_unused]] const std::string& data)
		{
			if (PlayerContainer.target != address)
			{
				return;
			}

			const auto pos = data.find_first_of('\n');
			if (pos == std::string::npos)
			{
				return;
			}

			const Utils::InfoString info(data.substr(0, pos));

			Dvar::Var("uiSi_ServerName").set(info.get("sv_hostname"));
			Dvar::Var("uiSi_MaxClients").set(info.get("sv_maxclients"));
			Dvar::Var("uiSi_Version").set(info.get("version"));
			Dvar::Var("uiSi_SecurityLevel").set(info.get("sv_securityLevel"));
			Dvar::Var("uiSi_isPrivate").set(info.get("isPrivate") == "0" ? "@MENU_NO" : "@MENU_YES");
			Dvar::Var("uiSi_Hardcore").set(info.get("g_hardcore") == "0" ? "@MENU_DISABLED" : "@MENU_ENABLED");
			Dvar::Var("uiSi_KillCam").set(info.get("scr_game_allowkillcam") == "0" ? "@MENU_NO" : "@MENU_YES");
			Dvar::Var("uiSi_MapName").set(info.get("mapname"));
			Dvar::Var("uiSi_MapNameLoc").set(Localization::LocalizeMapName(info.get("mapname").data()));
			Dvar::Var("uiSi_GameType").set(Game::UI_LocalizeGameType(info.get("g_gametype").data()));
			Dvar::Var("uiSi_ModName").set("");
			Dvar::Var("uiSi_aimAssist").set(info.get("aimAssist") == "0" ? "@MENU_DISABLED" : "@MENU_ENABLED");
			Dvar::Var("uiSi_voiceChat").set(info.get("voiceChat") == "0" ? "@MENU_DISABLED" : "@MENU_ENABLED");

			switch (std::strtol(info.get("scr_team_fftype").data(), nullptr, 10))
			{
			default:
				Dvar::Var("uiSi_ffType").set("@MENU_DISABLED");
				break;
			case 1:
				Dvar::Var("uiSi_ffType").set("@MENU_ENABLED");
				break;
			case 2:
				Dvar::Var("uiSi_ffType").set("@MPUI_RULES_REFLECT");
				break;
			case 3:
				Dvar::Var("uiSi_ffType").set("@MPUI_RULES_SHARED");
				break;
			}

			if (Utils::String::StartsWith(info.get("fs_game"), "mods/"))
			{
				const auto mod = info.get("fs_game");
				Dvar::Var("uiSi_ModName").set(mod.substr(5));
			}

			const auto lines = Utils::String::Split(data, '\n');

			if (lines.size() <= 1) return;

			for (std::size_t i = 1; i < lines.size(); ++i)
			{
				Container::Player player;

				std::string currentData = lines[i];

				if (currentData.size() < 3) continue;

				// Insert score
				player.score = std::strtol(currentData.substr(0, currentData.find_first_of(' ')).data(), nullptr, 10);

				// Remove score
				currentData = currentData.substr(currentData.find_first_of(' ') + 1);

				// Insert ping
				player.ping = std::strtol(currentData.substr(0, currentData.find_first_of(' ')).data(), nullptr, 10);

				// Remove ping
				currentData = currentData.substr(currentData.find_first_of(' ') + 1);

				if (currentData[0] == '\"')
				{
					currentData = currentData.substr(1);
				}

				if (currentData.back() == '\"')
				{
					currentData.pop_back();
				}

				player.name = currentData;

				PlayerContainer.playerList.push_back(player);
			}
		});
	}

	ServerInfo::~ServerInfo()
	{
		PlayerContainer.playerList.clear();
	}
}
