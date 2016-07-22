#include "STDInclude.hpp"

namespace Components
{
	Theatre::Container Theatre::DemoContainer;

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
		Game::FS_Write(&sequence, 4, *Game::demoFile);
	}

	void Theatre::StoreBaseline(PBYTE snapshotMsg)
	{
		// Store offset and length
		Theatre::BaselineSnapshotMsgLen = *reinterpret_cast<int*>(snapshotMsg + 20);
		Theatre::BaselineSnapshotMsgOff = *reinterpret_cast<int*>(snapshotMsg + 28) - 7;

		// Copy to our snapshot buffer
		std::memcpy(Theatre::BaselineSnapshot, *reinterpret_cast<DWORD**>(snapshotMsg + 8), *reinterpret_cast<DWORD*>(snapshotMsg + 20));
	}

	void __declspec(naked) Theatre::BaselineStoreStub()
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

		Game::FS_Write(&byte0, 1, *Game::demoFile);
		Game::FS_Write(Game::serverMessageSequence, 4, *Game::demoFile);
		Game::FS_Write(&fileCompressedSize, 4, *Game::demoFile);
		Game::FS_Write(&byte8, 4, *Game::demoFile);

		for (int i = 0; i < compressedSize; i += 1024)
		{
			int size = std::min(compressedSize - i, 1024);

			if (i + size >= sizeof(cmpData))
			{
				Logger::Print("Error: Writing compressed demo baseline exceeded buffer\n");
				break;
			}
			
			Game::FS_Write(&cmpData[i], size, *Game::demoFile);
		}
	}

	void __declspec(naked) Theatre::BaselineToFileStub()
	{
		__asm
		{
			call Theatre::WriteBaseline

			// Restore overwritten operation
			mov ecx, 0A5E9C4h
			mov [ecx], 0

			// Return to original code
			mov ecx, 5A863Ah
			jmp ecx
		}
	}

	void __declspec(naked) Theatre::AdjustTimeDeltaStub()
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
			mov eax, 5A1AD0h
			jmp eax
		}
	}

	void __declspec(naked) Theatre::ServerTimedOutStub()
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
			mov esi, 5A8E08h
			jmp esi
		}
	}

	void __declspec(naked) Theatre::UISetActiveMenuStub()
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
			mov eax, 4CB3F6h
			jmp eax
		}
	}

	void Theatre::RecordStub(int channel, char* message, char* file)
	{
		Game::Com_Printf(channel, message, file);

		Theatre::DemoContainer.CurrentInfo.Name = file;
		Theatre::DemoContainer.CurrentInfo.Mapname = Dvar::Var("mapname").Get<const char*>();
		Theatre::DemoContainer.CurrentInfo.Gametype = Dvar::Var("g_gametype").Get<const char*>();
		Theatre::DemoContainer.CurrentInfo.Author = Steam::SteamFriends()->GetPersonaName();
		Theatre::DemoContainer.CurrentInfo.Length = Game::Sys_Milliseconds();
		std::time(&Theatre::DemoContainer.CurrentInfo.TimeStamp);
	}

	void Theatre::StopRecordStub(int channel, char* message)
	{
		Game::Com_Printf(channel, message);

		// Store correct length
		Theatre::DemoContainer.CurrentInfo.Length = Game::Sys_Milliseconds() - Theatre::DemoContainer.CurrentInfo.Length;

		// Write metadata
		FileSystem::FileWriter meta(fmt::sprintf("%s.json", Theatre::DemoContainer.CurrentInfo.Name.data()));
		meta.Write(json11::Json(Theatre::DemoContainer.CurrentInfo).dump());
	}

	void Theatre::LoadDemos()
	{
		Theatre::DemoContainer.CurrentSelection = 0;
		Theatre::DemoContainer.Demos.clear();

		auto demos = FileSystem::GetFileList("demos/", "dm_13");

		for (auto demo : demos)
		{
			FileSystem::File meta(fmt::sprintf("demos/%s.json", demo.data()));

			if (meta.Exists())
			{
				std::string error;
				json11::Json metaObject = json11::Json::parse(meta.GetBuffer(), error);

				if (metaObject.is_object())
				{
					Theatre::Container::DemoInfo info;

					info.Name      = demo.substr(0, demo.find_last_of("."));
					info.Author    = metaObject["author"].string_value();
					info.Gametype  = metaObject["gametype"].string_value();
					info.Mapname   = metaObject["mapname"].string_value();
					info.Length    = (int)metaObject["length"].number_value();
					info.TimeStamp = _atoi64(metaObject["timestamp"].string_value().data());

					Theatre::DemoContainer.Demos.push_back(info);
				}
			}
		}

		// Reverse, latest demo first!
		std::reverse(Theatre::DemoContainer.Demos.begin(), Theatre::DemoContainer.Demos.end());
	}

	void Theatre::DeleteDemo()
	{
		if (Theatre::DemoContainer.CurrentSelection < Theatre::DemoContainer.Demos.size())
		{
			Theatre::Container::DemoInfo info = Theatre::DemoContainer.Demos[Theatre::DemoContainer.CurrentSelection];
	
			Logger::Print("Deleting demo %s...\n", info.Name.data());

			FileSystem::DeleteFile("demos", info.Name + ".dm_13");
			FileSystem::DeleteFile("demos", info.Name + ".dm_13.json");

			// Reset our ui_demo_* dvars here, because the theater menu needs it.
			Dvar::Var("ui_demo_mapname").Set("");
			Dvar::Var("ui_demo_mapname_localized").Set("");
			Dvar::Var("ui_demo_gametype").Set("");
			Dvar::Var("ui_demo_length").Set("");
			Dvar::Var("ui_demo_author").Set("");
			Dvar::Var("ui_demo_date").Set("");

			// Reload demos
			Theatre::LoadDemos();
		}
	}

	void Theatre::PlayDemo()
	{
		if (Theatre::DemoContainer.CurrentSelection < Theatre::DemoContainer.Demos.size())
		{
			Command::Execute(fmt::sprintf("demo %s", Theatre::DemoContainer.Demos[Theatre::DemoContainer.CurrentSelection].Name.data()), true);
			Command::Execute("demoback", false);
		}
	}

	unsigned int Theatre::GetDemoCount()
	{
		return Theatre::DemoContainer.Demos.size();
	}

	// Omit column here
	const char* Theatre::GetDemoText(unsigned int item, int column)
	{
		if (item < Theatre::DemoContainer.Demos.size())
		{
			Theatre::Container::DemoInfo info = Theatre::DemoContainer.Demos[item];

			return Utils::String::VA("%s on %s", Game::UI_LocalizeGameType(info.Gametype.data()), Game::UI_LocalizeMapName(info.Mapname.data()));
		}

		return "";
	}

	void Theatre::SelectDemo(unsigned int index)
	{
		if (index < Theatre::DemoContainer.Demos.size())
		{
			Theatre::DemoContainer.CurrentSelection = index;
			Theatre::Container::DemoInfo info = Theatre::DemoContainer.Demos[index];

			tm time;
			char buffer[1000] = { 0 };
			localtime_s(&time, &info.TimeStamp);
			asctime_s(buffer, sizeof buffer, &time);

			Dvar::Var("ui_demo_mapname").Set(info.Mapname);
			Dvar::Var("ui_demo_mapname_localized").Set(Game::UI_LocalizeMapName(info.Mapname.data()));
			Dvar::Var("ui_demo_gametype").Set(Game::UI_LocalizeGameType(info.Gametype.data()));
			Dvar::Var("ui_demo_length").Set(Utils::String::FormatTimeSpan(info.Length));
			Dvar::Var("ui_demo_author").Set(info.Author);
			Dvar::Var("ui_demo_date").Set(buffer);
		}
	}

	uint32_t Theatre::InitCGameStub()
	{
		if (Dvar::Var("cl_autoRecord").Get<bool>() && !*Game::demoPlaying)
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

			int numDel = files.size() - Dvar::Var("cl_demosKeep").Get<int>();

			for (int i = 0; i < numDel; ++i)
			{
				Logger::Print("Deleting old demo %s\n", files[i].data());
				FileSystem::DeleteFile("demos", files[i].data());
				FileSystem::DeleteFile("demos", fmt::sprintf("%s.json", files[i].data()));
			}

			Command::Execute(fmt::format("record auto_{}", time(0)), true);
		}

		return Utils::Hook::Call<DWORD()>(0x42BBB0)();
	}

	void Theatre::MapChangeStub()
	{
		if (*Game::demoRecording)
		{
			Command::Execute("stoprecord", true);
		}

		Utils::Hook::Call<void()>(0x464A60)();
	}

	void Theatre::MapChangeSVStub(char* a1, char* a2)
	{
		if (*Game::demoRecording)
		{
			Command::Execute("stoprecord", true);
		}

		Utils::Hook::Call<void(char*, char*)>(0x487C50)(a1, a2);
	}

	Theatre::Theatre()
	{
		Dvar::Register<bool>("cl_autoRecord", true, Game::dvar_flag::DVAR_FLAG_SAVED, "Automatically record games.");
		Dvar::Register<int>("cl_demosKeep", 30, 1, 999, Game::dvar_flag::DVAR_FLAG_SAVED, "How many demos to keep with autorecord.");

		Utils::Hook(0x5A8370, Theatre::GamestateWriteStub, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x5A85D2, Theatre::RecordGamestateStub, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x5ABE36, Theatre::BaselineStoreStub, HOOK_JUMP).Install()->Quick();
		Utils::Hook(0x5A8630, Theatre::BaselineToFileStub, HOOK_JUMP).Install()->Quick();
		Utils::Hook(0x4CB3EF, Theatre::UISetActiveMenuStub, HOOK_JUMP).Install()->Quick();
		Utils::Hook(0x50320E, Theatre::AdjustTimeDeltaStub, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x5A8E03, Theatre::ServerTimedOutStub, HOOK_JUMP).Install()->Quick();

		// Hook commands to enforce metadata generation
		Utils::Hook(0x5A82AE, Theatre::RecordStub, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x5A8156, Theatre::StopRecordStub, HOOK_CALL).Install()->Quick();

		// Autorecording
		Utils::Hook(0x5A1D6A, Theatre::InitCGameStub, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x4A712A, Theatre::MapChangeStub, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x5AA91C, Theatre::MapChangeSVStub, HOOK_CALL).Install()->Quick();

		// UIScripts
		UIScript::Add("loadDemos", Theatre::LoadDemos);
		UIScript::Add("launchDemo", Theatre::PlayDemo);
		UIScript::Add("deleteDemo", Theatre::DeleteDemo);

		// Feeder
		UIFeeder::Add(10.0f, Theatre::GetDemoCount, Theatre::GetDemoText, Theatre::SelectDemo);

		// set the configstrings stuff to load the default (empty) string table; this should allow demo recording on all gametypes/maps
		if(!Dedicated::IsEnabled()) Utils::Hook::Set<char*>(0x47440B, "mp/defaultStringTable.csv");
	
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
