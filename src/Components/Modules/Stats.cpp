#include <STDInclude.hpp>
#include "ServerCommands.hpp"
#include "Stats.hpp"

#include "GSC/Script.hpp"

namespace Components
{
	std::int64_t* Stats::GetStatsID()
	{
		static std::int64_t id = 0x110000100001337;
		return &id;
	}

	bool Stats::IsMaxLevel()
	{
		// 2516000 should be the max experience.
		return (Game::Live_GetXp(0) >= Game::CL_GetMaxXP());
	}

	void Stats::SendStats()
	{
		// check if we're connected to a server...
		if (*reinterpret_cast<std::uint32_t*>(0xB2C540) >= 7)
		{
			for (unsigned char i = 0; i < 7; ++i)
			{
				Game::Com_Printf(0, "Sending stat packet %i to server.\n", i);

				// alloc
				Game::msg_t msg{};
				unsigned char buffer[2048]{};

				// init
				Game::MSG_Init(&msg, buffer, sizeof(buffer));
				Game::MSG_WriteString(&msg, "stats");

				// get stat buffer
				char *statbuffer = nullptr;
				if (Utils::Hook::Call<int(int)>(0x444CA0)(0))
				{
					statbuffer = &Utils::Hook::Call<char *(int)>(0x4C49F0)(0)[1240 * i];
				}

				// Client port?
				Game::MSG_WriteShort(&msg, *reinterpret_cast<short*>(0xA1E878));

				// Stat packet index
				Game::MSG_WriteByte(&msg, i);

				// write stat packet data
				if (statbuffer)
				{
					Game::MSG_WriteData(&msg, statbuffer, std::min(8192 - (i * 1240), 1240));
				}

				// send statpacket
				Network::SendRaw(Game::NS_CLIENT1, *reinterpret_cast<Game::netadr_t*>(0xA1E888), std::string(reinterpret_cast<char*>(msg.data), msg.cursize));
			}
		}
	}

	void Stats::UpdateClasses([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
	{
		SendStats();
	}

	int Stats::SaveStats(char* dest, const char* folder, const char* buffer, int size)
	{
		assert(*Game::fs_gameDirVar);

		if (!std::strcmp((*Game::fs_gameDirVar)->current.string, "mods/"))
		{
			folder = (*Game::fs_gameDirVar)->current.string;
		}

		return Utils::Hook::Call<int(char*, const char*, const char*, int)>(0x426450)(dest, folder, buffer, size);
	}

	void Stats::AddScriptFunctions()
	{
		GSC::Script::AddMethod("GetStat", [](const Game::scr_entref_t entref)
		{
			const auto* ent = GSC::Script::Scr_GetPlayerEntity(entref);
			const auto index = Game::Scr_GetInt(0);

			if (index < 0 || index > 3499)
			{
				Game::Scr_ParamError(0, Utils::String::VA("GetStat: invalid index %i", index));
			}

			if (ent->client->sess.connected <= Game::CON_DISCONNECTED)
			{
				Game::Scr_Error("GetStat: called on a disconnected player");
			}

			Game::Scr_AddInt(Game::SV_GetClientStat(ent->s.number, index));
		});

		GSC::Script::AddMethod("SetStat", [](const Game::scr_entref_t entref)
		{
			const auto* ent = GSC::Script::Scr_GetPlayerEntity(entref);

			const auto iNumParms = Game::Scr_GetNumParam();
			if (iNumParms != 2)
			{
				Game::Scr_Error(Utils::String::VA("GetStat: takes 2 arguments, got %u.", iNumParms));
			}

			const auto index = Game::Scr_GetInt(0);
			if (index < 0 || index > 3499)
			{
				Game::Scr_ParamError(0, Utils::String::VA("setstat: invalid index %i", index));
			}

			const auto value = Game::Scr_GetInt(1);
			if (index < 2000 && (value < 0 || value > 255))
			{
				Game::Scr_ParamError(1, Utils::String::VA("setstat: index %i is a byte value, and you're trying to set it to %i", index, value));
			}

			Game::SV_SetClientStat(ent->s.number, index, value);
		});
	}

	Stats::Stats()
	{
		// This UIScript should be added in the onClose code of the cac_popup menu,
		// so everytime the create-a-class window is closed, and a client is connected
		// to a server, the stats data of the client will be reuploaded to the server.
		// allowing the player to change their classes while connected to a server.
		UIScript::Add("UpdateClasses", UpdateClasses);

		// Allow playerdata to be changed while connected to a server
		Utils::Hook::Set<BYTE>(0x4376FD, 0xEB);

		// TODO: Allow playerdata changes in setPlayerData UI script.

		// Rename stat file
		Utils::Hook::SetString(0x71C048, "iw4x.stat");

		// Patch stats steamid
		Utils::Hook::Nop(0x682EBF, 20);
		Utils::Hook::Nop(0x6830B1, 20);

		Utils::Hook(0x682EBF, GetStatsID, HOOK_CALL).install()->quick();
		Utils::Hook(0x6830B1, GetStatsID, HOOK_CALL).install()->quick();
		//Utils::Hook::Set<BYTE>(0x68323A, 0xEB);

		// Never use remote stat saving
		Utils::Hook::Set<BYTE>(0x682F39, 0xEB);

		// Don't create stat backup
		Utils::Hook::Nop(0x402CE6, 2);

		// Write stats to mod folder if a mod is loaded
		Utils::Hook(0x682F7B, SaveStats, HOOK_CALL).install()->quick();

		AddScriptFunctions();

		// Skip silly Com_Error (LiveStorage_SetStat)
		Utils::Hook::Set<BYTE>(0x4CC5F9, 0xEB);

		// 'M' Seems to be used on Xbox only for parsing platform specific ranks
		ServerCommands::OnCommand('M', [](const Command::Params* params)
		{
			const auto* arg1 = params->get(1);
			const auto* arg2 = params->get(2);

			Game::LiveStorage_SetStat(Game::CL_ControllerIndexFromClientNum(0), std::atoi(arg1), std::atoi(arg2));
			return true;
		});

		Command::Add("statGet", [](const Command::Params* params)
		{
			if (params->size() < 2)
			{
				Logger::Print("statget usage: statget <index>\n");
				return;
			}

			const auto index = std::strtol(params->get(1), nullptr, 0);
			const auto stat = Game::LiveStorage_GetStat(0, index);
			Logger::Print(Game::CON_CHANNEL_SYSTEM, "Stat {}: {}\n", index, stat);
		});
	}
}
