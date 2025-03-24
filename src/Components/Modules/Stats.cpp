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
				char* statbuffer = nullptr;
				if (Utils::Hook::Call<int(int)>(0x444CA0)(0))
				{
					statbuffer = &Utils::Hook::Call<char* (int)>(0x4C49F0)(0)[1240 * i];
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

	void Stats::SprintfLiveStorageFilename(char* target, size_t size)
	{
		const auto fsGame = (*Game::fs_gameDirVar)->current.string;

		if (*fsGame)
		{
			SprintfLiveStorageFilenameWithFsGame(target, size, fsGame);
		}
		else
		{
			constexpr auto STATS_FILE = "iw4x.stat";
			const auto length = strnlen(STATS_FILE, size);
			std::strncpy(target, STATS_FILE, length);

			if (length < size)
			{
				target[length] = '\x00';
			}
		}
	}

	
	void Stats::SprintfLiveStorageFilenameWithFsGame(char* target, size_t size, const char* fsGame)
	{
		size_t fsGameLength = strnlen(fsGame, size);

		constexpr auto baseStatName = BASEGAME ".stat";
		const size_t baseStatNameLength = strnlen(baseStatName, 16);

		std::memcpy(target, fsGame, fsGameLength);

		// +1 for / and +1 for null terminator
		if (fsGameLength + baseStatNameLength + 1 + 1 > size)
		{
			std::memcpy(&target[size - baseStatNameLength - 1], baseStatName, baseStatNameLength);
			target[size - baseStatNameLength - 2] = '\\';
			target[size - 1] = '\x00';
		}
		else
		{
			const auto lastPosition = fsGameLength + 1 + baseStatNameLength;
			assert(lastPosition < size);

			target[fsGameLength] = '\\';
			std::memcpy(&target[fsGameLength + 1], baseStatName, baseStatNameLength);
			target[lastPosition] = '\x00';
		}	
	}

	// Old iw4x versions used to write stats in the mods directory. Now we do it like CoD4 does - player directory.
	void Stats::MoveOldStatsToNewFolder()
	{
		const auto basePath = (*Game::fs_basepath)->current.string + "\\mods"s;

		auto modFolders = FileSystem::GetSysFileList(basePath, "", true);
		
		constexpr size_t statsDirLength = 32;
		char statsDirName[statsDirLength]; // 32 is how big the game allocates it normally

		for (auto modName : modFolders)
		{
			const auto fsName = "mods\\"s + modName;

			SprintfLiveStorageFilenameWithFsGame(statsDirName, statsDirLength, fsName.data());

			const auto newFileName = statsDirName;
			const auto newFile = FileSystem::File(newFileName);
			if (!newFile.exists())
			{
				const auto oldFilePath = basePath + "\\" + modName + "\\iw4x.stat";
				if (Utils::IO::FileExists(oldFilePath))
				{
					if (Utils::IO::WriteFile(
						(*Game::fs_basepath)->current.string + "\\players\\"s + newFileName,
						Utils::IO::ReadFile(oldFilePath)
					))
					{
						Utils::IO::RemoveFile(oldFilePath);
					}
				}
			}
		}
	}

	uint32_t Stats::HashFilename()
	{
		return 0xFC30D07E;
	}

	__declspec(naked) void Stats::HashFilenameStub()
	{
		_asm
		{
			mov eax, 0xFC30D07E
			retn
		}
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

		// IW4x stats hashing used to be a constant because fs_game profiles were broken (Steam ID constant, filename constant)
		// Now they're fixed, and so the hash is no longer constant. This is a problem for backward compatibility
		// So we overwrite the function used specifically for Steam storage to return always the same hash
		// The one we used to have our steam ID/filename combination
		Utils::Hook(0x682DB0, HashFilenameStub, HOOK_JUMP).install()->quick();

		// TODO: Allow playerdata changes in setPlayerData UI script.

		// Rename stat file
		Utils::Hook::SetString(0x71C048, "%s");
		Utils::Hook(0x4A5D85, SprintfLiveStorageFilename, HOOK_CALL).install()->quick(); // Non-Modded
		Utils::Hook(0x4A5D9D, SprintfLiveStorageFilename, HOOK_CALL).install()->quick(); // Modded

		// Patch stats steamid
		Utils::Hook::Nop(0x682EBF, 20);
		Utils::Hook::Nop(0x6830B1, 20);

		Utils::Hook(0x682EBF, GetStatsID, HOOK_CALL).install()->quick();
		Utils::Hook(0x6830B1, GetStatsID, HOOK_CALL).install()->quick();

		// Set custom class names even when FS_Game is active
		Utils::Hook::Nop(0x4B091B, 2);
		Utils::Hook::Nop(0x60A48D, 2);
		Utils::Hook::Nop(0x479A20, 2);
		
		// Never use remote stat saving
		Utils::Hook::Set<BYTE>(0x682F39, 0xEB);

		// Don't create stat backup
		Utils::Hook::Nop(0x402CE6, 2);

		AddScriptFunctions();

		// Skip silly Com_Error (LiveStorage_SetStat)
		Utils::Hook::Set<BYTE>(0x4CC5F9, 0xEB);

		Scheduler::Once([](){
			MoveOldStatsToNewFolder();
		}, Components::Scheduler::Pipeline::MAIN);

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
