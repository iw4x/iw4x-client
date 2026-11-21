#include "Server.hpp"

namespace Game
{
  SV_AddTestClient_t SV_AddTestClient = SV_AddTestClient_t(0x140235230);
  SV_IsTestClient_t SV_IsTestClient = SV_IsTestClient_t(0x140236F20);
  SV_GameClientNum_Score_t SV_GameClientNum_Score = SV_GameClientNum_Score_t(0x14019CEE0);
  SV_SetBrushModel_t SV_SetBrushModel = SV_SetBrushModel_t(0x140231C70);
  SV_UnlinkEntity_t SV_UnlinkEntity = SV_UnlinkEntity_t(0x140233B30);
  SV_LinkEntity_t SV_LinkEntity = SV_LinkEntity_t(0x1402324C0);
  SV_GameSendServerCommand_t SV_GameSendServerCommand = SV_GameSendServerCommand_t(0x140231910);
  SV_SendServerCommand_t SV_SendServerCommand = SV_SendServerCommand_t(0x14023B6D0);
  SV_Cmd_TokenizeString_t SV_Cmd_TokenizeString = SV_Cmd_TokenizeString_t(0x1401E7FC0);
  SV_Cmd_EndTokenizedString_t SV_Cmd_EndTokenizedString = SV_Cmd_EndTokenizedString_t(0x1401E7F80);
  SV_Cmd_ArgvBuffer_t SV_Cmd_ArgvBuffer = SV_Cmd_ArgvBuffer_t(0x1401E7F40);
  SV_DirectConnect_t SV_DirectConnect = SV_DirectConnect_t(0x140235A10);
  SV_SetConfigstring_t SV_SetConfigstring = SV_SetConfigstring_t(0x1402391E0);
  SV_GetConfigstringConst_t SV_GetConfigstringConst = SV_GetConfigstringConst_t(0x140238720);
  SV_Loaded_t SV_Loaded = SV_Loaded_t(0x140238E60);
  SV_ClientThink_t SV_ClientThink = SV_ClientThink_t(0x140235790);
  SV_DropClient_t SV_DropClient = SV_DropClient_t(0x1402363F0);
  SV_FindClientByAddress_t SV_FindClientByAddress = SV_FindClientByAddress_t(0x14023A980);
  SV_WaitServer_t SV_WaitServer = SV_WaitServer_t(0x14023BF10);
  SV_SpawnServer_t SV_SpawnServer = SV_SpawnServer_t(0x140239750);
  SV_GetClientPersistentDataBuffer_t SV_GetClientPersistentDataBuffer = SV_GetClientPersistentDataBuffer_t(0x140236C90);
  SV_GetClientPersistentDataModifiedFlags_t SV_GetClientPersistentDataModifiedFlags = SV_GetClientPersistentDataModifiedFlags_t(0x140236CB0);

  int* svs_time = reinterpret_cast<int*>(0x14340CB00);
  int* sv_timeResidual = reinterpret_cast<int*>(0x1464FBD84);
  int* sv_serverId_value = reinterpret_cast<int*>(0x1422CC508);
  int* svs_clientCount = reinterpret_cast<int*>(0x14340CB08);
  int* sv_migrate = reinterpret_cast<int*>(0x1422CC068);
  client_s* svs_clients = reinterpret_cast<client_s*>(0x14340CB10);

  unsigned short* sv_sconfigstrings = reinterpret_cast<unsigned short*>(0x1464FCDA2);
  unsigned short* sv_emptyConfigString = reinterpret_cast<unsigned short*>(0x1464FCDA0);

  volatile long* sv_thread_owns_game = reinterpret_cast<volatile long*>(0x1422CC530);

  int SV_GetServerThreadOwnsGame()
  {
    return *sv_thread_owns_game;
  }

  void SV_GameDropClient(int clientNum, const char* reason)
  {
    AssertSize(client_s, 0xA67B0);

    assert((*sv_maxclients)->current.integer >= 1 && (*sv_maxclients)->current.integer <= 18);

    if (clientNum >= 0 && clientNum < (*sv_maxclients)->current.integer)
    {
      SV_DropClient(&svs_clients[clientNum], reason, true);
    }
  }

  void SV_DropAllBots()
  {
    for (auto i = 0; i < *svs_clientCount; ++i)
    {
      if (svs_clients[i].header.state != CS_FREE
        && svs_clients[i].header.netchan.remoteAddress.type == NA_BOT)
      {
        SV_GameDropClient(i, "GAME_GET_TO_COVER");
      }
    }
  }

  client_s* SV_GetPlayerByNum()
  {
    if (!(*com_sv_running)->current.enabled)
    {
      return nullptr;
    }

    const int argc (sv_cmd_args->argc[sv_cmd_args->nesting]);
    if (argc < 2)
    {
      Com_Printf(CON_CHANNEL_DONT_FILTER, "No player specified.\n");
      return nullptr;
    }

    const char* s (sv_cmd_args->argv[sv_cmd_args->nesting][1]);
    for (const char* p (s); *p != '\0'; ++p)
    {
      if (*p < '0' || *p > '9')
      {
        Com_Printf(CON_CHANNEL_DONT_FILTER, "Bad slot number: %s\n", s);
        return nullptr;
      }
    }

    const int n (std::atoi(s));
    if (n < 0 || n >= *svs_clientCount)
    {
      Com_Printf(CON_CHANNEL_DONT_FILTER, "Bad client slot: %i\n", n);
      return nullptr;
    }

    client_s* cl (&svs_clients[n]);
    if (cl->header.state == CS_FREE)
    {
      Com_Printf(CON_CHANNEL_DONT_FILTER, "Client %i is not active\n", n);
      return nullptr;
    }

    return cl;
  }

  int SV_GetClientStat(int clientNum, int index)
  {
    assert(svs_clients[clientNum].statPacketsReceived == (1 << MAX_STATPACKETS) - 1);
    assert(svs_clients[clientNum].header.state >= CS_CONNECTED);

    // The stats start with a checksum, which we skip. After it come the 2000
    // stats that are bytes and then the ones that are ints.
    //
    if (index < 2000)
    {
      return svs_clients[clientNum].stats.__s0.binary[index];
    }

    if (index < 3498)
    {
      return svs_clients[clientNum].stats.__s0.data[index - 2000];
    }

    assert(0 && "Unhandled stat index");
    return 0;
  }

  void SV_SetClientStat(int clientNum, int index, int value)
  {
    assert(svs_clients[clientNum].statPacketsReceived == (1 << MAX_STATPACKETS) - 1);
    assert(svs_clients[clientNum].header.state >= CS_CONNECTED);

    if (index < 2000)
    {
      assert(value >= 0 && value <= 255);

      if (svs_clients[clientNum].stats.__s0.binary[index] == value)
      {
        return;
      }

      svs_clients[clientNum].stats.__s0.binary[index] = static_cast<unsigned char>(value);
    }
    else if (index < 3498)
    {
      if (svs_clients[clientNum].stats.__s0.data[index - 2000] == value)
      {
        return;
      }

      svs_clients[clientNum].stats.__s0.data[index - 2000] = value;
    }
    else
    {
      assert(0 && "Unhandled stat index");
      return;
    }

    // Note that on x86 we send this as 'M', which x64 uses for a command of
    // its own, so we send it as 'G', which x64 doesn't (see
    // Components::Stats).
    //
    SV_SendServerCommand(&svs_clients[clientNum], SV_CMD_RELIABLE, "%c %i %i", 'G', index, value);
  }

  std::optional<bool> sv_spawnCheats;

  namespace
  {
    // Return the argument at index of the server command we are executing,
    // or the empty string if the command has fewer arguments.
    //
    const char* SV_Argv(int index)
    {
      return index < sv_cmd_args->argc[sv_cmd_args->nesting]
        ? sv_cmd_args->argv[sv_cmd_args->nesting][index]
        : "";
    }

    // Load the map and spawn the server on it. The command we are executing
    // decides whether the new server has cheats enabled: devmap enables them
    // and map disables them.
    //
    void SV_Map(const char* map, bool mapIsPreloaded)
    {
      com_errorPrintsCount = 0;

      // The map can also be given as its BSP path (maps/mp/<map>.d3dbsp), in
      // which case we strip it down to the map name since that's what the
      // map's zone is called. We also lower-case the name and replace '%'
      // with '_', the same as the game.
      //
      // Note that x86 strips the extension without checking that the name is
      // long enough to have one.
      //
      char name[64];
      {
        if (I_strnicmp(map, "maps/mp/", 8) == 0)
        {
          map += 8;
        }

        std::size_t n (std::strlen(map));
        if (n >= 7 && I_stricmp(map + n - 3, "bsp") == 0)
        {
          n -= 7; // .d3dbsp
        }

        n = std::min(n, sizeof(name) - 1);
        std::memcpy(name, map, n);
        name[n] = '\0';

        std::ranges::replace(std::span(name, n), '%', '_');
        I_strlwr(name);
      }

      // @@ On x86 the map can also be a savegame (<name>.svg), whose header
      //    says which map to load. x64 compiles out reading the header, so
      //    for now we don't support it.
      //
      const int savegame (*sv_migrate != 0 ? 1 : 0);

      // Make sure we have the map. With a mod loaded, it can also be one of
      // the user maps.
      //
      if (!DB_FileExists(name, FFD_DEFAULT) &&
          ((*fs_gameDirVar)->current.string[0] == '\0' || !DB_FileExists(name, FFD_USER_MAP)))
      {
        Com_PrintError(CON_CHANNEL_ERROR, "Can't find map \"%s\".\nA mod is required for custom maps\n", name);
        return;
      }

      const bool devmap (I_stricmp(SV_Argv(0), "devmap") == 0);

      FS_ConvertPath(name);

      sv_spawnCheats = devmap;
      SV_SpawnServer(name, mapIsPreloaded, savegame, devmap, *sv_migrate);
      sv_spawnCheats.reset();
    }
  }

  void SV_Map_f()
  {
    const char* map (SV_Argv(1));
    if (*map == '\0')
    {
      return;
    }

    const bool mapIsPreloaded (std::atoi(SV_Argv(2)) != 0);
    *sv_migrate = std::atoi(SV_Argv(3));

    SV_Map(map, mapIsPreloaded);
  }

  void SV_DevMap_f()
  {
    AssertOffset(PartyData, inParty, 0x1814);

    const char* map (SV_Argv(1));
    if (*map == '\0')
    {
      return;
    }

    if (g_partyData->inParty)
    {
      Party_StopParty(g_partyData);
    }

    if (g_lobbyData->inParty)
    {
      Party_StopParty(g_lobbyData);
    }

    Com_Shutdown("EXE_SERVERKILLED");

    // Leave the online game. While we do, the change doesn't count as a
    // change to the user or system info, which the game would otherwise send
    // to the server and the clients.
    //
    int* allowed (Sys::GetTls<int>(Sys::TLS_OFFSET::DVAR_ALLOWED_MODIFIED_FLAGS));
    const int flags (*allowed);

    *allowed = ~(DVAR_USERINFO | DVAR_SYSTEMINFO);
    Dvar_SetBool(*onlinegame, false);
    *allowed = flags;

    // Note that x86 first checks that controller 0 is a local controller,
    // which it always is, and so we don't.
    //
    LiveStorage_EndGame(0);

    SV_Map(map, false);
  }
}
