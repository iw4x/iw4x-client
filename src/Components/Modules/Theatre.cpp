#include "Theatre.hpp"
#include "Game/SteamScrambledData.hpp"
#include "UIFeeder.hpp"
#include "Weapon.hpp"

namespace
{
	enum SnapshotFixType
	{
		SNAPSHOT_FIX_NONE,
		SNAPSHOT_FIX_RETAIL,
		SNAPSHOT_FIX_STEAM
	};

	void UpdateWeaponSelection(Game::cg_s& cg, const Game::snapshot_s& snapshot)
	{
		if (static_cast<unsigned int>(cg.weaponSelect) != snapshot.ps.weapCommon.weapon)
		{
			// change ammo counter and ammo clip counter
			cg.weaponSelect = snapshot.ps.weapCommon.weapon;

			// show weapon name
			cg.weaponSelectTime = snapshot.serverTime;
		}
	}

	void UpdateAmmoIndices(Game::snapshot_s& snapshot)
	{
		for (unsigned int i = 0; i < std::size(snapshot.ps.weaponsEquipped); ++i)
		{
			const auto index = snapshot.ps.weaponsEquipped[i];
			if (index)
			{
				const auto* weaponDef = Game::BG_GetWeaponDef(index);

				// fix ammo indices to prevent the ammo from showing up empty
				snapshot.ps.weapCommon.ammoInClip[i].clipIndex = weaponDef->iClipIndex;
				snapshot.ps.weapCommon.ammoNotInClip[i].ammoType = weaponDef->iAmmoIndex;
			}
		}
	}

	void FixSnapshotDataForSteam(Game::snapshot_s& snapshot)
	{
		for (auto& elem : snapshot.ps.hud.archival)
		{
			if (elem.type == Game::HE_TYPE_FREE)
			{
				break;
			}

			if (elem.type >= Game::HE_TYPE_MAPNAME)
			{
				// Steam version misses HE_TYPE_MAPNAME and HE_TYPE_GAMETYPE
				elem.type = static_cast<Game::he_type_t>(elem.type + 2);
			}
		}

		for (auto& elem : snapshot.ps.hud.current)
		{
			if (elem.type == Game::HE_TYPE_FREE)
			{
				break;
			}

			if (elem.type >= Game::HE_TYPE_MAPNAME)
			{
				// Steam version misses HE_TYPE_MAPNAME and HE_TYPE_GAMETYPE
				elem.type = static_cast<Game::he_type_t>(elem.type + 2);
			}
		}

		static constexpr auto BASEGAME_WEAPON_LIMIT = Components::Weapon::BASEGAME_WEAPON_LIMIT;
		static constexpr auto STEAM_WEAPON_LIMIT = 1400;

		for (auto i = 0; i < std::ssize(snapshot.entities) && i < snapshot.numEntities; ++i)
		{
			auto& ent = snapshot.entities[i];

			if (ent.eType == Game::ET_ITEM || ent.eType == Game::ET_MISSILE || ent.eType == Game::ET_TURRET)
			{
				if (ent.index.item >= STEAM_WEAPON_LIMIT)
				{
					const auto weapIdx = ent.index.item % STEAM_WEAPON_LIMIT;
					const auto weapModel = ent.index.item / STEAM_WEAPON_LIMIT;

					// Steam version uses a limit of 1400 weapons
					// this has to fixed in some way to prevent the game from crashing
					ent.index.item = weapIdx + weapModel * BASEGAME_WEAPON_LIMIT;
				}
			}
			else if (ent.eType == static_cast<int>(Game::ET_EVENTS) + static_cast<int>(Game::EV_OBITUARY))
			{
				const auto val = ((ent.un1.eventParm2 << 8) | ent.eventParm);

				if (val >= STEAM_WEAPON_LIMIT)
				{
					// Steam version uses a limit of 1400 weapons
					// this has to fixed in some way to prevent the game from crashing
					ent.un1.eventParm2 = BASEGAME_WEAPON_LIMIT / 256;
					ent.eventParm = (val - (STEAM_WEAPON_LIMIT - BASEGAME_WEAPON_LIMIT)) - (ent.un1.eventParm2 * 256);
				}
			}
		}
	}

	template <SnapshotFixType type>
	int CL_GetSnapshotStub(int localClientNum, int snapshotNumber, Game::snapshot_s* snapshot)
	{
		const auto result = Game::CL_GetSnapshot(localClientNum, snapshotNumber, snapshot);

		if (Game::clientConnections->demoplaying)
		{
			UpdateWeaponSelection(Game::cgArray[localClientNum], *snapshot);

			if constexpr (type == SNAPSHOT_FIX_RETAIL || type == SNAPSHOT_FIX_STEAM)
			{
				UpdateAmmoIndices(*snapshot);

				if constexpr (type == SNAPSHOT_FIX_STEAM)
				{
					FixSnapshotDataForSteam(*snapshot);
				}
			}
		}

		return result;
	}

	inline auto curScrambledDataPtr = scrambledData;
	void UpdateScrambleBuffer(unsigned int index)
	{
		const auto* ptr = (scrambledData + 16 * (index % 25099));
		curScrambledDataPtr = ptr;
	}

	template <unsigned int count, bool descramble>
		requires (count == 1 || count == 2 || count == 4)
	int MSG_ReadBytes(Game::msg_t* msg)
	{
		if (msg->readcount + count > static_cast<unsigned int>(msg->cursize))
		{
			msg->overflowed = 1;
			return -1;
		}

		int val{};
		std::memcpy(&val, &msg->data[msg->readcount], count);

		if constexpr (descramble)
		{
			if (msg->readcount < 16)
			{
				int scrambledVal{};

				if constexpr (count == 1)
				{
					std::memcpy(&scrambledVal, &curScrambledDataPtr[msg->readcount & 15], count);
					val = (val - scrambledVal) & 0xFF;
				}
				else if constexpr (count == 2)
				{
					std::memcpy(&scrambledVal, &curScrambledDataPtr[msg->readcount & 14], count);
					val = (val - scrambledVal) & 0xFFFF;
				}
				else
				{
					std::memcpy(&scrambledVal, &curScrambledDataPtr[msg->readcount & 12], count);
					val = (val - scrambledVal) & 0xFFFF'FFFF;
				}
			}
		}

		msg->readcount += count;
		return val;
	}

	void FixStringForSteam(char* string)
	{
		if (string[1] != ' ' && string[1] != '\0')
		{
			return;
		}

		if (string[0] < 'A' || string[0] > 'z')
		{
			return;
		}

		static constexpr const char STRING_TYPES[]
		{
			// 'X' is a case that does nothing
			// 'b' is the scoreboard (sb) identifier and is replaced with 'X'
			// as Steam sb strings use a different format and sb is irrelevant for demos anyway

		  // ABCDEFGHIJKLMNOPQRSTUVWXYZ
		    "EFGHIJXLXNOXQRSTUVWXhiXXXX"
		    "XXXXXX"
		  // abcdefghijklmnopqrstuvwxyz
		    "aXcdefgklmnopqrstuvwxyABCD"
		};

		string[0] = STRING_TYPES[string[0] - 'A'];
	}

	const char* MSG_ReadString(Game::msg_t* msg, char* string, unsigned int maxChars)
	{
		for (unsigned int i = 0; ; ++i)
		{
			auto val = MSG_ReadBytes<1, true>(msg);
			if (val == -1)
			{
				val = 0;
			}
			if (i < maxChars)
			{
				string[i] = static_cast<char>((val == 146) ? 39 : val);
			}
			if (val == 0)
			{
				break;
			}
		}

		FixStringForSteam(string);

		string[maxChars - 1] = 0;
		return string;
	}

	int MSG_ReadByteStub(Game::msg_t* msg)
	{
		const auto svcType = MSG_ReadBytes<1, true>(msg);

		switch (svcType)
		{
		case 0:
			return Game::svc_ops_e::svc_snapshot;
		case 1:
			return Game::svc_ops_e::svc_serverCommand;
		case 2:
			return Game::svc_ops_e::svc_gamestate;
		case 4:
			return Game::svc_ops_e::svc_matchdata;
		case 5:
			return Game::svc_ops_e::svc_EOF;
		case 3:
			return Game::svc_ops_e::svc_configstring; // for CL_ParseGamestate
		default:
			// trigger warning for illegible message type in CL_ParseServerMessage
			return -1;
		}
	}

	int MSG_ReadLongStub1(Game::msg_t* msg)
	{
		const auto reliableAcknowledge = MSG_ReadBytes<4, false>(msg);

		UpdateScrambleBuffer(reliableAcknowledge);
		return reliableAcknowledge;
	}

	int MSG_ReadLongStub2(Game::msg_t* msg)
	{
		const auto serverCommandSequence = MSG_ReadBytes<4, true>(msg);

		UpdateScrambleBuffer(Game::clientConnections->reliableAcknowledge + (serverCommandSequence << 8));
		return serverCommandSequence;
	}

	int MSG_ReadLongStub3(Game::msg_t* msg)
	{
		const auto serverTime = MSG_ReadBytes<4, true>(msg);

		UpdateScrambleBuffer(serverTime / 50);
		return serverTime;
	}

	int MSG_ReadLongStub4(Game::msg_t* msg)
	{
		const auto serverCommandSequence = MSG_ReadBytes<4, true>(msg);

		UpdateScrambleBuffer(serverCommandSequence);
		return serverCommandSequence;
	}
}

namespace Components
{
	Theatre::DemoInfo Theatre::CurrentInfo;
	unsigned int Theatre::CurrentSelection;
	std::vector<Theatre::DemoInfo> Theatre::Demos;

	Dvar::Var Theatre::CLAutoRecord;
	Dvar::Var Theatre::CLDemosKeep;

	char Theatre::BaselineSnapshot[131072] = {0};
	int Theatre::BaselineSnapshotMsgLen;
	int Theatre::BaselineSnapshotMsgOff;

	nlohmann::json Theatre::DemoInfo::to_json() const
	{
		return nlohmann::json
		{
			{ "mapname", mapname },
			{ "gametype", gametype },
			{ "author", author },
			{ "length", length },
			{ "timestamp", std::to_string(timeStamp) },
		};
	}

	void Theatre::GamestateWriteStub(Game::msg_t* msg, char byte)
	{
		Game::MSG_WriteLong(msg, 0);
		Game::MSG_WriteByte(msg, byte);
	}

	void Theatre::RecordGamestateStub()
	{
		const auto sequence = (Game::clientConnections->serverMessageSequence - 1);
		Game::FS_WriteToDemo(&sequence, 4, Game::clientConnections->demofile);
	}

	void Theatre::StoreBaseline(Game::msg_t* snapshotMsg)
	{
		if (!Game::clientConnections->demoplaying)
		{
			// Store offset and length
			BaselineSnapshotMsgLen = snapshotMsg->cursize;
			BaselineSnapshotMsgOff = snapshotMsg->readcount - 2; // to account for clSnapshot_t deltaNum and snapFlags

			// Copy to our snapshot buffer
			std::memcpy(BaselineSnapshot, snapshotMsg->data, snapshotMsg->cursize);
		}
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

		Game::MSG_Init(&buf, bufData, sizeof(bufData));
		Game::MSG_WriteByte(&buf, Game::svc_snapshot);
		Game::MSG_WriteLong(&buf, Game::clients[0].snap.serverTime);
		Game::MSG_WriteData(&buf, &BaselineSnapshot[BaselineSnapshotMsgOff], BaselineSnapshotMsgLen - BaselineSnapshotMsgOff);
		Game::MSG_WriteByte(&buf, Game::svc_EOF);

		const auto compressedSize = Utils::Huffman::Compress(buf.data, cmpData, buf.cursize, sizeof(cmpData));
		const auto fileCompressedSize = compressedSize + 4;

		int byte8 = 8;
		unsigned char byte0 = 0;

		Game::FS_WriteToDemo(&byte0, sizeof(unsigned char), Game::clientConnections->demofile);
		Game::FS_WriteToDemo(&Game::clientConnections->serverMessageSequence, sizeof(int), Game::clientConnections->demofile);
		Game::FS_WriteToDemo(&fileCompressedSize, sizeof(int), Game::clientConnections->demofile);
		Game::FS_WriteToDemo(&byte8, sizeof(int), Game::clientConnections->demofile);

		for (auto i = 0; i < compressedSize; i += 1024)
		{
			const auto size = std::min(compressedSize - i, 1024);

			if (i + size >= sizeof(cmpData))
			{
				Logger::PrintError(Game::CON_CHANNEL_ERROR, "Writing compressed demo baseline exceeded buffer\n");
				break;
			}

			Game::FS_WriteToDemo(&cmpData[i], size, Game::clientConnections->demofile);
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

	bool Theatre::AdjustTimeDelta()
	{
		if (Game::clientConnections->demoplaying)
		{
			auto& clientActive = Game::clients[0];
			assert(clientActive.serverTime > 0);
			assert(clientActive.snap.serverTime > 0);

			if (clientActive.serverTime + 1000 < clientActive.snap.serverTime)
			{
				// don't wait for the game to catch up if there's large gap in server times between snapshots
				// this makes it unnecessary to use the 'demoback' command at the start of a demo
				clientActive.serverTimeDelta = clientActive.snap.serverTime - Game::cls->realtime;
			}

			return true;
		}

		return false;
	}

	__declspec(naked) void Theatre::AdjustTimeDeltaStub()
	{
		__asm
		{
			push eax
			pushad
			call AdjustTimeDelta
			mov[esp + 20h], eax
			popad
			pop eax

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
			mov eax, 0xA5EA0C  // clientConnections.demoplaying
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
			mov eax, 0xA5EA0C // clientConnections.demoplaying
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

	void Theatre::CG_CompassDrawPlayerMapLocationSelector_Stub(const int localClientNum, Game::CompassType compassType, const Game::rectDef_s* parentRect, const Game::rectDef_s* rect, Game::Material* material, float* color)
	{
		if (!Game::clientConnections->demoplaying)
		{
			Utils::Hook::Call<void(int, Game::CompassType, const Game::rectDef_s*, const Game::rectDef_s*, Game::Material*, float*)>(0x45BD60)(localClientNum, compassType, parentRect, rect, material, color);
		}
	}

	void Theatre::CL_WriteDemoClientArchive_Hk(void(*write)(const void* buffer, int len, int localClientNum), const Game::playerState_s* ps, const float* viewangles, [[maybe_unused]] const float* selectedLocation, [[maybe_unused]] const float selectedLocationAngle, int localClientNum, int index)
	{
		assert(write);
		assert(ps);

		const unsigned char msgType = 1;
		(write)(&msgType, sizeof(unsigned char), localClientNum);

		(write)(&index, sizeof(int), localClientNum);

		(write)(ps->origin, sizeof(float[3]), localClientNum);
		(write)(ps->velocity, sizeof(float[3]), localClientNum);
		(write)(&ps->movementDir, sizeof(int), localClientNum);
		(write)(&ps->bobCycle, sizeof(int), localClientNum);
		(write)(ps, sizeof(Game::playerState_s*), localClientNum);
		(write)(viewangles, sizeof(float[3]), localClientNum);

		// Disable locationSelectionInfo
		const auto locationSelectionInfo = 0;
		(write)(&locationSelectionInfo, sizeof(int), localClientNum);
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
			if (FileSystem::File meta = std::format("demos/{}.json", demo))
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
					demoInfo.timeStamp = std::strtoll(timestamp.data(), nullptr, 10);

					Demos.push_back(demoInfo);
				}
				catch (const nlohmann::json::parse_error& ex)
				{
					Logger::PrintError(Game::CON_CHANNEL_ERROR, "JSON Parse Error: {}\n", ex.what());
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
			return Utils::String::VA("%s on %s", Game::UI_LocalizeGameType(info.gametype.data()), Localization::LocalizeMapName(info.mapname.data()));
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
			asctime_s(buffer, sizeof(buffer), &time);

			Dvar::Var("ui_demo_mapname").set(info.mapname);
			Dvar::Var("ui_demo_mapname_localized").set(Localization::LocalizeMapName(info.mapname.data()));
			Dvar::Var("ui_demo_gametype").set(Game::UI_LocalizeGameType(info.gametype.data()));
			Dvar::Var("ui_demo_length").set(Utils::String::FormatTimeSpan(info.length));
			Dvar::Var("ui_demo_author").set(info.author);
			Dvar::Var("ui_demo_date").set(buffer);
		}
	}

	int Theatre::CL_FirstSnapshot_Stub()
	{
		if (CLAutoRecord.get<bool>() && !Game::clientConnections->demoplaying)
		{
			std::vector<std::string> files;
			auto demos = FileSystem::GetFileList("demos/", "dm_13");

			for (auto& demo : demos)
			{
				if (Utils::String::StartsWith(demo, "auto_"))
				{
					files.push_back(demo);
				}
			}

			auto numDel = static_cast<int>(files.size()) - CLDemosKeep.get<int>();

			for (auto i = 0; i < numDel; ++i)
			{
				Logger::Print("Deleting old demo {}\n", files[i]);
				FileSystem::_DeleteFile("demos", files[i]);
				FileSystem::_DeleteFile("demos", std::format("{}.json", files[i]));
			}

			Command::Execute(Utils::String::VA("record auto_%lld", std::time(nullptr)), true);
		}

		return Utils::Hook::Call<int()>(0x42BBB0)(); // DB_GetLoadedFlags
	}

	void Theatre::SV_SpawnServer_Stub()
	{
		StopRecording();
		Game::Com_SyncThreads();
	}

	void Theatre::StopRecording()
	{
		if (Game::clientConnections->demorecording)
		{
			Command::Execute("stoprecord", true);
		}
	}

	Theatre::Theatre()
	{
		AssertOffset(Game::clientConnection_t, demorecording, 0x40190);
		AssertOffset(Game::clientConnection_t, demoplaying, 0x40194);
		AssertOffset(Game::clientConnection_t, demofile, 0x401A4);
		AssertOffset(Game::clientConnection_t, serverMessageSequence, 0x2013C);

		CLAutoRecord = Dvar::Register<bool>("cl_autoRecord", true, Game::DVAR_ARCHIVE, "Automatically record games");
		CLDemosKeep = Dvar::Register<int>("cl_demosKeep", 30, 1, 999, Game::DVAR_ARCHIVE, "How many demos to keep with autorecord");

		if (Flags::HasFlag("steamdemo"))
		{
			Utils::Hook(0x4C1C20, MSG_ReadBytes<1, true>, HOOK_JUMP).install()->quick(); // MSG_ReadByte
			Utils::Hook(0x40BDD0, MSG_ReadBytes<2, true>, HOOK_JUMP).install()->quick(); // MSG_ReadShort
			Utils::Hook(0x4C9550, MSG_ReadBytes<4, true>, HOOK_JUMP).install()->quick(); // MSG_ReadLong
			Utils::Hook(0x47A530, MSG_ReadString, HOOK_JUMP).install()->quick();         // MSG_ReadString

			Utils::Hook(0x4A9F76, MSG_ReadByteStub, HOOK_CALL).install()->quick(); // CL_ParseServerMessage
			Utils::Hook(0x5AC347, MSG_ReadByteStub, HOOK_CALL).install()->quick(); // CL_ParseGamestate
			Utils::Hook(0x5AC5FC, MSG_ReadByteStub, HOOK_CALL).install()->quick(); // CL_ParseGamestate

			Utils::Hook(0x5A9C9F, MSG_ReadLongStub1, HOOK_CALL).install()->quick(); // CL_ReadDemoNetworkPacket
			Utils::Hook(0x5AC28D, MSG_ReadLongStub2, HOOK_CALL).install()->quick(); // CL_ParseGamestate
			Utils::Hook(0x5ABD7C, MSG_ReadLongStub3, HOOK_CALL).install()->quick(); // CL_ParseSnapshot
			Utils::Hook(0x5AC778, MSG_ReadLongStub4, HOOK_CALL).install()->quick(); // CL_ParseCommandString

			Utils::Hook(0x5950A8, CL_GetSnapshotStub<SNAPSHOT_FIX_STEAM>, HOOK_CALL).install()->quick();
		}
		else if (Flags::HasFlag("retaildemo"))
		{
			Utils::Hook(0x5950A8, CL_GetSnapshotStub<SNAPSHOT_FIX_RETAIL>, HOOK_CALL).install()->quick();
		}
		else
		{
			Utils::Hook(0x5950A8, CL_GetSnapshotStub<SNAPSHOT_FIX_NONE>, HOOK_CALL).install()->quick();
		}

		Utils::Hook(0x5A8370, GamestateWriteStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x5A85D2, RecordGamestateStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x5ABE36, BaselineStoreStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x5A8630, BaselineToFileStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x4CB3EF, UISetActiveMenuStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x50320E, AdjustTimeDeltaStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x5A8E03, ServerTimedOutStub, HOOK_JUMP).install()->quick();

		// Fix issue with locationSelectionInfo by disabling it
		Utils::Hook(0x5AC20F, CL_WriteDemoClientArchive_Hk, HOOK_CALL).install()->quick();
		Utils::Hook(0x4964A6, CG_CompassDrawPlayerMapLocationSelector_Stub, HOOK_CALL).install()->quick();

		// Hook commands to enforce metadata generation
		Utils::Hook(0x5A82AE, RecordStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x5A8156, StopRecordStub, HOOK_CALL).install()->quick();

		// Autorecording
		Utils::Hook(0x5A1D6A, CL_FirstSnapshot_Stub, HOOK_CALL).install()->quick();
		Utils::Hook(0x4A712A, SV_SpawnServer_Stub, HOOK_CALL).install()->quick();

		// UIScripts
		UIScript::Add("loadDemos", LoadDemos);
		UIScript::Add("launchDemo", PlayDemo);
		UIScript::Add("deleteDemo", DeleteDemo);

		// Feeder
		UIFeeder::Add(10.0f, GetDemoCount, GetDemoText, SelectDemo);

		// Set the configstrings stuff to load the default (empty) string table; this should allow demo recording on all gametypes/maps
		if (!Dedicated::IsEnabled()) Utils::Hook::Set<const char*>(0x47440B, "mp/defaultStringTable.csv");

		// Change font size
		Utils::Hook::Set<std::uint8_t>(0x5AC854, 2);
		Utils::Hook::Set<std::uint8_t>(0x5AC85A, 2);
	}
}
