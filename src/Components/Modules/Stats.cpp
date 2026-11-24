#include "Stats.hpp"
#include "ServerCommands.hpp"

#include "GSC/Script.hpp"

namespace Components
{
  namespace
  {
    // The stats of the first controller. The player data is at 4, whether we
    // have it at 0x2000, and whether it changed since we last wrote it at
    // 0x2001.
    //
    constexpr std::uintptr_t LiveStorageStats = 0x141BD8BC0;
    constexpr std::size_t PlayerDataSize = 0x1FFC;

    struct StructuredDataLookup
    {
      void* def;
      void* type;
      std::uint64_t offset;
    };

    struct StructuredDataBuffer
    {
      std::uint8_t* data;
      std::size_t size;
    };

    const auto StructuredData_GetPathFromArgs =
      reinterpret_cast<bool (*) (Game::StructuredDataDef*, int (*) (), const char* (*) (int), StructuredDataLookup*, int*)> (0x1402801A0);

    const auto StructuredData_SetResultStrings = reinterpret_cast<const char* const*> (0x140425938);

    int CmdArgc ()
    {
      return Command::ClientParams ().size ();
    }

    const char* CmdArgv (int i)
    {
      return Command::ClientParams ().get (i);
    }
  }

  bool Stats::IsMaxLevel()
  {
    // 2516000 should be the max experience.
    return (Game::Live_GetXp(0) >= Game::CL_GetMaxXP());
  }

  void Stats::SendStats()
  {
    // check if we're connected to a server...
    if (*reinterpret_cast<std::uint32_t*>(0x1406CCB78) >= 7)
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
        if (Game::LiveStorage_DoWeHaveStats(0))
        {
          statbuffer = &Utils::Hook::Call<char* (int)>(0x1401F79E0)(0)[1240 * i];
        }

        // Client port?
        Game::MSG_WriteShort(&msg, *reinterpret_cast<short*>(0x140BF9620));

        // Stat packet index
        Game::MSG_WriteByte(&msg, i);

        // write stat packet data
        if (statbuffer)
        {
          Game::MSG_WriteData(&msg, statbuffer, std::min(8192 - (i * 1240), 1240));
        }

        // send statpacket
        Network::SendRaw(Game::NS_CLIENT1, *Game::connectedHost, std::string(reinterpret_cast<char*>(msg.data), msg.cursize));
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

    constexpr auto baseStatName = BASEGAME_LEGACY ".stat";
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
    return STATS_HASH;
  }

  int Stats::LiveStorage_DataSetValue_Stub(void* state, void* ddlContext, const int unused, const char* value)
  {
    Command::ClientParams params;
    if (params.size() == 3 && _stricmp(params.get(1), "prestige") == 0)
    {
      char* end;
      const auto prestige = std::strtol(value, &end, 10);
      if (end != value && *end == '\0')
      {
        value = Utils::String::VA("%d", std::clamp(prestige, 0l, static_cast<long>(MAX_PRESTIGE)));
      }
    }

    return Utils::Hook::Call<int(void*, void*, int, const char*)>(0x1401F8BE0)(state, ddlContext, unused, value);
  }

  // Set a field of our player data, given the path to it and then the
  // value, the way the setPlayerData command does on x86.
  //
  // Note that on x86 we skip the warning the game prints when we set our
  // player data while connected to a server.
  //
  void Stats::LiveStorage_DataSetCmd()
  {
    auto* stats (reinterpret_cast<std::uint8_t*> (LiveStorageStats));

    if (!Game::LiveStorage_DoWeHaveStats (0))
    {
      Game::Com_PrintError (Game::CON_CHANNEL_CLIENT, "Cannot set data: we have not downloaded player stats\n");
      return;
    }

    StructuredDataBuffer buffer {stats + 4, PlayerDataSize};

    StructuredDataLookup lookup;
    int count;
    if (!StructuredData_GetPathFromArgs (Game::StructuredDataDef_GetAsset ("mp/playerdata.def", PlayerDataSize), CmdArgc, CmdArgv, &lookup, &count))
      return;

    const Command::ClientParams params;
    const auto argc (params.size ());

    if (count >= argc)
    {
      Game::Com_PrintError (Game::CON_CHANNEL_SYSTEM, "Not enough arguments to setPlayerData\n");
      return;
    }

    if (count < argc - 1)
    {
      Game::Com_PrintError (Game::CON_CHANNEL_SYSTEM, "Too many arguments to setPlayerData\n");
      return;
    }

    switch (const auto r = LiveStorage_DataSetValue_Stub (&lookup, &buffer, 0, params.get (count)))
    {
      case 0:
      {
        stats[0x2001] = 1;
        break;
      }
      case 1:
      {
        break;
      }
      default:
      {
        Game::Com_PrintError (Game::CON_CHANNEL_SYSTEM, "setPlayerData: %s\n", StructuredData_SetResultStrings[r]);

        std::string command ("Command was: ");
        for (auto i (0); i < argc; ++i)
        {
          command += params.get (i);
          command += ' ';
        }

        Game::Com_Printf (Game::CON_CHANNEL_SYSTEM, "%s\n", command.c_str ());
        break;
      }
    }
  }

  // Set our stats as the server tells us to (the N server command, which
  // is J on x64).
  //
  // The command is a list of offset and data pairs with each data byte
  // encoded as a hex pair or as Z or O for 0 or 1. The game writes the data
  // into our stats buffer without checking that we have the stats (we may
  // not: when fs_game changes, vid_restart re-reads them after reinitializing
  // cgame) nor that the data fits (only the offset is checked). So we make
  // sure of both and ignore the command otherwise.
  //
  // Note that the game still validates the rest (the number of arguments,
  // the encoding, etc).
  //
  void Stats::
  CG_SetStatsFromServer (int localClientNum)
  {
    // The stats buffer is followed by its state.
    //
    constexpr std::size_t size (0x1FFC);

    int c (Game::CL_ControllerIndexFromClientNum (localClientNum));

    if (!Game::LiveStorage_DoWeHaveStats (c))
    {
      Logger::Warning (Game::CON_CHANNEL_CLIENT,
                       "Ignoring stats from server: stats not fetched (cont {})\n",
                       c);
      return;
    }

    Command::ClientParams ps;

    for (int i (1); i < ps.size (); i += 2)
    {
      std::size_t o (std::strtoul (ps.get (i), nullptr, 10));

      // Leave an invalid offset to the game.
      //
      if (o >= size)
        break;

      std::size_t n (0);
      for (const char* p (i + 1 < ps.size () ? ps.get (i + 1) : "");
           *p != '\0';
           p += (*p == 'Z' || *p == 'O' || p[1] == '\0' ? 1 : 2))
        ++n;

      if (n > size - o)
      {
        Logger::Warning (Game::CON_CHANNEL_CLIENT,
                         "Ignoring stats from server: {} bytes at offset {} "
                         "are out of bounds\n",
                         n, o);
        return;
      }
    }

    Utils::Hook::Call<void (int)> (0x1400E8AE0) (localClientNum);
  }

  // Record the items we unlocked since the last map before cgame forgets them.
  //
  // The game remembers which items were unlocked when cgame starts and, on
  // disconnect, marks those unlocked since as new (and the titles and icons
  // among them as unlocked). But cgame also starts again on each map change
  // without us disconnecting, and then simply takes a new snapshot. So staying
  // on a server through a map rotation, anything we unlocked before it never
  // shows as new. We do what disconnecting would before the snapshot.
  //
  void Stats::
  CG_InitUnlockedItems (int localClientNum)
  {
    // Nothing to compare against if the snapshot wasn't taken or if we have
    // disconnected since.
    //
    if (Utils::Hook::Get<bool> (0x1406CCC7D))
    {
      Utils::Hook::Call<void (int)> (0x1401FEA40) (localClientNum); // Titles and icons.
      Utils::Hook::Call<void (int)> (0x1401FED40) (localClientNum); // New items.
    }

    Utils::Hook::Call<void (int)> (0x1401FEBE0) (localClientNum);
  }

  Stats::Stats()
  {
    // This UIScript should be added in the onClose code of the cac_popup menu,
    // so everytime the create-a-class window is closed, and a client is connected
    // to a server, the stats data of the client will be reuploaded to the server.
    // allowing the player to change their classes while connected to a server.
    UIScript::Add("UpdateClasses", UpdateClasses);

    // Allow playerdata to be changed while connected to a server
    Utils::Hook::Set<BYTE>(0x1401F6FCF, 0xEB);

    // The same for the setPlayerData menu script, which the game ignores
    // unless we are disconnected (it only runs if the connection state is
    // below CA_CONNECTING). A script's setPlayerData on the server never
    // reaches the host's own stats on x64, so menus that let players change
    // their player data in game have to set it themselves. The call is
    // replaced with "mov al, 1".
    Utils::Hook::Set<BYTE>(0x140266B54, 0xB0);
    Utils::Hook::Set<BYTE>(0x140266B55, 0x01);
    Utils::Hook::Nop(0x140266B56, 3);

    Utils::Hook(0x1401F7084, LiveStorage_DataSetValue_Stub, HOOK_CALL).install()->quick();

    // IW4x stats hashing used to be a constant because fs_game profiles were broken (Steam ID constant, filename constant)
    // Now they're fixed, and so the hash is no longer constant. This is a problem for backward compatibility
    // So we overwrite the function used specifically for Steam storage to return always the same hash
    // The one we used to have our steam ID/filename combination
    //
    // Note that x64 inlines the hash into the write, where we patch it in
    // along with the Steam ID below.
    //
    Utils::Hook(0x14024BD70, HashFilename, HOOK_JUMP).install()->quick(); // Read

    // TODO: Allow playerdata changes in setPlayerData UI script.

    // x64 compiles out the setPlayerData command (presumably the later
    // revision only changes player data through its backend), which our
    // unlockstats runs, so we register it ourselves.
    //
    Command::Add("setPlayerData", LiveStorage_DataSetCmd);

    // Rename stat file
    //
    // Note that x64 picks the format of the profile name with or without
    // fs_game and makes it with a single call.
    //
    Utils::Hook::PlaceString(0x14024B452, 7, 0x14039F368, "%s"); // Read
    Utils::Hook::PlaceString(0x14024BA98, 7, 0x14039F368, "%s"); // Write
    Utils::Hook(0x1401F76B6, SprintfLiveStorageFilename, HOOK_CALL).install()->quick();

    // Patch stats steamid
    //
    // On x64 the read keeps the ID in r13 and the write keeps it in r14. In
    // the write we also replace the inlined file name hash: the part it
    // takes from the ID (in rdi) becomes zero and the rest our hash. That's a
    // lot of bytes to write by hand, so if the stats ever stop reading after
    // a change here, this is probably the first place to look.
    //
    {
      constexpr std::uintptr_t read (0x14024B3C2);
      Utils::Hook::Nop(read, 24);
      Utils::Hook::Set<std::uint16_t>(read, 0xBD49); // mov r13, imm64
      Utils::Hook::Set<std::uint64_t>(read + 2, STATS_ID);

      constexpr std::uintptr_t write (0x14024B837);
      constexpr std::uintptr_t writeHash (0x14024B885);
      Utils::Hook::Nop(write, writeHash - write);
      Utils::Hook::Set<std::uint16_t>(write, 0xBE49); // mov r14, imm64
      Utils::Hook::Set<std::uint64_t>(write + 2, STATS_ID);
      Utils::Hook::Set<std::uint16_t>(write + 10, 0xFF31); // xor edi, edi
      Utils::Hook::Set<std::uint8_t>(write + 12, 0xB8); // mov eax, imm32
      Utils::Hook::Set<std::uint32_t>(write + 13, STATS_HASH);
      Utils::Hook::Set<std::uint8_t>(write + 17, 0xEB); // jmp short
      Utils::Hook::Set<std::int8_t>(write + 18, static_cast<std::int8_t>(writeHash - (write + 19)));
    }

    // Note that on x86 we also set the custom class names when fs_game is
    // set, which x64 always does anyway.
    //

    // Never use remote stat saving
    Utils::Hook::Set<BYTE>(0x14024BA6C, 0xEB);

    // Don't create stat backup
    Utils::Hook::Set<BYTE>(0x14024B769, 0xEB);

    AddScriptFunctions();

    // Skip silly Com_Error (LiveStorage_SetStat)
    Utils::Hook::Set<BYTE>(0x1401F8DD3, 0xEB);

    Scheduler::Once([](){
      MoveOldStatsToNewFolder();
    }, Components::Scheduler::Pipeline::MAIN);

    // Note that x64 sends the stats as the J server command.
    //
    Utils::Hook (0x1400E7090, CG_SetStatsFromServer, HOOK_CALL).install ()->quick (); // CG_DeployServerCommand: J.
    Utils::Hook (0x1400D6B70, CG_InitUnlockedItems, HOOK_CALL).install ()->quick (); // CG_Init: snapshot unlocked items.

    // 'M' Seems to be used on Xbox only for parsing platform specific ranks
    //
    // On x86 we take this as the M server command, but x64 uses M for one of
    // its own, so here we take it as G (see Game::SV_SetClientStat()).
    //
    ServerCommands::OnCommand('G', [](const Command::Params* params)
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
