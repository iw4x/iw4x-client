#include <STDInclude.hpp>
#include "ClientCommand.hpp"
#include "MapRotation.hpp"
#include "Vote.hpp"

using namespace Utils::String;

namespace Components
{
	std::unordered_map<std::string, Vote::CommandHandler> Vote::VoteCommands =
	{
		{"map_restart", HandleMapRestart},
		{"map_rotate", HandleMapRotate},
		{"typemap", HandleTypemap},
		{"map", HandleMap},
		{"g_gametype", HandleGametype},
		{"kick", HandleKick},
		{"tempBanUser", HandleKick}
	};

	void Vote::DisplayVote(const Game::gentity_s* ent)
	{
		Game::SV_GameSendServerCommand(-1, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_CALLEDAVOTE\x15%s\"", 0x65, ent->client->sess.cs.name));
		Game::level->voteNo = 0;
		Game::level->voteYes = 1;
		Game::level->voteTime = Game::level->time + 30000;

		for (auto i = 0; i < Game::level->maxclients; ++i)
		{
			Game::level->clients[i].ps.eFlags &= ~Game::EF_VOTED;
		}

		ent->client->ps.eFlags |= Game::EF_VOTED;
		Game::SV_SetConfigstring(Game::CS_VOTE_TIME, VA("%i %i", Game::level->voteTime, *Game::sv_serverId_value));
		Game::SV_SetConfigstring(Game::CS_VOTE_STRING, Game::level->voteDisplayString);
		Game::SV_SetConfigstring(Game::CS_VOTE_YES, VA("%i", Game::level->voteYes));
		Game::SV_SetConfigstring(Game::CS_VOTE_NO, VA("%i", Game::level->voteNo));
	}

	bool Vote::IsInvalidVoteString(const std::string& input)
	{
		static const char* separators[] = { "\n", "\r", ";" };

		return std::ranges::any_of(separators,
			[&](const std::string& str) { return input.find(str) != std::string::npos; });
	}

	bool Vote::HandleMapRestart([[maybe_unused]] const Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
	{
		sprintf_s(Game::level->voteString, "fast_restart");
		sprintf_s(Game::level->voteDisplayString, "GAME_VOTE_MAPRESTART");
		return true;
	}

	bool Vote::HandleMapRotate([[maybe_unused]] const Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
	{
		sprintf_s(Game::level->voteString, "%s", params->get(1));
		sprintf_s(Game::level->voteDisplayString, "GAME_VOTE_NEXTMAP");
		return true;
	}

	bool Vote::HandleTypemap([[maybe_unused]] const Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
	{
		char arg2[0x100]{};
		char arg3[0x100]{};

		strncpy_s(arg2, params->get(2), _TRUNCATE);
		strncpy_s(arg3, params->get(3), _TRUNCATE);

		// This prevents abuse
		if (!MapRotation::Contains("map", arg3))
		{
			Game::SV_GameSendServerCommand(ent - Game::g_entities, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_NOTONROTATION\"", 0x65));
			return false;
		}

		if (!Game::Scr_IsValidGameType(arg2))
		{
			Game::SV_GameSendServerCommand(ent - Game::g_entities, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_INVALIDGAMETYPE\"", 0x65));
			return false;
		}

		if (!std::strcmp(arg2, (*Game::g_gametype)->current.string))
		{
			arg2[0] = '\0';
		}

		const auto* mapname = Game::Dvar_RegisterString("mapname", "", Game::DVAR_ROM | Game::DVAR_SERVERINFO, "Current map name");
		if (!std::strcmp(arg3, mapname->current.string))
		{
			arg3[0] = '\0';
		}

		if (!arg2[0] && !arg3[0])
		{
			Game::SV_GameSendServerCommand(ent - Game::g_entities, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_TYPEMAP_NOCHANGE\"", 0x65));
			return false;
		}

		if (arg3[0])
		{
			if (arg2[0])
			{
				sprintf_s(Game::level->voteString, "g_gametype %s; map %s", arg2, arg3);
			}
			else
			{
				sprintf_s(Game::level->voteString, "map %s", arg3);
			}

			if (arg2[0])
			{
				sprintf_s(Game::level->voteDisplayString, "GAME_VOTE_GAMETYPE\x14%s\x15 - \x14GAME_VOTE_MAP\x15%s", Game::Scr_GetGameTypeNameForScript(arg2), arg3);
			}
			else
			{
				sprintf_s(Game::level->voteDisplayString, "GAME_VOTE_MAP\x15%s", arg3);
			}
		}
		else
		{
			sprintf_s(Game::level->voteString, "g_gametype %s; map_restart", arg2);
			sprintf_s(Game::level->voteDisplayString, "GAME_VOTE_GAMETYPE\x14%s", Game::Scr_GetGameTypeNameForScript(arg2));
		}

		return true;
	}

	bool Vote::HandleMap([[maybe_unused]] const Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
	{
		// This prevents abuse
		if (!MapRotation::Contains("map", params->get(2)))
		{
			Game::SV_GameSendServerCommand(ent - Game::g_entities, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_NOTONROTATION\"", 0x65));
			return false;
		}

		sprintf_s(Game::level->voteString, "%s %s", params->get(1), params->get(2));
		sprintf_s(Game::level->voteDisplayString, "GAME_VOTE_MAP\x15%s", params->get(2));
		return true;
	}

	bool Vote::HandleGametype([[maybe_unused]] const Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
	{
		if (!Game::Scr_IsValidGameType(params->get(2)))
		{
			Game::SV_GameSendServerCommand(ent - Game::g_entities, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_INVALIDGAMETYPE\"", 0x65));
			return false;
		}

		sprintf_s(Game::level->voteString, "%s %s; map_restart", params->get(2), params->get(3));
		sprintf_s(Game::level->voteDisplayString, "GAME_VOTE_GAMETYPE\x14%s", Game::Scr_GetGameTypeNameForScript(params->get(2)));
		return true;
	}

	bool Vote::HandleKick(const Game::gentity_s* ent, const Command::ServerParams* params)
	{
		char cleanName[0x40]{};

		auto kicknum = Game::level->maxclients;
		for (auto i = 0; i < Game::level->maxclients; ++i)
		{
			if (Game::level->clients[i].sess.connected == Game::CON_CONNECTED)
			{
				strncpy_s(cleanName, Game::level->clients[i].sess.cs.name, _TRUNCATE);
				Game::I_CleanStr(cleanName);
				if (Utils::String::Compare(cleanName, params->get(2)))
				{
					kicknum = i;
				}
			}
		}

		if (kicknum == Game::level->maxclients)
		{
			Game::SV_GameSendServerCommand(ent - Game::g_entities, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_CLIENTNOTONSERVER\"", 0x65));
			return false;
		}

		sprintf_s(Game::level->voteString, "%s \"%d\"", "tempBanClient", kicknum); // kick and tempBanClient do the same thing
		sprintf_s(Game::level->voteDisplayString, "GAME_VOTE_KICK\x15(%i)%s", kicknum, Game::level->clients[kicknum].sess.cs.name);
		return true;
	}

	void Vote::Scr_VoteCalled(Game::gentity_s* self, const char* command, const char* param1, const char* param2)
	{
		Game::Scr_AddString(param2);
		Game::Scr_AddString(param1);
		Game::Scr_AddString(command);
		Game::Scr_Notify(self, Game::scr_const->call_vote, 3);
	}

	void Vote::Scr_PlayerVote(Game::gentity_s* self, const char* option)
	{
		Game::Scr_AddString(option);
		Game::Scr_Notify(self, Game::scr_const->vote, 1);
	}

	void Vote::Cmd_CallVote_f(Game::gentity_s* ent, const Command::ServerParams* params)
	{
		if (!(*Game::g_allowVote)->current.enabled)
		{
			Game::SV_GameSendServerCommand(ent - Game::g_entities, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_VOTINGNOTENABLED\"", 0x65));
			return;
		}

		if (Game::level->numConnectedClients < 2)
		{
			Game::SV_GameSendServerCommand(ent - Game::g_entities, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_VOTINGNOTENOUGHPLAYERS\"", 0x65));
			return;
		}

		if ((*Game::g_oldVoting)->current.enabled)
		{
			if (Game::level->voteTime)
			{
				Game::SV_GameSendServerCommand(ent - Game::g_entities, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_VOTEALREADYINPROGRESS\"", 0x65));
				return;
			}

			if (ent->client->sess.voteCount >= 3)
			{
				Game::SV_GameSendServerCommand(ent - Game::g_entities, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_MAXVOTESCALLED\"", 0x65));
				return;
			}

			if (ent->client->sess.cs.team == Game::TEAM_SPECTATOR)
			{
				Game::SV_GameSendServerCommand(ent - Game::g_entities, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_NOSPECTATORCALLVOTE\"", 0x65));
				return;
			}
		}

		if (IsInvalidVoteString(params->get(1)) ||
			IsInvalidVoteString(params->get(2)) ||
			IsInvalidVoteString(params->get(3)))
		{
			Game::SV_GameSendServerCommand(ent - Game::g_entities, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_INVALIDVOTESTRING\"", 0x65));
			return; 
		}

		if (!(*Game::g_oldVoting)->current.enabled)
		{
			Scr_VoteCalled(ent, params->get(1), params->get(2), params->get(3));
			return;
		}

		const auto itr = VoteCommands.find(params->get(1));
		if (itr == VoteCommands.end())
		{
			Game::SV_GameSendServerCommand(ent - Game::g_entities, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_INVALIDVOTESTRING\"", 0x65));
			Game::SV_GameSendServerCommand(ent - Game::g_entities, Game::SV_CMD_CAN_IGNORE, VA(CallVoteDesc, 0x65));
			return;
		}

		if (Game::level->voteExecuteTime)
		{
			Game::level->voteExecuteTime = 0;
			Game::Cbuf_AddText(0, VA("%s\n", Game::level->voteString));
		}

		const auto shouldDisplay = itr->second(ent, params);
		if (shouldDisplay)
		{
			DisplayVote(ent);
		}
	}

	void Vote::Cmd_Vote_f(Game::gentity_s* ent, const Command::ServerParams* params)
	{
		char arg1[0x100]{};

		if ((*Game::g_oldVoting)->current.enabled)
		{
			if (!Game::level->voteTime)
			{
				Game::SV_GameSendServerCommand(ent - Game::g_entities, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_NOVOTEINPROGRESS\"", 0x65));
				return;
			}

			if ((ent->client->ps.eFlags & Game::EF_VOTED) != 0)
			{
				Game::SV_GameSendServerCommand(ent - Game::g_entities, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_VOTEALREADYCAST\"", 0x65));
				return;
			}

			if (ent->client->sess.cs.team == Game::TEAM_SPECTATOR)
			{
				Game::SV_GameSendServerCommand(ent - Game::g_entities, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_NOSPECTATORVOTE\"", 0x65));
				return;
			}

			Game::SV_GameSendServerCommand(ent - Game::g_entities, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_VOTECAST\"", 0x65));
			ent->client->ps.eFlags |= Game::EF_VOTED;
		}

		strncpy_s(arg1, params->get(1), _TRUNCATE);
		if (arg1[0] == 'y' || arg1[0] == 'Y' || arg1[0] == '1')
		{
			if ((*Game::g_oldVoting)->current.enabled)
			{
				Game::SV_SetConfigstring(Game::CS_VOTE_YES, VA("%i", ++Game::level->voteYes));
			}
			else
			{
				Scr_PlayerVote(ent, "yes");
			}
		}
		else if ((*Game::g_oldVoting)->current.enabled)
		{
			Game::SV_SetConfigstring(Game::CS_VOTE_NO, VA("%i", ++Game::level->voteNo));
		}
		else
		{
			Scr_PlayerVote(ent, "no");
		}
	}

	Vote::Vote()
	{
		// Replicate g_allowVote
		Utils::Hook::Set<std::uint32_t>(0x5E3A4F, Game::DVAR_INTERNAL | Game::DVAR_CODINFO);

		ClientCommand::Add("callvote", Cmd_CallVote_f);
		ClientCommand::Add("vote", Cmd_Vote_f);

		Menus::Add("ui_mp/scriptmenus/callvote.menu");
		Menus::Add("ui_mp/scriptmenus/kickplayer.menu");

		UIScript::Add("voteKick", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
		{
			if (info->playerIndex >= 0 && info->playerIndex < Game::sharedUiInfo->playerCount)
			{
				Game::Cbuf_AddText(0, VA("callvote kick \"%s\"\n", Game::sharedUiInfo->playerNames[info->playerIndex]));
			}
		});

		UIScript::Add("voteTempBan", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
		{
			if (info->playerIndex >= 0 && info->playerIndex < Game::sharedUiInfo->playerCount)
			{
				Game::Cbuf_AddText(0, VA("callvote tempBanUser \"%s\"\n", Game::sharedUiInfo->playerNames[info->playerIndex]));
			}
		});

		UIScript::Add("voteTypeMap", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
		{
			Game::Cbuf_AddText(0, VA("callvote typemap %s %s\n", Game::sharedUiInfo->gameTypes[(*Game::ui_netGameType)->current.integer].gameType,
				Game::sharedUiInfo->mapList[(*Game::ui_currentMap)->current.integer].mapName));
		});

		UIScript::Add("voteMap", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
		{
			if ((*Game::ui_currentMap)->current.integer >= 0 &&
				(*Game::ui_currentMap)->current.integer < Game::sharedUiInfo->mapCount)
			{
				Game::Cbuf_AddText(0, VA("callvote map %s\n", Game::sharedUiInfo->mapList[(*Game::ui_currentMap)->current.integer].mapName));
			}
		});

		UIScript::Add("voteGame", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
		{
			Game::Cbuf_AddText(0, VA("callvote g_gametype %s\n", Game::sharedUiInfo->gameTypes[(*Game::ui_netGameType)->current.integer].gameType));
		});
	}
}
