#include <STDInclude.hpp>

namespace Components
{
	Theatre::DemoInfo Theatre::CurrentInfo;
	unsigned int Theatre::CurrentSelection;
	std::vector<Theatre::DemoInfo> Theatre::Demos;

	char Theatre::BaselineSnapshot[131072] = {0};
	int Theatre::BaselineSnapshotMsgLen;
	int Theatre::BaselineSnapshotMsgOff;

	void Theatre::GamestateWriteStub(Game::msg_t* msg, char byte)
	{
		Game::MSG_WriteLong(msg, 0);
		Game::MSG_WriteByte(msg, byte);
	}

	void Theatre::RecordGamestateStub()
	{
		const auto sequence = (*Game::serverMessageSequence - 1);
		Game::FS_WriteToDemo(&sequence, 4, *Game::demoFile);
	}

	void Theatre::StoreBaseline(PBYTE snapshotMsg)
	{
		// Store offset and length
		BaselineSnapshotMsgLen = *reinterpret_cast<int*>(snapshotMsg + 20);
		BaselineSnapshotMsgOff = *reinterpret_cast<int*>(snapshotMsg + 28) - 7;

		// Copy to our snapshot buffer
		std::memcpy(BaselineSnapshot, *reinterpret_cast<DWORD**>(snapshotMsg + 8), *reinterpret_cast<DWORD*>(snapshotMsg + 20));
	}

	__declspec(naked) void Theatre::BaselineStoreStub()
	{
		_asm
		{
			push edi
			call StoreBaseline
			pop edi

			mov edx, 5ABEF5h
			jmp edx
		}
	}

	void Theatre::WriteBaseline()
	{
		static unsigned char bufData[131072];
		static unsigned char cmpData[131072];

		Game::msg_t buf;

		Game::MSG_Init(&buf, bufData, 131072);
		Game::MSG_WriteData(&buf, &BaselineSnapshot[BaselineSnapshotMsgOff], BaselineSnapshotMsgLen - BaselineSnapshotMsgOff);
		Game::MSG_WriteByte(&buf, 6);

		const auto compressedSize = Game::MSG_WriteBitsCompress(false, buf.data, cmpData, buf.cursize);
		const auto fileCompressedSize = compressedSize + 4;

		int byte8 = 8;
		char byte0 = 0;

		Game::FS_WriteToDemo(&byte0, 1, *Game::demoFile);
		Game::FS_WriteToDemo(Game::serverMessageSequence, 4, *Game::demoFile);
		Game::FS_WriteToDemo(&fileCompressedSize, 4, *Game::demoFile);
		Game::FS_WriteToDemo(&byte8, 4, *Game::demoFile);

		for (auto i = 0; i < compressedSize; i += 1024)
		{
			const auto size = std::min(compressedSize - i, 1024);

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
			call WriteBaseline
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

		CurrentInfo.name = file;
		CurrentInfo.mapname = (*Game::sv_mapname)->current.string;
		CurrentInfo.gametype = (*Game::sv_gametype)->current.string;
		CurrentInfo.author = Steam::SteamFriends()->GetPersonaName();
		CurrentInfo.length = Game::Sys_Milliseconds();
		std::time(&CurrentInfo.timeStamp);
	}

	void Theatre::StopRecordStub(int channel, char* message)
	{
		Game::Com_Printf(channel, message);

		// Store correct length
		CurrentInfo.length = Game::Sys_Milliseconds() - CurrentInfo.length;

		// Write metadata
		FileSystem::FileWriter meta(std::format("{}.json", CurrentInfo.name));
		meta.write(nlohmann::json(CurrentInfo.to_json()).dump());
	}

	void Theatre::LoadDemos([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
	{
		CurrentSelection = 0;
		Demos.clear();

		const auto demos = FileSystem::GetFileList("demos/", "dm_13");

		for (auto demo : demos)
		{
			FileSystem::File meta(Utils::String::VA("demos/%s.json", demo.data()));

			if (meta.exists())
			{
				nlohmann::json metaObject;
				try
				{
					metaObject = nlohmann::json::parse(meta.getBuffer());

					DemoInfo demoInfo;
					demoInfo.name = demo.substr(0, demo.find_last_of("."));
					demoInfo.author = metaObject["author"].get<std::string>();
					demoInfo.gametype = metaObject["gametype"].get<std::string>();
					demoInfo.mapname = metaObject["mapname"].get<std::string>();
					demoInfo.length = metaObject["length"].get<int>();
					auto timestamp = metaObject["timestamp"].get<std::string>();
					demoInfo.timeStamp = _atoi64(timestamp.data());

					Demos.push_back(demoInfo);
				}
				catch (const nlohmann::json::parse_error& ex)
				{
					Logger::PrintError(Game::CON_CHANNEL_ERROR, "Json Parse Error: {}\n", ex.what());
				}
			}
		}

		// Reverse, latest demo first!
		std::ranges::reverse(Demos);
	}

	void Theatre::DeleteDemo([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
	{
		if (CurrentSelection < Demos.size())
		{
			auto demoInfo = Demos.at(CurrentSelection);

			Logger::Print("Deleting demo {}...\n", demoInfo.name);

			FileSystem::_DeleteFile("demos", demoInfo.name + ".dm_13");
			FileSystem::_DeleteFile("demos", demoInfo.name + ".dm_13.json");

			// Reset our ui_demo_* dvars here, because the theater menu needs it.
			Dvar::Var("ui_demo_mapname").set("");
			Dvar::Var("ui_demo_mapname_localized").set("");
			Dvar::Var("ui_demo_gametype").set("");
			Dvar::Var("ui_demo_length").set("");
			Dvar::Var("ui_demo_author").set("");
			Dvar::Var("ui_demo_date").set("");

			// Reload demos
			LoadDemos(UIScript::Token(), info);
		}
	}

	void Theatre::PlayDemo([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
	{
		if (CurrentSelection < Demos.size())
		{
			Command::Execute(std::format("demo {}", Demos[CurrentSelection].name), true);
			Command::Execute("demoback", false);
		}
	}

	unsigned int Theatre::GetDemoCount()
	{
		return Demos.size();
	}

	// Omit column here
	const char* Theatre::GetDemoText(unsigned int item, int /*column*/)
	{
		if (item < Demos.size())
		{
			auto info = Demos.at(item);
			return Utils::String::VA("%s on %s", Game::UI_LocalizeGameType(info.gametype.data()), Game::UI_LocalizeMapName(info.mapname.data()));
		}

		return "";
	}

	void Theatre::SelectDemo(unsigned int index)
	{
		if (index < Demos.size())
		{
			CurrentSelection = index;
			auto info = Demos.at(index);

			tm time;
			char buffer[1000] = {0};
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
			auto demos = FileSystem::GetFileList("demos/", "dm_13");

			for (auto demo : demos)
			{
				if (Utils::String::StartsWith(demo, "auto_"))
				{
					files.push_back(demo);
				}
			}

			auto numDel = static_cast<int>(files.size()) - Dvar::Var("cl_demosKeep").get<int>();

			for (auto i = 0; i < numDel; ++i)
			{
				Logger::Print("Deleting old demo {}\n", files[i]);
				FileSystem::_DeleteFile("demos", files[i].data());
				FileSystem::_DeleteFile("demos", Utils::String::VA("%s.json", files[i].data()));
			}

			Command::Execute(Utils::String::VA("record auto_%lld", time(nullptr)), true);
		}

		return Utils::Hook::Call<DWORD()>(0x42BBB0)();
	}

	void Theatre::MapChangeStub()
	{
		StopRecording();
		Utils::Hook::Call<void()>(0x464A60)();
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

		Utils::Hook(0x5A8370, GamestateWriteStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x5A85D2, RecordGamestateStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x5ABE36, BaselineStoreStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x5A8630, BaselineToFileStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x4CB3EF, UISetActiveMenuStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x50320E, AdjustTimeDeltaStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x5A8E03, ServerTimedOutStub, HOOK_JUMP).install()->quick();

		// Hook commands to enforce metadata generation
		Utils::Hook(0x5A82AE, RecordStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x5A8156, StopRecordStub, HOOK_CALL).install()->quick();

		// Autorecording
		Utils::Hook(0x5A1D6A, InitCGameStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x4A712A, MapChangeStub, HOOK_CALL).install()->quick();

		// UIScripts
		UIScript::Add("loadDemos", LoadDemos);
		UIScript::Add("launchDemo", PlayDemo);
		UIScript::Add("deleteDemo", DeleteDemo);

		// Feeder
		UIFeeder::Add(10.0f, GetDemoCount, GetDemoText, SelectDemo);

		// set the configstrings stuff to load the default (empty) string table; this should allow demo recording on all gametypes/maps
		if (!Dedicated::IsEnabled()) Utils::Hook::Set<const char*>(0x47440B, "mp/defaultStringTable.csv");

		// Change font size
		Utils::Hook::Set<BYTE>(0x5AC854, 2);
		Utils::Hook::Set<BYTE>(0x5AC85A, 2);
	}
}
