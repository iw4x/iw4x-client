#include <STDInclude.hpp>

namespace Components
{
	Theatre::DemoInfo Theatre::CurrentInfo;
	unsigned int Theatre::CurrentSelection;
	std::vector<Theatre::DemoInfo> Theatre::Demos;

	char Theatre::BaselineSnapshot[131072] = { 0 };
	int Theatre::BaselineSnapshotMsgLen;
	int Theatre::BaselineSnapshotMsgOff;

	void Theatre::GamestateWriteStub(Game::msg_t* msg, char byte)
	{
		Game::MSG_WriteLong(msg, 0);
		Game::MSG_WriteByte(msg, byte);
	}

	void Theatre::RecordGamestateStub()
	{
		int sequence = (*Game::serverMessageSequence - 1);
		Game::FS_WriteToDemo(&sequence, 4, *Game::demoFile);
	}

	void Theatre::StoreBaseline(PBYTE snapshotMsg)
	{
		// Store offset and length
		Theatre::BaselineSnapshotMsgLen = *reinterpret_cast<int*>(snapshotMsg + 20);
		Theatre::BaselineSnapshotMsgOff = *reinterpret_cast<int*>(snapshotMsg + 28) - 7;

		// Copy to our snapshot buffer
		std::memcpy(Theatre::BaselineSnapshot, *reinterpret_cast<DWORD**>(snapshotMsg + 8), *reinterpret_cast<DWORD*>(snapshotMsg + 20));
	}

	__declspec(naked) void Theatre::BaselineStoreStub()
	{
		_asm
		{
			push edi
			call Theatre::StoreBaseline
			pop edi

			mov edx, 5ABEF5h
			jmp edx
		}
	}

	void Theatre::WriteBaseline()
	{
		static char bufData[131072];
		static char cmpData[131072];

		Game::msg_t buf;

		Game::MSG_Init(&buf, bufData, 131072);
		Game::MSG_WriteData(&buf, &Theatre::BaselineSnapshot[Theatre::BaselineSnapshotMsgOff], Theatre::BaselineSnapshotMsgLen - Theatre::BaselineSnapshotMsgOff);
		Game::MSG_WriteByte(&buf, 6);

		int compressedSize = Game::MSG_WriteBitsCompress(false, buf.data, cmpData, buf.cursize);
		int fileCompressedSize = compressedSize + 4;

		int byte8 = 8;
		char byte0 = 0;

		Game::FS_WriteToDemo(&byte0, 1, *Game::demoFile);
		Game::FS_WriteToDemo(Game::serverMessageSequence, 4, *Game::demoFile);
		Game::FS_WriteToDemo(&fileCompressedSize, 4, *Game::demoFile);
		Game::FS_WriteToDemo(&byte8, 4, *Game::demoFile);

		for (int i = 0; i < compressedSize; i += 1024)
		{
			int size = std::min(compressedSize - i, 1024);

			if (i + size >= sizeof(cmpData))
			{
				Logger::PrintError(Game::CON_CHANNEL_ERROR, "Writing compressed demo baseline exceeded buffer\n");
				break;
			}

			Game::FS_WriteToDemo(&cmpData[i], size, *Game::demoFile);
		}
	}

	__declspec(naked) void Theatre::BaselineToFileStub()
	{
		__asm
		{
			pushad
			call Theatre::WriteBaseline
			popad

			// Restore overwritten operation
			mov ecx, 0A5E9C4h
			mov [ecx], 0

			// Return to original code
			push 5A863Ah
			retn
		}
	}

	__declspec(naked) void Theatre::AdjustTimeDeltaStub()
	{
		__asm
		{
			mov eax, Game::demoPlaying
			mov eax, [eax]
			test al, al
			jz continue

			// delta doesn't drift for demos
			retn

		continue:
			push 5A1AD0h
			retn
		}
	}

	__declspec(naked) void Theatre::ServerTimedOutStub()
	{
		__asm
		{
			mov eax, Game::demoPlaying
			mov eax, [eax]
			test al, al
			jz continue

			mov eax, 5A8E70h
			jmp eax

		continue:
			mov eax, 0B2BB90h
			push 5A8E08h
			retn
		}
	}

	__declspec(naked) void Theatre::UISetActiveMenuStub()
	{
		__asm
		{
			mov eax, Game::demoPlaying
			mov eax, [eax]
			test al, al
			jz continue

			mov eax, 4CB49Ch
			jmp eax

		continue:
			mov ecx, [esp + 10h]
			push 10h
			push ecx
			push 4CB3F6h
			retn
		}
	}

	void Theatre::RecordStub(int channel, char* message, char* file)
	{
		Game::Com_Printf(channel, message, file);

		Theatre::CurrentInfo.name = file;
		Theatre::CurrentInfo.mapname = Dvar::Var("mapname").get<const char*>();
		Theatre::CurrentInfo.gametype = Dvar::Var("g_gametype").get<const char*>();
		Theatre::CurrentInfo.author = Steam::SteamFriends()->GetPersonaName();
		Theatre::CurrentInfo.length = Game::Sys_Milliseconds();
		std::time(&Theatre::CurrentInfo.timeStamp);
	}

	void Theatre::StopRecordStub(int channel, char* message)
	{
		Game::Com_Printf(channel, message);

		// Store correct length
		Theatre::CurrentInfo.length = Game::Sys_Milliseconds() - Theatre::CurrentInfo.length;

		// Write metadata
		FileSystem::FileWriter meta(Utils::String::VA("%s.json", Theatre::CurrentInfo.name.data()));
		meta.write(nlohmann::json(Theatre::CurrentInfo.to_json()).dump());
	}

	void Theatre::LoadDemos(UIScript::Token)
	{
		Theatre::CurrentSelection = 0;
		Theatre::Demos.clear();

		auto demos = FileSystem::GetFileList("demos/", "dm_13");

		for (auto demo : demos)
		{
			FileSystem::File meta(Utils::String::VA("demos/%s.json", demo.data()));

			if (meta.exists())
			{
				std::string error;
				nlohmann::json metaObject = nlohmann::json::parse(meta.getBuffer());

				if (metaObject.is_object())
				{
					Theatre::DemoInfo info;

					info.name      = demo.substr(0, demo.find_last_of("."));
					info.author    = metaObject["author"].get<std::string>();
					info.gametype  = metaObject["gametype"].get<std::string>();
					info.mapname   = metaObject["mapname"].get<std::string>();
					info.length    = metaObject["length"].get<int>();
					info.timeStamp = _atoi64(metaObject["timestamp"].get<std::string>().data());

					Theatre::Demos.push_back(info);
				}
			}
		}

		// Reverse, latest demo first!
		std::reverse(Theatre::Demos.begin(), Theatre::Demos.end());
	}

	void Theatre::DeleteDemo(UIScript::Token)
	{
		if (Theatre::CurrentSelection < Theatre::Demos.size())
		{
			Theatre::DemoInfo info = Theatre::Demos[Theatre::CurrentSelection];

			Logger::Print("Deleting demo {}...\n", info.name);

			FileSystem::DeleteFile("demos", info.name + ".dm_13");
			FileSystem::DeleteFile("demos", info.name + ".dm_13.json");

			// Reset our ui_demo_* dvars here, because the theater menu needs it.
			Dvar::Var("ui_demo_mapname").set("");
			Dvar::Var("ui_demo_mapname_localized").set("");
			Dvar::Var("ui_demo_gametype").set("");
			Dvar::Var("ui_demo_length").set("");
			Dvar::Var("ui_demo_author").set("");
			Dvar::Var("ui_demo_date").set("");

			// Reload demos
			Theatre::LoadDemos(UIScript::Token());
		}
	}

	void Theatre::PlayDemo(UIScript::Token)
	{
		if (Theatre::CurrentSelection < Theatre::Demos.size())
		{
			Command::Execute(Utils::String::VA("demo %s", Theatre::Demos[Theatre::CurrentSelection].name.data()), true);
			Command::Execute("demoback", false);
		}
	}

	unsigned int Theatre::GetDemoCount()
	{
		return Theatre::Demos.size();
	}

	// Omit column here
	const char* Theatre::GetDemoText(unsigned int item, int /*column*/)
	{
		if (item < Theatre::Demos.size())
		{
			Theatre::DemoInfo info = Theatre::Demos[item];

			return Utils::String::VA("%s on %s", Game::UI_LocalizeGameType(info.gametype.data()), Game::UI_LocalizeMapName(info.mapname.data()));
		}

		return "";
	}

	void Theatre::SelectDemo(unsigned int index)
	{
		if (index < Theatre::Demos.size())
		{
			Theatre::CurrentSelection = index;
			Theatre::DemoInfo info = Theatre::Demos[index];

			tm time;
			char buffer[1000] = { 0 };
			localtime_s(&time, &info.timeStamp);
			asctime_s(buffer, sizeof buffer, &time);

			Dvar::Var("ui_demo_mapname").set(info.mapname);
			Dvar::Var("ui_demo_mapname_localized").set(Game::UI_LocalizeMapName(info.mapname.data()));
			Dvar::Var("ui_demo_gametype").set(Game::UI_LocalizeGameType(info.gametype.data()));
			Dvar::Var("ui_demo_length").set(Utils::String::FormatTimeSpan(info.length));
			Dvar::Var("ui_demo_author").set(info.author);
			Dvar::Var("ui_demo_date").set(buffer);
		}
	}

	uint32_t Theatre::InitCGameStub()
	{
		if (Dvar::Var("cl_autoRecord").get<bool>() && !*Game::demoPlaying)
		{
			std::vector<std::string> files;
			std::vector<std::string> demos = FileSystem::GetFileList("demos/", "dm_13");

			for (auto demo : demos)
			{
				if (Utils::String::StartsWith(demo, "auto_"))
				{
					files.push_back(demo);
				}
			}

			int numDel = files.size() - Dvar::Var("cl_demosKeep").get<int>();

			for (int i = 0; i < numDel; ++i)
			{
				Logger::Print("Deleting old demo {}\n", files[i]);
				FileSystem::DeleteFile("demos", files[i].data());
				FileSystem::DeleteFile("demos", Utils::String::VA("%s.json", files[i].data()));
			}

			Command::Execute(Utils::String::VA("record auto_%lld", time(nullptr)), true);
		}

		return Utils::Hook::Call<DWORD()>(0x42BBB0)();
	}

	void Theatre::MapChangeStub()
	{
		Theatre::StopRecording();
		Utils::Hook::Call<void()>(0x464A60)();
	}

	// DANGEROUS, DON'T USE THIS ONE!
	void Theatre::MapChangeSVStub(char* a1, char* a2)
	{
		Theatre::StopRecording();
		Utils::Hook::Call<void(char*, char*)>(0x487C50)(a1, a2);
	}

	void Theatre::StopRecording()
	{
		if (*Game::demoRecording)
		{
			Command::Execute("stoprecord", true);
		}
	}

	Theatre::Theatre()
	{
		Dvar::Register<bool>("cl_autoRecord", true, Game::DVAR_ARCHIVE, "Automatically record games.");
		Dvar::Register<int>("cl_demosKeep", 30, 1, 999, Game::DVAR_ARCHIVE, "How many demos to keep with autorecord.");

		Utils::Hook(0x5A8370, Theatre::GamestateWriteStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x5A85D2, Theatre::RecordGamestateStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x5ABE36, Theatre::BaselineStoreStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x5A8630, Theatre::BaselineToFileStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x4CB3EF, Theatre::UISetActiveMenuStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x50320E, Theatre::AdjustTimeDeltaStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x5A8E03, Theatre::ServerTimedOutStub, HOOK_JUMP).install()->quick();

		// Hook commands to enforce metadata generation
		Utils::Hook(0x5A82AE, Theatre::RecordStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x5A8156, Theatre::StopRecordStub, HOOK_CALL).install()->quick();

		// Autorecording
		Utils::Hook(0x5A1D6A, Theatre::InitCGameStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x4A712A, Theatre::MapChangeStub, HOOK_CALL).install()->quick();
		//Utils::Hook(0x5AA91C, Theatre::MapChangeSVStub, HOOK_CALL).install()->quick();

		// UIScripts
		UIScript::Add("loadDemos", Theatre::LoadDemos);
		UIScript::Add("launchDemo", Theatre::PlayDemo);
		UIScript::Add("deleteDemo", Theatre::DeleteDemo);

		// Feeder
		UIFeeder::Add(10.0f, Theatre::GetDemoCount, Theatre::GetDemoText, Theatre::SelectDemo);

		// set the configstrings stuff to load the default (empty) string table; this should allow demo recording on all gametypes/maps
		if (!Dedicated::IsEnabled()) Utils::Hook::Set<const char*>(0x47440B, "mp/defaultStringTable.csv");

		// Change font size
		Utils::Hook::Set<BYTE>(0x5AC854, 2);
		Utils::Hook::Set<BYTE>(0x5AC85A, 2);

// 		Command::Add("democycle", [] (Command::Params params)
// 		{
// 			// Cmd_FollowCycle_f
// 			Utils::Hook::Call<void(Game::gentity_t*, int)>(0x458ED0)(Game::g_entities, -1);
// 		});
	}
}
