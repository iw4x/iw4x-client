#include "Theatre.hpp"
#include "Compatibility.hpp"
#include "UIFeeder.hpp"
#include "Weapon.hpp"

namespace
{
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

  // Note that x64 compiles out CG_AddLagometerSnapshotInfo(), which x86
  // calls in CG_ReadNextSnapshot() right after CL_GetSnapshot(), so we call
  // it here.
  //
  int CL_GetSnapshotStub(int localClientNum, int snapshotNumber, Game::snapshot_s* snapshot)
  {
    const auto result = Game::CL_GetSnapshot(localClientNum, snapshotNumber, snapshot);

    Game::CG_AddLagometerSnapshotInfo(result ? snapshot : nullptr);

    if (Game::clientConnections->demoplaying)
    {
      UpdateWeaponSelection(Game::cgArray[localClientNum], *snapshot);
    }

    return result;
  }

  // Note that on x86 we take over where CL_ReadDemoNetworkPacket() returns
  // once it parsed the message, which on x64 is right after it calls
  // CL_ParseServerMessage().
  //
  void CL_ReadDemoNetworkPacketStub(int localClientNum, Game::msg_t* msg)
  {
    Utils::Hook::Call<void(int, Game::msg_t*)>(0x140100C80)(localClientNum, msg); // CL_ParseServerMessage

    assert(Game::clientConnections->demoplaying);

    auto* clc = Game::clientConnections;
    auto* cgs = Game::cgsArray;

    const auto serverCommandSequence = clc->serverCommandSequence;
    const auto lastExecutedServerCommand = clc->lastExecutedServerCommand;

    // Check if the command string backlog is equal to or greater than half the size of command string buffer (128 / 2 = 64)
    // and execute them now to make room for new command strings without overwriting unprocessed ones
    // likely to happen when fast forwarding, very likely when rewinding
    if (lastExecutedServerCommand + std::ssize(clc->serverCommands) / 2 <= serverCommandSequence)
    {
      if (lastExecutedServerCommand == 0)
      {
        // Update the old server command sequences, otherwise we may be executing old server commands when rewinding
        // this would ignore command strings if they were to be included in the gamestate message
        clc->lastExecutedServerCommand = serverCommandSequence;
        cgs->serverCommandSequence = serverCommandSequence;

        // Reset the viewmodel otherwise it may be hidden for a short while when rewinding
        auto* cg = Game::cgArray;
        cg->landTime = 0;

        Components::Command::Execute("demoAddMissingStrings", false);
      }
      else
      {
        for (auto i = lastExecutedServerCommand + 1; i <= serverCommandSequence; ++i)
        {
          // Note that x64 numbers the server commands as the Steam game
          // does, which makes the fast restart 'x' ('B' on x86).
          //
          static constexpr auto mapRestart = 'x';
          if (clc->serverCommands[i & 127][0] == mapRestart)
          {
            // Ignoring fast restart commands, because they crash in CG_ClearEntityFxHandles,
            // because Game::cgArray->snap is a nullptr when rewinding
            // they also appear to cause crashes when fast forwarding
            clc->serverCommands[i & 127][0] = '\0';
          }
        }

        Game::CG_ExecuteNewServerCommands(0, serverCommandSequence);
      }

      assert(cgs->serverCommandSequence == clc->serverCommandSequence
        && clc->lastExecutedServerCommand == clc->serverCommandSequence);
    }
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

  // CL_Record_f() writes the configstrings op right after the map and the
  // gametype, where the server's gamestate has the configstrings checksum.
  //
  void Theatre::GamestateWriteStub(Game::msg_t* msg, char byte)
  {
    Game::MSG_WriteLong(msg, 0);
    Compatibility::MSG_WriteServerOp(msg, byte);
  }

  // The gamestate takes the message sequence before the baseline's.
  //
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

  void Theatre::WriteBaseline()
  {
    static unsigned char bufData[131072];
    static unsigned char cmpData[131072];

    Game::msg_t buf;

    Game::MSG_Init(&buf, bufData, sizeof(bufData));
    Compatibility::MSG_WriteServerOp(&buf, Game::svc_snapshot);
    Game::MSG_WriteLong(&buf, Game::clients[0].snap.serverTime);
    Game::MSG_WriteData(&buf, &BaselineSnapshot[BaselineSnapshotMsgOff], BaselineSnapshotMsgLen - BaselineSnapshotMsgOff);
    Compatibility::MSG_WriteServerOp(&buf, Game::svc_EOF);

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

      if (static_cast<std::size_t>(i + size) >= sizeof(cmpData))
      {
        Logger::PrintError(Game::CON_CHANNEL_ERROR, "Writing compressed demo baseline exceeded buffer\n");
        break;
      }

      Game::FS_WriteToDemo(&cmpData[i], size, Game::clientConnections->demofile);
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

  // CL_Record_f() ends on clearing the client archive index, which we do
  // once we wrote the baseline after the gamestate.
  //
  void Theatre::BaselineToFileStub()
  {
    WriteBaseline();

    Game::clientConnections->lastClientArchiveIndex = 0;
  }

  // A demo has no server to time out.
  //
  bool Theatre::CL_ServerTimedOut_Stub()
  {
    return Game::clientConnections->demoplaying || Utils::Hook::Call<bool()>(0x1402A14C0)();
  }

  void Theatre::CG_CompassDrawPlayerMapLocationSelector_Stub(const int localClientNum, Game::CompassType compassType, const Game::rectDef_s* parentRect, const Game::rectDef_s* rect, Game::Material* material, float* color)
  {
    if (!Game::clientConnections->demoplaying)
    {
      Utils::Hook::Call<void(int, Game::CompassType, const Game::rectDef_s*, const Game::rectDef_s*, Game::Material*, float*)>(0x1400A4800)(localClientNum, compassType, parentRect, rect, material, color);
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

    // Note that on x86 we write the size of a pointer here, which is that of
    // the command time there.
    //
    (write)(&ps->commandTime, sizeof(int), localClientNum);
    (write)(viewangles, sizeof(float[3]), localClientNum);

    // Disable locationSelectionInfo
    const auto locationSelectionInfo = 0;
    (write)(&locationSelectionInfo, sizeof(int), localClientNum);
  }

  // Note that on x86 we take the demo's name from where CL_Record_f()
  // prints it, which x64 compiles out. So we take it from where the command
  // opens the demo instead.
  //
  Game::fileHandle_t Theatre::RecordStub(const char* file)
  {
    CurrentInfo.name = file;
    CurrentInfo.mapname = (*Game::sv_mapname)->current.string;
    CurrentInfo.gametype = (*Game::sv_gametype)->current.string;
    CurrentInfo.author = Steam::SteamFriends()->GetPersonaName();
    CurrentInfo.length = Game::Sys_Milliseconds();
    std::time(&CurrentInfo.timeStamp);

    return Game::FS_FOpenFileWrite(file);
  }

  // Note that on x86 we write the metadata where CL_StopRecord_f() prints
  // that it stopped, which x64 compiles out. So we run the command ourselves
  // and write it if a demo was recording.
  //
  void Theatre::CL_StopRecord_f_Hk()
  {
    const auto recording = Game::clientConnections->demorecording != 0;

    Utils::Hook::Call<void()>(0x1400FD9E0)(); // CL_StopRecord_f

    if (!recording)
    {
      return;
    }

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

    const auto demos = FileSystem::GetFileList("demos/", DemoExtension);

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

      FileSystem::_DeleteFile("demos", std::format("{}.{}", demoInfo.name, DemoExtension));
      FileSystem::_DeleteFile("demos", std::format("{}.{}.json", demoInfo.name, DemoExtension));

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
    return static_cast<unsigned int>(Demos.size());
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
    if (Game::clientConnections->demoplaying)
    {
      auto* sv_cheats = const_cast<Game::dvar_t*>(Game::dvar_cheats);
      if (!sv_cheats->current.enabled)
      {
        sv_cheats->current.enabled = true;
        sv_cheats->modified = true;
      }
    }
    else if (CLAutoRecord.get<bool>())
    {
      std::vector<std::string> files;
      auto demos = FileSystem::GetFileList("demos/", DemoExtension);

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

      // Set attempt count to 50 to give the game ample opportunity to catch up,
      // though it is expected to succeed on the first attempt
      Scheduler::Schedule([syncAttempts = 50]() mutable
      {
        const auto serverCommandSequence = Game::clientConnections->serverCommandSequence;
        const auto lastExecutedServerCommand = Game::clientConnections->lastExecutedServerCommand;

        // Allow the game to catch up with the execution of the command strings in case they modify the game state strings,
        // otherwise the demo may not contain all necessary game state strings
        if (lastExecutedServerCommand == serverCommandSequence || --syncAttempts < 0)
        {
          const auto timestamp = static_cast<long long>(std::time(nullptr));
          Command::Execute(Utils::String::VA("record auto_%lld", timestamp), true);
          return true;
        }

        return false;
      }, Scheduler::Pipeline::MAIN);
    }

    return Utils::Hook::Call<int()>(0x14012DD00)(); // DB_GetLoadedFlags
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

  // The last configstring the game takes, which CL_ConfigstringModified()
  // checks.
  //
  // Note that x64 has 4339 configstrings where x86 has 4139, so we can't
  // bound them with CS_LAST as on x86.
  //
  constexpr unsigned long MaxConfigstringIndex = Game::MAX_CONFIGSTRINGS - 1;

  auto demoGetGamestateCallback = [](const Components::Command::Params* params)
  {
    if (params->size() != 2)
    {
      Components::Logger::Print("demoGetGamestate: invalid parameter count = {}, expected parameter count = {}\n", params->size(), 2);
      return;
    }

    if (!Game::clientConnections->demoplaying && !Game::dvar_cheats->current.enabled)
    {
      Components::Logger::Print("demoGetGamestate: demo must be playing\n");
      return;
    }

    const auto index = std::strtoul(params->get(1), nullptr, 10);
    if (index > MaxConfigstringIndex)
    {
      Components::Logger::Print("demoGetGamestate: invalid index = {}, max index = {}\n", index, MaxConfigstringIndex);
      return;
    }

    const auto* ptr = Game::CL_GetConfigString(static_cast<int>(index));
    if (!ptr)
    {
      Components::Logger::Print("demoGetGamestate: invalid string address\n");
      return;
    }

    Components::Logger::Print("demoGetGamestate: gamestate index {} = {}\n", index, ptr);
  };

  auto demoSetGamestateCallback = [](const Components::Command::Params* params)
  {
    typedef void (*CL_ConfigstringModified_t)(int localClientNum);
    CL_ConfigstringModified_t CL_ConfigstringModified = CL_ConfigstringModified_t(0x1400F3EC0);

    if (params->size() != 3)
    {
      Components::Logger::Print("demoSetGamestate: invalid parameter count = {}, expected parameter count = {}\n", params->size(), 3);
      return;
    }

    if (!Game::clientConnections->demoplaying && !Game::dvar_cheats->current.enabled)
    {
      Components::Logger::Print("demoSetGamestate: demo must be playing\n");
      return;
    }

    const auto index = std::strtoul(params->get(1), nullptr, 10);
    if (index > MaxConfigstringIndex)
    {
      Components::Logger::Print("demoSetGamestate: invalid index = {}, max index = {}\n", index, MaxConfigstringIndex);
      return;
    }

    CL_ConfigstringModified(0);

    Components::Logger::Print("demoSetGamestate: gamestate index {} modified\n", index);
  };

  auto demoAddMissingStringsCallback = [](const Components::Command::Params*)
  {
    if (!Game::clientConnections->demoplaying && !Game::dvar_cheats->current.enabled)
    {
      Components::Logger::Print("demoAddMissingStrings: demo must be playing\n");
      return;
    }

    static constexpr auto CS_TAGS = 2741;
    static constexpr auto CS_TAGS_FIRST = CS_TAGS + 1;

    auto findTagIndex = [](std::string_view sv) -> size_t
    {
      for (size_t i = CS_TAGS_FIRST, j = 0; i < CS_TAGS_FIRST + 10; ++i, ++j)
      {
        const auto* ptr = Game::CL_GetConfigString(static_cast<int>(i));
        if (ptr && ptr == sv)
        {
          return j;
        }
      }

      return 0;
    };

    constexpr std::array<std::string_view, 4> tagStrings
    {
      "tag_player",
      "tag_stowed_back",
      "tag_stow_back_mid_attach",
      "tag_weapon"
    };

    for (size_t i = CS_TAGS_FIRST; i < CS_TAGS_FIRST + 10; ++i)
    {
      const auto* ptr = Game::CL_GetConfigString(static_cast<int>(i));
      if (ptr && ptr[0] == '\0')
      {
        const auto tagIndex = findTagIndex(tagStrings[0]);
        if (tagIndex < tagStrings.size())
        {
          for (size_t j = 0; j < tagStrings.size(); ++j)
          {
            Command::Execute(std::format("demoSetGamestate {} {}", CS_TAGS_FIRST + tagIndex + j, tagStrings[j]).c_str(), false);
          }

          Components::Logger::Print("demoAddMissingStrings: tag strings added\n");
        }

        break;
      }
    }
  };

  Theatre::Theatre()
  {
    AssertOffset(Game::clientConnection_t, serverMessageSequence, 0x2013C);
    AssertOffset(Game::clientConnection_t, lastClientArchiveIndex, 0x4014C);
    AssertOffset(Game::clientConnection_t, demorecording, 0x40190);
    AssertOffset(Game::clientConnection_t, demoplaying, 0x40194);
    AssertOffset(Game::clientConnection_t, demowaiting, 0x4019C);
    AssertOffset(Game::clientConnection_t, demofile, 0x401A8);

    AssertOffset(Game::clientActive_t, snap.serverTime, 0x312C);
    AssertOffset(Game::clientActive_t, serverTime, 0x3158);
    AssertOffset(Game::clientActive_t, serverTimeDelta, 0x3164);

    AssertOffset(Game::cgs_t, serverCommandSequence, 0x14);

    AssertOffset(Game::snapshot_s, ps.weapCommon.weapon, 0x2B4);
    AssertOffset(Game::snapshot_s, serverTime, 0x3124);

    AssertOffset(Game::playerState_s, bobCycle, 0x18);
    AssertOffset(Game::playerState_s, origin, 0x1C);
    AssertOffset(Game::playerState_s, velocity, 0x28);
    AssertOffset(Game::playerState_s, movementDir, 0xAC);

    CLAutoRecord = Dvar::Register<bool>("cl_autoRecord", true, Game::DVAR_ARCHIVE, "Automatically record games");
    CLDemosKeep = Dvar::Register<int>("cl_demosKeep", 100, 1, 999, Game::DVAR_ARCHIVE, "How many demos to keep with autorecord");

    // Note that on x86 we also play Steam and retail demos (the steamdemo
    // and retaildemo flags). The x64 game is the Steam build without the
    // scrambled messages, and it records with its own protocol, so we play
    // only its own demos.
    //
    // Note also that x64 inlines CG_ReadNextSnapshot() into
    // CG_ProcessSnapshots() twice.
    //
    Utils::Hook(0x1400E95D9, CL_GetSnapshotStub, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400E969B, CL_GetSnapshotStub, HOOK_CALL).install()->quick();

    Utils::Hook(0x1400FCD91, CL_ReadDemoNetworkPacketStub, HOOK_CALL).install()->quick();

    Components::Command::Add("demoGetGamestate", demoGetGamestateCallback);
    Components::Command::Add("demoSetGamestate", demoSetGamestateCallback);
    Components::Command::Add("demoAddMissingStrings", demoAddMissingStringsCallback);

    Utils::Hook(0x1400FCFD4, GamestateWriteStub, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400FD228, RecordGamestateStub, HOOK_CALL).install()->quick();
    Utils::Hook(0x140100F82, Theatre_BaselineStoreStub, HOOK_JUMP).install()->quick();

    Utils::Hook(0x1400FD2AE, BaselineToFileStub, HOOK_CALL).install()->quick();
    Utils::Hook::Nop(0x1400FD2B3, 5);

    Utils::Hook(0x140270E86, Theatre_UISetActiveMenuStub, HOOK_JUMP).install()->quick();

    // Note that x64 inlines CL_AdjustTimeDelta() into CL_SetCGameTime().
    //
    Utils::Hook(0x1400F51A1, Theatre_AdjustTimeDeltaStub, HOOK_JUMP).install()->quick();

    Utils::Hook(0x1400FD509, CL_ServerTimedOut_Stub, HOOK_CALL).install()->quick();

    // Fix issue with locationSelectionInfo by disabling it
    Utils::Hook(0x14010130E, CL_WriteDemoClientArchive_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400DF280, CG_CompassDrawPlayerMapLocationSelector_Stub, HOOK_CALL).install()->quick();

    // Hook commands to enforce metadata generation
    Utils::Hook(0x1400FCEF6, RecordStub, HOOK_CALL).install()->quick();
    Utils::Hook::Displace(0x1400FB8AE, 7, 0x1400FD9E0, reinterpret_cast<uintptr_t>(Utils::Hook::Relay(0x1400FB8AE + 7, reinterpret_cast<void*>(CL_StopRecord_f_Hk))));

    // Autorecording
    Utils::Hook(0x1400F4762, CL_FirstSnapshot_Stub, HOOK_CALL).install()->quick();
    Utils::Hook(0x14023977A, SV_SpawnServer_Stub, HOOK_CALL).install()->quick();

    // UIScripts
    UIScript::Add("loadDemos", LoadDemos);
    UIScript::Add("launchDemo", PlayDemo);
    UIScript::Add("deleteDemo", DeleteDemo);

    // Feeder
    UIFeeder::Add(10.0f, GetDemoCount, GetDemoText, SelectDemo);

    // Set the configstrings stuff to load the default (empty) string table; this should allow demo recording on all gametypes/maps
    if (!Dedicated::IsEnabled()) Utils::Hook::PlaceString(0x14027214A, 7, 0x1403A31A8, "mp/defaultStringTable.csv");

    // Change font size
    //
    // Note that x64 inlines SCR_DrawDemoRecording() into
    // SCR_DrawScreenField().
    //
    Utils::Hook::Set<std::uint8_t>(0x140101A8E, 2);
    Utils::Hook::Set<std::uint8_t>(0x140101A9E, 2);
  }
}

void Theatre_StoreBaseline(Game::msg_t* snapshotMsg)
{
  Components::Theatre::StoreBaseline(snapshotMsg);
}

bool Theatre_AdjustTimeDelta()
{
  return Components::Theatre::AdjustTimeDelta();
}
