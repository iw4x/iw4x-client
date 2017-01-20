#include "STDInclude.hpp"

namespace Components
{
	Utils::Signal<QuickPatch::Callback> QuickPatch::ShutdownSignal;

	int64_t* QuickPatch::GetStatsID()
	{
		static int64_t id = 0x110000100001337;
		return &id;
	}

	void QuickPatch::OnShutdown(Utils::Slot<QuickPatch::Callback> callback)
	{
		QuickPatch::ShutdownSignal.connect(callback);
	}

	void QuickPatch::ShutdownStub(int num)
	{
		QuickPatch::ShutdownSignal();
		Utils::Hook::Call<void(int)>(0x46B370)(num);
	}

	void QuickPatch::OnFrame(Utils::Slot<QuickPatch::Callback> callback)
	{
		if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled())
		{
			Dedicated::OnFrame(callback);
		}
		else
		{
			Renderer::OnFrame(callback);
		}
	}

	void QuickPatch::Once(Utils::Slot<QuickPatch::Callback> callback)
	{
		if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled())
		{
			Dedicated::Once(callback);
		}
		else
		{
			Renderer::Once(callback);
		}
	}

	void QuickPatch::UnlockStats()
	{
		if (Game::CL_IsCgameInitialized())
		{
			Toast::Show("cardicon_locked", "^1Error", "Not allowed while ingame.", 3000);
			return;
		}

		Command::Execute("setPlayerData prestige 10");
		Command::Execute("setPlayerData experience 2516000");
		Command::Execute("setPlayerData iconUnlocked cardicon_prestige10_02 1");

		// Unlock challenges
		Game::StringTable* challengeTable = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_STRINGTABLE, "mp/allchallengestable.csv").stringTable;

		if (challengeTable)
		{
			for (int i = 0; i < challengeTable->rowCount; ++i)
			{
				// Find challenge
				const char* challenge = Game::TableLookup(challengeTable, i, 0);

				int maxState = 0;
				int maxProgress = 0;

				// Find correct tier and progress
				for (int j = 0; j < 10; ++j)
				{
					int progress = atoi(Game::TableLookup(challengeTable, i, 6 + j * 2));
					if (!progress) break;

					maxState = j + 2;
					maxProgress = progress;
				}

				Command::Execute(Utils::String::VA("setPlayerData challengeState %s %d", challenge, maxState));
				Command::Execute(Utils::String::VA("setPlayerData challengeProgress %s %d", challenge, maxProgress));
			}
		}
	}

	int QuickPatch::MsgReadBitsCompressCheckSV(const char *from, char *to, int size)
	{
		static char buffer[0x8000];

		if (size > 0x800) return 0;
		size = Game::MSG_ReadBitsCompress(from, buffer, size);

		if (size > 0x800) return 0;
		std::memcpy(to, buffer, size);

		return size;
	}

	int QuickPatch::MsgReadBitsCompressCheckCL(const char *from, char *to, int size)
	{
		static char buffer[0x100000];

		if (size > 0x20000) return 0;
		size = Game::MSG_ReadBitsCompress(from, buffer, size);

		if (size > 0x20000) return 0;
		std::memcpy(to, buffer, size);

		return size;
	}

	void QuickPatch::SelectStringTableEntryInDvarStub()
	{
		Command::ClientParams args;

		if (args.length() >= 4)
		{
			std::string cmd = args[0];
			std::string table = args[1];
			std::string col = args[2];
			std::string dvarName = args[3];
			Game::dvar_t* dvar = Game::Dvar_FindVar(dvarName.data());

			if (Command::Find(dvarName) || (dvar && (dvar->flags & (Game::DVAR_FLAG_WRITEPROTECTED | Game::DVAR_FLAG_CHEAT | Game::DVAR_FLAG_READONLY))))
			{
				return;
			}
		}

		Game::CL_SelectStringTableEntryInDvar_f();
	}

	void QuickPatch::CompareMaterialStateBits()
	{
		Game::DB_EnumXAssets(Game::XAssetType::ASSET_TYPE_MATERIAL, [] (Game::XAssetHeader header, void* /*unused*/)
		{
			bool first = true;
			Game::Material* material = header.material;
			Logger::Print(6, "Checking material %s...", material->name);
			#define COMPARE_TECHNIQUES(x) if(!(material->stateBitsEntry[(Game::MaterialTechniqueType:: ## x)] == material->stateBitsEntry[(Game::MaterialTechniqueType:: ## x ## _DFOG)])) \
											 { \
											 	 	if(first) { Logger::Print("\nMismatch in material %s:\n", material->name); first = false; } \
													Logger::Print(6, #x " != " #x "_DFOG\n"); \
													Logger::Print("0x02%hhx %hhb\n", material->stateBitsEntry[(Game::MaterialTechniqueType:: ## x)], material->stateBitsEntry[(Game::MaterialTechniqueType:: ## x)]); \
													Logger::Print("0x02%hhx %hhb\n", material->stateBitsEntry[(Game::MaterialTechniqueType:: ## x ## _DFOG)], material->stateBitsEntry[(Game::MaterialTechniqueType:: ## x ## _DFOG)]); \
											 }
			COMPARE_TECHNIQUES(TECHNIQUE_EMISSIVE)
			COMPARE_TECHNIQUES(TECHNIQUE_EMISSIVE_SHADOW)
			COMPARE_TECHNIQUES(TECHNIQUE_LIT)
			COMPARE_TECHNIQUES(TECHNIQUE_LIT_SUN)
			COMPARE_TECHNIQUES(TECHNIQUE_LIT_SUN_SHADOW)
			COMPARE_TECHNIQUES(TECHNIQUE_LIT_SPOT)
			COMPARE_TECHNIQUES(TECHNIQUE_LIT_SPOT_SHADOW)
			COMPARE_TECHNIQUES(TECHNIQUE_LIT_OMNI)
			COMPARE_TECHNIQUES(TECHNIQUE_LIT_OMNI_SHADOW)
			COMPARE_TECHNIQUES(TECHNIQUE_LIT_INSTANCED)
			COMPARE_TECHNIQUES(TECHNIQUE_LIT_INSTANCED_SUN)
			COMPARE_TECHNIQUES(TECHNIQUE_LIT_INSTANCED_SUN_SHADOW)
			COMPARE_TECHNIQUES(TECHNIQUE_LIT_INSTANCED_SPOT)
			COMPARE_TECHNIQUES(TECHNIQUE_LIT_INSTANCED_SPOT_SHADOW)
			COMPARE_TECHNIQUES(TECHNIQUE_LIT_INSTANCED_OMNI)
			COMPARE_TECHNIQUES(TECHNIQUE_LIT_INSTANCED_OMNI_SHADOW)
			#undef COMPARE_TECHNIQUES
			if(first)
			{
				Logger::Print(6, "no mismatches found\n");
			}

		}, nullptr, false);
	}

	void QuickPatch::testFunc()
	{

	}

	QuickPatch::QuickPatch()
	{
		// protocol version (workaround for hacks)
		Utils::Hook::Set<int>(0x4FB501, PROTOCOL);

		// protocol command
		Utils::Hook::Set<int>(0x4D36A9, PROTOCOL);
		Utils::Hook::Set<int>(0x4D36AE, PROTOCOL);
		Utils::Hook::Set<int>(0x4D36B3, PROTOCOL);

		// internal version is 99, most servers should accept it
		Utils::Hook::Set<int>(0x463C61, 208);

		// remove system pre-init stuff (improper quit, disk full)
		Utils::Hook::Set<BYTE>(0x411350, 0xC3);

		// remove STEAMSTART checking for DRM IPC
		Utils::Hook::Nop(0x451145, 5);
		Utils::Hook::Set<BYTE>(0x45114C, 0xEB);

		// LSP disabled
		Utils::Hook::Set<BYTE>(0x435950, 0xC3); // LSP HELLO
		Utils::Hook::Set<BYTE>(0x49C220, 0xC3); // We wanted to send a logging packet, but we haven't connected to LSP!
		Utils::Hook::Set<BYTE>(0x4BD900, 0xC3); // main LSP response func
		Utils::Hook::Set<BYTE>(0x682170, 0xC3); // Telling LSP that we're playing a private match
		Utils::Hook::Nop(0x4FD448, 5);          // Don't create lsp_socket

		// Don't delete config files if corrupted
		Utils::Hook::Set<BYTE>(0x47DCB3, 0xEB);
		Utils::Hook::Set<BYTE>(0x4402B6, 0);

		// hopefully allow alt-tab during game, used at least in alt-enter handling
		Utils::Hook::Set<DWORD>(0x45ACE0, 0xC301B0);

		// fs_basegame
		Utils::Hook::Set<char*>(0x6431D1, BASEGAME);

		// UI version string
		Utils::Hook::Set<char*>(0x43F73B, "IW4x: " VERSION);

		// console version string
		Utils::Hook::Set<char*>(0x4B12BB, "IW4x " VERSION " (built " __DATE__ " " __TIME__ ")");

		// version string
		Utils::Hook::Set<char*>(0x60BD56, "IW4x (" VERSION ")");

		// version string color
		static float buildLocColor[] = { 1.0f, 1.0f, 1.0f, 0.8f };
		Utils::Hook::Set(0x43F710, buildLocColor);

		// Shift ui version string to the left (ui_buildlocation)
		Utils::Hook::Nop(0x6310A0, 5); // Don't register the initial dvar
		Utils::Hook::Nop(0x6310B8, 5); // Don't write the result
		Dvar::OnInit([] ()
		{
			*reinterpret_cast<Game::dvar_t**>(0x62E4B64) = Game::Dvar_RegisterVec2("ui_buildLocation", -80.0f, 15.0f, -10000.0, 10000.0, Game::DVAR_FLAG_READONLY, "Where to draw the build number");
		});

		// console title
		if (ZoneBuilder::IsEnabled())
		{
			Utils::Hook::Set<char*>(0x4289E8, "IW4x (" VERSION "): ZoneBuilder");
		}
		else if (Dedicated::IsEnabled())
		{
			Utils::Hook::Set<char*>(0x4289E8, "IW4x (r" VERSION "): Dedicated");
		}
		else
		{
			Utils::Hook::Set<char*>(0x4289E8, "IW4x (r" VERSION "): Console");
		}

		// window title
		Utils::Hook::Set<char*>(0x5076A0, "IW4x: Multiplayer");

		// sv_hostname
		Utils::Hook::Set<char*>(0x4D378B, "IW4Host");

		// shortversion
		Utils::Hook::Set<char*>(0x60BD91, SHORTVERSION);

		// console logo
		Utils::Hook::Set<char*>(0x428A66, BASEGAME "/images/logo.bmp");

		// splash logo
		Utils::Hook::Set<char*>(0x475F9E, BASEGAME "/images/splash.bmp");

		Utils::Hook::Set<char*>(0x4876C6, "Successfully read stats data\n");

		// Numerical ping (cg_scoreboardPingText 1)
		Utils::Hook::Set<BYTE>(0x45888E, 1);
		Utils::Hook::Set<BYTE>(0x45888C, Game::dvar_flag::DVAR_FLAG_CHEAT);

		// increase font sizes for chat on higher resolutions
		static float float13 = 13.0f;
		static float float10 = 10.0f;
		Utils::Hook::Set<float*>(0x5814AE, &float13);
		Utils::Hook::Set<float*>(0x5814C8, &float10);

		// Enable commandline arguments
		Utils::Hook::Set<BYTE>(0x464AE4, 0xEB);

		// remove limit on IWD file loading
		Utils::Hook::Set<BYTE>(0x642BF3, 0xEB);

		// Disable UPNP
		Utils::Hook::Nop(0x60BE24, 5);

		// disable the IWNet IP detection (default 'got ipdetect' flag to 1)
		Utils::Hook::Set<BYTE>(0x649D6F0, 1);

		// Fix stats sleeping
		Utils::Hook::Set<BYTE>(0x6832BA, 0xEB);
		Utils::Hook::Set<BYTE>(0x4BD190, 0xC3);

		// remove 'impure stats' checking
		Utils::Hook::Set<BYTE>(0x4BB250, 0x33);
		Utils::Hook::Set<BYTE>(0x4BB251, 0xC0);
		Utils::Hook::Set<DWORD>(0x4BB252, 0xC3909090);

		// default sv_pure to 0
		Utils::Hook::Set<BYTE>(0x4D3A74, 0);

		// Force debug logging
		Utils::Hook::Nop(0x4AA89F, 8);
		//Utils::Hook::Set<BYTE>(0x6FF53C, 0);

		// remove activeAction execution (exploit in mods)
		Utils::Hook::Set<BYTE>(0x5A1D43, 0xEB);

		// disable bind protection
		Utils::Hook::Set<BYTE>(0x4DACA2, 0xEB);

		// require Windows 5
		Utils::Hook::Set<BYTE>(0x467ADF, 5);
		Utils::Hook::Set<char>(0x6DF5D6, '5');

		// disable 'ignoring asset' notices
		Utils::Hook::Nop(0x5BB902, 5);

		// disable migration_dvarErrors
		Utils::Hook::Set<BYTE>(0x60BDA7, 0);

		// allow joining 'developer 1' servers
		Utils::Hook::Set<BYTE>(0x478BA2, 0xEB);

		// fs_game fixes
		Utils::Hook::Nop(0x4A5D74, 2); // remove fs_game profiles
		Utils::Hook::Set<BYTE>(0x4081FD, 0xEB); // defaultweapon
		Utils::Hook::Set<BYTE>(0x452C1D, 0xEB); // LoadObj weaponDefs

		// filesystem init default_mp.cfg check
		Utils::Hook::Nop(0x461A9E, 5);
		Utils::Hook::Nop(0x461AAA, 5);
		Utils::Hook::Set<BYTE>(0x461AB4, 0xEB);

		// vid_restart when ingame
		Utils::Hook::Nop(0x4CA1FA, 6);

		// Filter log (initially com_logFilter, but I don't see why that dvar is needed)
		// Seems like it's needed for B3, so there is a separate handling for dedicated servers in Dedicated.cpp
		if (!Dedicated::IsEnabled())
		{
			Utils::Hook::Nop(0x647466, 5); // 'dvar set' lines
			Utils::Hook::Nop(0x5DF4F2, 5); // 'sending splash open' lines
		}

		// intro stuff
		Utils::Hook::Nop(0x60BEE9, 5); // Don't show legals
		Utils::Hook::Nop(0x60BEF6, 5); // Don't reset the intro dvar
		Utils::Hook::Set<char*>(0x60BED2, "unskippablecinematic IW_logo\n");
		Utils::Hook::Set<char*>(0x51C2A4, "%s\\" BASEGAME "\\video\\%s.bik");
		Utils::Hook::Set<DWORD>(0x51C2C2, 0x78A0AC);

		// Redirect logs
		Utils::Hook::Set<char*>(0x5E44D8, "logs/games_mp.log");
		Utils::Hook::Set<char*>(0x60A90C, "logs/console_mp.log");
		Utils::Hook::Set<char*>(0x60A918, "logs/console_mp.log");

		// Rename config
		Utils::Hook::Set<char*>(0x461B4B, CLIENT_CONFIG);
		Utils::Hook::Set<char*>(0x47DCBB, CLIENT_CONFIG);
		Utils::Hook::Set<char*>(0x6098F8, CLIENT_CONFIG);
		Utils::Hook::Set<char*>(0x60B279, CLIENT_CONFIG);
		Utils::Hook::Set<char*>(0x60BBD4, CLIENT_CONFIG);

		Utils::Hook(0x4D697A, QuickPatch::ShutdownStub, HOOK_CALL).install()->quick();

		// Disable profile system
//		Utils::Hook::Nop(0x60BEB1, 5);          // GamerProfile_InitAllProfiles - Causes an error, when calling a harrier killstreak.
//		Utils::Hook::Nop(0x60BEB8, 5);          // GamerProfile_LogInProfile
//		Utils::Hook::Nop(0x4059EA, 5);          // GamerProfile_RegisterCommands
		Utils::Hook::Nop(0x4059EF, 5);          // GamerProfile_RegisterDvars
		Utils::Hook::Nop(0x47DF9A, 5);          // GamerProfile_UpdateSystemDvars
		Utils::Hook::Set<BYTE>(0x5AF0D0, 0xC3); // GamerProfile_SaveProfile
		Utils::Hook::Set<BYTE>(0x4E6870, 0xC3); // GamerProfile_UpdateSystemVarsFromProfile
		Utils::Hook::Set<BYTE>(0x4C37F0, 0xC3); // GamerProfile_UpdateProfileAndSaveIfNeeded
		Utils::Hook::Set<BYTE>(0x633CA0, 0xC3); // GamerProfile_SetPercentCompleteMP

		Utils::Hook::Nop(0x5AF1EC, 5); // Profile loading error
		Utils::Hook::Set<BYTE>(0x5AE212, 0xC3); // Profile reading

		// GamerProfile_RegisterCommands
		// Some random function used as nullsub :P
		Utils::Hook::Set<DWORD>(0x45B868, 0x5188FB); // profile_menuDvarsSetup
		Utils::Hook::Set<DWORD>(0x45B87E, 0x5188FB); // profile_menuDvarsFinish
		Utils::Hook::Set<DWORD>(0x45B894, 0x5188FB); // profile_toggleInvertedPitch
		Utils::Hook::Set<DWORD>(0x45B8AA, 0x5188FB); // profile_setViewSensitivity
		Utils::Hook::Set<DWORD>(0x45B8C3, 0x5188FB); // profile_setButtonsConfig
		Utils::Hook::Set<DWORD>(0x45B8D9, 0x5188FB); // profile_setSticksConfig
		Utils::Hook::Set<DWORD>(0x45B8EF, 0x5188FB); // profile_toggleAutoAim
		Utils::Hook::Set<DWORD>(0x45B905, 0x5188FB); // profile_SetHasEverPlayed_MainMenu
		Utils::Hook::Set<DWORD>(0x45B91E, 0x5188FB); // profile_SetHasEverPlayed_SP
		Utils::Hook::Set<DWORD>(0x45B934, 0x5188FB); // profile_SetHasEverPlayed_SO
		Utils::Hook::Set<DWORD>(0x45B94A, 0x5188FB); // profile_SetHasEverPlayed_MP
		Utils::Hook::Set<DWORD>(0x45B960, 0x5188FB); // profile_setVolume
		Utils::Hook::Set<DWORD>(0x45B979, 0x5188FB); // profile_setGamma
		Utils::Hook::Set<DWORD>(0x45B98F, 0x5188FB); // profile_setBlacklevel
		Utils::Hook::Set<DWORD>(0x45B9A5, 0x5188FB); // profile_toggleCanSkipOffensiveMissions

		// Patch SV_IsClientUsingOnlineStatsOffline
		Utils::Hook::Set<DWORD>(0x46B710, 0x90C3C033);

		// Fix mouse pitch adjustments
		UIScript::Add("updateui_mousePitch", [] (UIScript::Token)
		{
			if (Dvar::Var("ui_mousePitch").get<bool>())
			{
				Dvar::Var("m_pitch").set(-0.022f);
			}
			else
			{
				Dvar::Var("m_pitch").set(0.022f);
			}
		});

		// Rename stat file
		Utils::Hook::SetString(0x71C048, "iw4x.stat");

		// Patch stats steamid
		Utils::Hook::Nop(0x682EBF, 20);
		Utils::Hook::Nop(0x6830B1, 20);
		Utils::Hook(0x682EBF, QuickPatch::GetStatsID, HOOK_CALL).install()->quick();
		Utils::Hook(0x6830B1, QuickPatch::GetStatsID, HOOK_CALL).install()->quick();
		//Utils::Hook::Set<BYTE>(0x68323A, 0xEB);

		// Exploit fixes
		Utils::Hook::Set<BYTE>(0x412370, 0xC3);                                                      // SV_SteamAuthClient
		Utils::Hook::Set<BYTE>(0x5A8C70, 0xC3);                                                      // CL_HandleRelayPacket
		Utils::Hook(0x414D92, QuickPatch::MsgReadBitsCompressCheckSV, HOOK_CALL).install()->quick(); // SV_ExecuteClientCommands
		Utils::Hook(0x4A9F56, QuickPatch::MsgReadBitsCompressCheckCL, HOOK_CALL).install()->quick(); // CL_ParseServerMessage

		// Patch selectStringTableEntryInDvar
		Utils::Hook::Set(0x405959, QuickPatch::SelectStringTableEntryInDvarStub);

		Command::Add("unlockstats", [] (Command::Params*)
		{
			QuickPatch::UnlockStats();
		});

		Command::Add("crash", [] (Command::Params*)
		{
			throw new std::exception();
		});

		Command::Add("checkmaterials", [] (Command::Params*)
		{
			QuickPatch::CompareMaterialStateBits();
		});

		Command::Add("test", [](Command::Params*)
		{
			QuickPatch::testFunc();
		});

		// Dvars
		Dvar::Register<bool>("ui_streamFriendly", false, Game::DVAR_FLAG_SAVED, "Stream friendly UI");

		// Debug patches
#ifdef DEBUG
		// ui_debugMode 1
		//Utils::Hook::Set<bool>(0x6312E0, true);

		// fs_debug 1
		Utils::Hook::Set<bool>(0x643172, true);

		// developer 2
		Utils::Hook::Set<BYTE>(0x4FA425, 2);
		Utils::Hook::Set<BYTE>(0x51B087, 2);
		Utils::Hook::Set<BYTE>(0x60AE13, 2);

		// developer_Script 1
		Utils::Hook::Set<bool>(0x60AE2B, true);

		// Disable cheat protection for dvars
		Utils::Hook::Set<BYTE>(0x647682, 0xEB);

		// Constantly draw the mini console
		Utils::Hook::Set<BYTE>(0x412A45, 0xEB);
		Renderer::OnFrame([] ()
		{
			if (*reinterpret_cast<Game::Font**>(0x62E4BAC))
			{
				Game::Con_DrawMiniConsole(0, 2, 4, (Game::CL_IsCgameInitialized() ? 1.0f : 0.4f));
			}
		});
#else
		// Remove missing tag message
		Utils::Hook::Nop(0x4EBF1A, 5);
#endif
	}

	QuickPatch::~QuickPatch()
	{
		QuickPatch::ShutdownSignal.clear();
	}

	bool QuickPatch::unitTest()
	{
		uint32_t randIntCount = 4'000'000;
		printf("Generating %d random integers...", randIntCount);

		auto startTime = std::chrono::high_resolution_clock::now();

		for (uint32_t i = 0; i < randIntCount; ++i)
		{
			Utils::Cryptography::Rand::GenerateInt();
		}

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
		Logger::Print("took %llims\n", duration);

		printf("Testing ZLib compression...");

		std::string test = Utils::String::VA("%c", Utils::Cryptography::Rand::GenerateInt());

		for (int i = 0; i < 21; ++i)
		{
			std::string compressed = Utils::Compression::ZLib::Compress(test);
			std::string decompressed = Utils::Compression::ZLib::Decompress(compressed);

			if (test != decompressed)
			{
				printf("Error\n");
				printf("Compressing %d bytes and decompressing failed!\n", test.size());
				return false;
			}

			auto size = test.size();
			for (unsigned int j = 0; j < size; ++j)
			{
				test.append(Utils::String::VA("%c", Utils::Cryptography::Rand::GenerateInt()));
			}
		}

		printf("Success\n");
		return true;
	}
}
