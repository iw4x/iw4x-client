#include "Operator.hpp"
#include "Events.hpp"

#include "GSC/ScriptError.hpp"

namespace Components
{
  namespace
  {
    const auto SV_MigrationStart = reinterpret_cast<void (*) (const char*)> (0x14023CD50);
    const auto SV_MigrationInProgress = reinterpret_cast<bool (*) ()> (0x14023C750);
    const auto SV_MapRestart = reinterpret_cast<void (*) (int, int)> (0x140234890);
    const auto SV_FastRestart_f = reinterpret_cast<void (*) ()> (0x140234650);
    const auto SV_GameClientNum = reinterpret_cast<Game::playerState_s* (*) (int)> (0x1402319A0);
    const auto G_GetClientState = reinterpret_cast<Game::clientState_s* (*) (int)> (0x14019CF10);
    const auto BG_GetPerkIndexForName = reinterpret_cast<unsigned int (*) (const char*)> (0x14008D7C0);
    const auto PartyHost_RemoveMember = reinterpret_cast<void (*) (Game::PartyData*, int, bool, const char*)> (0x140110F70);
    const auto SND_FadeAllSounds = reinterpret_cast<void (*) (float, int)> (0x140243E30);
    const auto Dvar_InfoString = reinterpret_cast<const char* (*) (int, int)> (0x1401FC010);
    const auto MemFile_WriteData = reinterpret_cast<void (*) (void*, int, const void*)> (0x140288970);
    const auto MemFile_ReadData = reinterpret_cast<void (*) (void*, int, void*)> (0x140287A60);

    const auto& xblive_privatematch = *reinterpret_cast<const Game::dvar_t* const*> (0x146784ED0);
    const auto& sv_kickBanTime = *reinterpret_cast<const Game::dvar_t* const*> (0x14650B738);
    const auto& sv_rejoinTimeout = *reinterpret_cast<const Game::dvar_t* const*> (0x14650B6A8);

    // Whether the server restarts the map on its next frame, and whether that
    // restart reloads the scripts. x64 still keeps both, presumably for its
    // own fast_restart, which is lucky for us.
    //
    auto& sv_restartMap = *reinterpret_cast<int*> (0x1422CC060);
    auto& sv_restartLoadScripts = *reinterpret_cast<int*> (0x1422CC064);

    constexpr int CS_LOCALIZED_STRINGS = 0x1FF;
    constexpr int CS_SHADERS = 0x9B5;

    // The briefly banned guids along with when we banned them. They are laid
    // out as on x86, since we archive them for the new host the same way.
    //
    struct TempBan
    {
      char guid[17];
      int time;
    };

    static_assert (sizeof (TempBan) == 0x18);
    static_assert (offsetof (TempBan, time) == 0x14);

    TempBan TempBans[16];

    static_assert (sizeof (TempBans) == 0x180);

    // Return the slot of the first free ban, or else of the oldest one.
    //
    int SV_TempBanGetFreeSlot ()
    {
      int r (0);

      for (int i (0); i < static_cast<int> (std::size (TempBans)); ++i)
      {
        if (TempBans[i].guid[0] == '\0')
        {
          return i;
        }

        if (TempBans[i].time < TempBans[r].time)
        {
          r = i;
        }
      }

      return r;
    }

    void SV_BanGuidBriefly (const char* guid)
    {
      TempBan& b (TempBans[SV_TempBanGetFreeSlot ()]);

      std::memcpy (b.guid, guid, sizeof (b.guid));
      b.time = *Game::svs_time;
    }

    bool SV_IsTempBannedGuid (const char* guid)
    {
      if (*guid == '\0')
      {
        return false;
      }

      for (const TempBan& b : TempBans)
      {
        if (std::strcmp (b.guid, guid) == 0 &&
            static_cast<double> (*Game::svs_time - b.time) <= sv_kickBanTime->current.value * 1000.0)
        {
          return true;
        }
      }

      return false;
    }

    // Kick the client for the reason. If we pass a buffer for the player's
    // name, then also return their name without its colors along with their
    // guid.
    //
    // The host can't be kicked, for obvious reasons. In an online game that
    // isn't a private match the server migrates to another host instead, in
    // which case we return true without filling the buffers.
    //
    bool SV_KickClient (Game::client_s* cl, char* playerName, int maxPlayerNameLen, const char* reason, char* guid)
    {
      if (cl->header.netchan.remoteAddress.type == Game::NA_LOOPBACK)
      {
        if (xblive_privatematch->current.enabled || !(*Game::onlinegame)->current.enabled)
        {
          Game::SV_SendServerCommand (nullptr, Game::SV_CMD_CAN_IGNORE, "%c \"EXE_CANNOTKICKHOSTPLAYER\"", 'e');
          return false;
        }

        if (!SV_MigrationInProgress ())
        {
          SV_MigrationStart (reason);
        }

        return true;
      }

      if (playerName)
      {
        Game::I_strncpyz (playerName, cl->name, maxPlayerNameLen);
        Game::I_CleanStr (playerName);
        std::memcpy (guid, cl->playerGuid, sizeof (cl->playerGuid));
      }

      Game::SV_DropClient (cl, reason, true);
      cl->lastPacketTime = *Game::svs_time;
      return true;
    }

    // Kick the player whose name is the first argument (or everyone, if it's
    // all) for the reason that is the second.
    //
    // Note that we only return true for a single player that we kicked. And
    // all only means everyone if no player happens to be called that. What if
    // someone actually is? Then they alone get kicked, which is x86's quirk
    // rather than ours, and probably not worth fixing.
    //
    bool SV_KickUser (const Command::Params* params, char* playerName, int maxPlayerNameLen, char* guid)
    {
      if (!(*Game::com_sv_running)->current.enabled)
      {
        Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "Server is not running.\n");
        return false;
      }

      if (params->size () < 2)
      {
        const char* cmd (params->get (0));
        Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "Usage: %s <player name> <optional reason>\n%s all = kick everyone\n", cmd, cmd);
        return false;
      }

      const char* reason (params->size () == 3 ? params->get (2) : "EXE_PLAYERKICKED");

      if (Game::client_s* cl = Game::SV_GetPlayerByName ())
      {
        return SV_KickClient (cl, playerName, maxPlayerNameLen, reason, guid);
      }

      if (Game::I_stricmp (params->get (1), "all") == 0)
      {
        for (int i (0); i < *Game::svs_clientCount; ++i)
        {
          if (Game::svs_clients[i].header.state != Game::CS_FREE)
          {
            SV_KickClient (&Game::svs_clients[i], nullptr, 0, reason, nullptr);
          }
        }
      }

      return false;
    }

    // Kick the client whose slot number is the first argument for the reason
    // that is the second.
    //
    bool SV_KickClientNum (const Command::Params* params, char* playerName, int maxPlayerNameLen, char* guid)
    {
      if (!(*Game::com_sv_running)->current.enabled)
      {
        Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "Server is not running.\n");
        return false;
      }

      if (params->size () < 2)
      {
        Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "Usage: %s <client number> <optional reason>\n", params->get (0));
        return false;
      }

      const char* reason (params->size () == 3 ? params->get (2) : "EXE_PLAYERKICKED");

      Game::client_s* cl (Game::SV_GetPlayerByNum ());
      return cl != nullptr && SV_KickClient (cl, playerName, maxPlayerNameLen, reason, guid);
    }

    // Kick a player and briefly ban their guid.
    //
    // Note that x86 leaves the name and the guid uninitialized when the host
    // migrates rather than being kicked, so presumably it bans whatever
    // happens to be on the stack. We start them empty instead, which bans no
    // guid.
    //
    template <bool (*kick) (const Command::Params*, char*, int, char*)>
    void SV_Ban_f (const Command::Params* params)
    {
      char playerName[64] {};
      char guid[20] {};

      if (kick (params, playerName, sizeof (playerName), guid))
      {
        Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "%s (guid \"%s\") was kicked for cheating\n", playerName, guid);
        SV_BanGuidBriefly (guid);
      }
    }

    // Drop the client if we haven't heard from them for sv_rejoinTimeout
    // seconds, and unregister a free slot from the party. Return true if we
    // did either.
    //
    bool SV_DropClientIfInactive (int clientNum, const char* reason, bool tellThem)
    {
      Game::client_s* cl (&Game::svs_clients[clientNum]);

      if (cl->header.state == Game::CS_FREE)
      {
        Game::Com_Printf (Game::CON_CHANNEL_SERVER, "Unregistering client %i from session because they are CS_FREE in SV_DropClientIfInactive.\n", clientNum);
        PartyHost_RemoveMember (Game::g_lobbyData, clientNum, false, "");
        return true;
      }

      if (*Game::svs_time - cl->lastPacketTime < sv_rejoinTimeout->current.integer * 1000)
      {
        return false;
      }

      Game::SV_DropClient (cl, reason, tellThem);
      return true;
    }

    // Return the length of the string as drawn, that is, without its color
    // codes.
    //
    int I_DrawStrlen (const char* s)
    {
      int r (0);

      while (*s != '\0')
      {
        if (s[0] == '^' && s[1] != '\0' && s[1] != '^' && s[1] >= '0' && s[1] <= '9')
        {
          s += 2;
        }
        else
        {
          ++r;
          ++s;
        }
      }

      return r;
    }

    // Print the info string with a key and its value per line, the keys padded
    // to a column.
    //
    void Info_Print (const char* s)
    {
      if (*s == '\\')
      {
        ++s;
      }

      while (*s != '\0')
      {
        std::string key;
        while (*s != '\0' && *s != '\\')
        {
          key += *s++;
        }

        if (key.size () < 20)
        {
          key.resize (20, ' ');
        }

        Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "%s", key.c_str ());

        if (*s == '\0')
        {
          Game::Com_Printf (Game::CON_CHANNEL_SYSTEM, "MISSING VALUE\n");
          return;
        }

        ++s;

        std::string value;
        while (*s != '\0' && *s != '\\')
        {
          value += *s++;
        }

        if (*s != '\0')
        {
          ++s;
        }

        Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "%s\n", value.c_str ());
      }
    }

    void SV_Status_f ()
    {
      if (!(*Game::com_sv_running)->current.enabled)
      {
        Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "Server is not running.\n");
        return;
      }

      Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "map: %s\n", (*Game::sv_mapname)->current.string);
      Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "num score ping guid                             name            lastmsg address               qport rate\n");
      Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "--- ----- ---- -------------------------------- --------------- ------- --------------------- ----- -----\n");

      for (int i (0); i < *Game::svs_clientCount; ++i)
      {
        const Game::client_s* cl (&Game::svs_clients[i]);

        if (cl->header.state == Game::CS_FREE)
        {
          continue;
        }

        Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "%3i ", i);
        Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "%5i ", Game::SV_GameClientNum_Score (i));

        if (cl->header.state == Game::CS_CONNECTED)
        {
          Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "CNCT ");
        }
        else if (cl->header.state == Game::CS_ZOMBIE)
        {
          Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "ZMBI ");
        }
        else
        {
          Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "%4i ", std::min (cl->ping, 9999));
        }

        Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "%32s ", cl->playerGuid);
        Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "%s^7", cl->name);

        for (int n (16 - I_DrawStrlen (cl->name)); n > 0; --n)
        {
          Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, " ");
        }

        Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "%7i ", *Game::svs_time - cl->lastPacketTime);

        const char* address (Game::NET_AdrToString (cl->header.netchan.remoteAddress));
        Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "%s", address);

        for (int n (22 - static_cast<int> (std::strlen (address))); n > 0; --n)
        {
          Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, " ");
        }

        Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "%5i", cl->header.netchan.qport);
        Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, " %5i", cl->rate);
        Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "\n");
      }

      Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "\n");
    }

    void SV_DumpUser_f (const Command::Params* params)
    {
      if (!(*Game::com_sv_running)->current.enabled)
      {
        Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "Server is not running.\n");
        return;
      }

      if (params->size () != 2)
      {
        Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "Usage: info <userid>\n");
        return;
      }

      if (const Game::client_s* cl = Game::SV_GetPlayerByName ())
      {
        Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "userinfo\n");
        Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "--------\n");
        Info_Print (cl->userinfo);
      }
    }

    // Restart the map on the server's next frame, or right away if the server
    // isn't running, which also reloads the scripts.
    //
    void SV_MapRestart_f ()
    {
      SND_FadeAllSounds (0.0f, 0);

      sv_restartMap = 1;
      sv_restartLoadScripts = 1;
      *Game::sv_migrate = 0;

      if (!(*Game::com_sv_running)->current.enabled)
      {
        SV_MapRestart (0, 1);
      }
    }

    void SV_SetPerk_f (const Command::Params* params)
    {
      Game::client_s* cl (Game::SV_GetPlayerByName ());
      if (cl == nullptr)
      {
        return;
      }

      const char* name (params->get (2));
      const unsigned int perk (BG_GetPerkIndexForName (name));

      if (perk >= Game::PERK_COUNT)
      {
        Game::Com_DPrintf (Game::CON_CHANNEL_DONT_FILTER, "Unknown perk: %s\n", name);
        return;
      }

      const int clientNum (static_cast<int> (cl - Game::svs_clients));
      const unsigned int bit (1u << (perk & 31));

      SV_GameClientNum (clientNum)->perks[perk >> 5] |= bit;
      G_GetClientState (clientNum)->perks[perk >> 5] |= bit;
    }

    void SV_DumpHudElems_f ()
    {
      // The names of the hud element types and of the teams.
      //
      constexpr const char* types[] {
        "free", "text", "value", "playername", "mapname", "gametype", "material", "timer down", "timer up",
        "timer static", "timer down tenths", "timer up tenths", "timer static tenths", "clock down", "clock up",
        "waypoint", "off", "all ents", "player bounding boxes"};

      constexpr const char* teams[] {"invalid", "axis", "allies", "spectator"};

      int archived (0);
      int current (0);

      for (int i (0); i < 1024; ++i)
      {
        const Game::game_hudelem_s& e (Game::g_hudelems[i]);

        if (e.elem.type == Game::HE_TYPE_FREE)
        {
          continue;
        }

        assert (static_cast<std::size_t> (e.elem.type) < std::size (types));
        assert (static_cast<std::size_t> (e.team) < std::size (teams));

        Game::Com_Printf (Game::CON_CHANNEL_PARSERSCRIPT, "%12s", types[e.elem.type]);

        if (e.team != 0)
        {
          Game::Com_Printf (Game::CON_CHANNEL_PARSERSCRIPT, "\t%s", teams[e.team]);
        }

        if (e.clientNum != static_cast<int> (Game::ENTITYNUM_NONE))
        {
          Game::Com_Printf (Game::CON_CHANNEL_PARSERSCRIPT, "\tclient %i", e.clientNum);
        }

        if (e.archived)
        {
          Game::Com_Printf (Game::CON_CHANNEL_PARSERSCRIPT, "\tarchived");
          ++archived;
        }
        else
        {
          Game::Com_Printf (Game::CON_CHANNEL_PARSERSCRIPT, "\tcurrent");
          ++current;
        }

        const auto configstring ([] (int index)
        {
          return Game::SL_ConvertToString (static_cast<Game::scr_string_t> (Game::SV_GetConfigstringConst (index)));
        });

        Game::Com_Printf (Game::CON_CHANNEL_PARSERSCRIPT, "\ttext \"%s\"", configstring (CS_LOCALIZED_STRINGS + e.elem.text));
        Game::Com_Printf (Game::CON_CHANNEL_PARSERSCRIPT, "\tmaterial \"%s\"", configstring (CS_SHADERS + e.elem.materialIndex));
        Game::Com_Printf (Game::CON_CHANNEL_PARSERSCRIPT, "\n");
      }

      Game::Com_Printf (Game::CON_CHANNEL_PARSERSCRIPT, "\n%i archived + %i current = %i hud elems\n", archived, current, archived + current);
    }

    // The script variables, laid out as on x86 but with the values widened.
    //
    // The layout takes a bit of explaining. An object's entry follows the
    // entry its id indexes, and the first word of its value is its reference
    // count, while the sibling is the first of its children. The children
    // live at one of two bases, depending on the object's id, and each one's
    // sibling indexes the hash, whose id is that of the next child. It's the
    // game's own scheme, not ours, and we only follow it far enough to count.
    // If the counts ever look off, then this is probably where we got it
    // wrong.
    //
    struct VariableValueInternal
    {
      std::uint16_t id;
      std::uint16_t prev;
      Game::VariableUnion u;
      unsigned int status;
      std::uint16_t name;
      std::uint16_t nextSibling;
    };

    static_assert (sizeof (VariableValueInternal) == 0x18);
    static_assert (offsetof (VariableValueInternal, u) == 0x8);
    static_assert (offsetof (VariableValueInternal, status) == 0x10);
    static_assert (offsetof (VariableValueInternal, nextSibling) == 0x16);

    const auto variableList = reinterpret_cast<const VariableValueInternal*> (0x141F4B800);

    constexpr unsigned int VARIABLELIST_COUNT = 0x22002;
    constexpr unsigned int VAR_STAT_MASK = 0x60;
    constexpr unsigned int VAR_MASK = 0x1F;
    constexpr unsigned int VAR_NAME_HIGH_MASK = 0x10000;

    // The parent of the endon lists, which scrVarPub keeps after the time
    // array.
    //
    const auto& pauseArrayId = *reinterpret_cast<const unsigned int*> (0x141F27794);

    // The script classes and the arrays of their entities.
    //
    struct scr_classStruct_t
    {
      std::uint16_t id;
      std::uint16_t entArrayId;
      char charId;
      const char* name;
    };

    static_assert (sizeof (scr_classStruct_t) == 0x10);

    const auto g_classMap = reinterpret_cast<const scr_classStruct_t*> (0x140423CD0);

    constexpr int CLASS_NUM_COUNT = 6;

    unsigned int ChildBase (unsigned int parentId)
    {
      return (parentId & 1) * 0xC801 + 0x9002;
    }

    const VariableValueInternal& ObjectEntry (unsigned int id)
    {
      return variableList[id + 1];
    }

    unsigned int Type (const VariableValueInternal& e)
    {
      return e.status & VAR_MASK;
    }

    // Return the id of the child that follows the one at index, or 0 if it's
    // the last.
    //
    unsigned int NextChild (unsigned int index, unsigned int base)
    {
      const unsigned int next (variableList[index + base].nextSibling);
      return next != 0 ? variableList[next + base].id : 0;
    }

    unsigned int RefCount (unsigned int id)
    {
      return *reinterpret_cast<const std::uint16_t*> (&ObjectEntry (id).u);
    }

    // Return the number of variables the object uses, counting each array it
    // points to as shared among its references.
    //
    float Scr_GetObjectUsage (unsigned int parentId)
    {
      float r (1.0f);
      const unsigned int base (ChildBase (parentId));

      for (unsigned int id (ObjectEntry (parentId).nextSibling); id != 0; id = NextChild (id, base))
      {
        const VariableValueInternal& e (variableList[id + base]);
        const unsigned int value (e.u.pointerValue);

        float usage (0.0f);
        if (Type (e) == Game::VAR_POINTER && Type (ObjectEntry (value)) == Game::VAR_ARRAY)
        {
          usage = Scr_GetObjectUsage (value) / (static_cast<float> (RefCount (value)) + 1.0f);
        }

        r += usage + 1.0f;
      }

      return r;
    }

    // Return the number of variables the thread's endon list uses.
    //
    float Scr_GetEndonUsage (unsigned int localId)
    {
      const unsigned int id (Game::FindVariable (pauseArrayId, localId + VAR_NAME_HIGH_MASK));
      if (id == 0)
      {
        return 0.0f;
      }

      const unsigned int endonId (variableList[id + ChildBase (pauseArrayId)].u.pointerValue);
      const unsigned int base (ChildBase (endonId));

      float r (1.0f);
      for (unsigned int child (ObjectEntry (endonId).nextSibling); child != 0; child = NextChild (child, base))
      {
        r += 1.0f;
      }

      return r;
    }

    // Return the number of variables the suspended thread uses, and the number
    // its endon lists use in endonUsage. We walk its stack from the top, where
    // each code position is a call into the thread's parent.
    //
    float Scr_GetThreadUsage (const Game::VariableStackBuffer* stackBuf, float* endonUsage)
    {
      constexpr std::size_t entry_size (1 + sizeof (Game::VariableUnion));

      unsigned int localId (stackBuf->localId);
      float r (Scr_GetObjectUsage (localId));
      *endonUsage = Scr_GetEndonUsage (localId);

      const char* p (stackBuf->buf + stackBuf->size * entry_size);

      for (int size (stackBuf->size); size != 0; --size)
      {
        p -= entry_size;

        const unsigned char type (static_cast<unsigned char> (p[0]));

        Game::VariableUnion u;
        std::memcpy (&u, p + 1, sizeof (u));

        if (type == Game::VAR_CODEPOS)
        {
          localId = ObjectEntry (localId).status >> 8;
          r += Scr_GetObjectUsage (localId);
          *endonUsage += Scr_GetEndonUsage (localId);
        }
        else if (type == Game::VAR_POINTER && Type (ObjectEntry (u.pointerValue)) == Game::VAR_ARRAY)
        {
          r += Scr_GetObjectUsage (u.pointerValue) / (static_cast<float> (RefCount (u.pointerValue)) + 1.0f);
        }
      }

      return r;
    }

    // A suspended thread's code positions, from where it is back to where it
    // was started, along with the variables it uses.
    //
    struct ThreadDebugInfo
    {
      const char* pos[32];
      int posSize;
      float varUsage;
      float endonUsage;
    };

    int ThreadInfoCompare (const void* a, const void* b)
    {
      const auto* x (static_cast<const ThreadDebugInfo*> (a));
      const auto* y (static_cast<const ThreadDebugInfo*> (b));

      for (int i (0); i < x->posSize && i < y->posSize; ++i)
      {
        if (x->pos[i] != y->pos[i])
        {
          return x->pos[i] < y->pos[i] ? -1 : 1;
        }
      }

      return x->posSize - y->posSize;
    }

    // Print the suspended threads grouped by where they are, and how many
    // variables each group uses (and the entities of each class).
    //
    // Note that x86 prints nothing if no thread is suspended, and says it's
    // out of memory if it can't allocate the threads. We throw in that case,
    // which seems more honest.
    //
    void Scr_DumpScriptThreads ()
    {
      std::vector<ThreadDebugInfo> infos;

      for (unsigned int i (1); i <= VARIABLELIST_COUNT; ++i)
      {
        const VariableValueInternal& e (variableList[i]);

        if ((e.status & VAR_STAT_MASK) == 0 || Type (e) != Game::VAR_STACK)
        {
          continue;
        }

        const Game::VariableStackBuffer* stackBuf (e.u.stackValue);
        constexpr std::size_t entry_size (1 + sizeof (Game::VariableUnion));

        // Collect the code positions from the bottom of the stack, followed by
        // where the thread is, storing them in reverse order.
        //
        const char* pos[32];
        int n (0);

        for (int j (0); j < stackBuf->size; ++j)
        {
          const char* p (stackBuf->buf + j * entry_size);

          if (static_cast<unsigned char> (p[0]) == Game::VAR_CODEPOS)
          {
            assert (n < static_cast<int> (std::size (pos)) - 1);
            std::memcpy (&pos[n++], p + 1, sizeof (pos[0]));
          }
        }

        pos[n++] = stackBuf->pos;

        ThreadDebugInfo info;
        info.varUsage = Scr_GetThreadUsage (stackBuf, &info.endonUsage);
        info.posSize = n;

        for (int j (0); j < n; ++j)
        {
          info.pos[j] = pos[n - 1 - j];
        }

        infos.push_back (info);
      }

      if (infos.empty ())
      {
        return;
      }

      std::qsort (infos.data (), infos.size (), sizeof (ThreadDebugInfo), ThreadInfoCompare);

      Game::Com_Printf (Game::CON_CHANNEL_PARSERSCRIPT, "********************************\n");

      float totalVarUsage (0.0f);
      float totalEndonUsage (0.0f);

      for (std::size_t i (0); i < infos.size ();)
      {
        const ThreadDebugInfo& info (infos[i]);

        int count (0);
        float varUsage (0.0f);
        float endonUsage (0.0f);

        do
        {
          ++count;
          varUsage += infos[i].varUsage;
          endonUsage += infos[i].endonUsage;
          ++i;
        }
        while (i < infos.size () && ThreadInfoCompare (&info, &infos[i]) == 0);

        totalVarUsage += varUsage;
        totalEndonUsage += endonUsage;

        Game::Com_Printf (Game::CON_CHANNEL_PARSERSCRIPT, "count: %d, var usage: %d, endon usage: %d\n", count, static_cast<int> (varUsage), static_cast<int> (endonUsage));
        GSC::ScriptError::Scr_PrintPrevCodePos (Game::CON_CHANNEL_PARSERSCRIPT, info.pos[0], 0);

        for (int j (1); j < info.posSize; ++j)
        {
          Game::Com_Printf (Game::CON_CHANNEL_PARSERSCRIPT, "called from:\n");
          GSC::ScriptError::Scr_PrintPrevCodePos (Game::CON_CHANNEL_PARSERSCRIPT, info.pos[j], 0);
        }
      }

      Game::Com_Printf (Game::CON_CHANNEL_PARSERSCRIPT, "********************************\n");
      Game::Com_Printf (Game::CON_CHANNEL_PARSERSCRIPT, "var usage: %d, endon usage: %d\n", static_cast<int> (totalVarUsage), static_cast<int> (totalEndonUsage));
      Game::Com_Printf (Game::CON_CHANNEL_PARSERSCRIPT, "\n");

      for (int classNum (0); classNum < CLASS_NUM_COUNT; ++classNum)
      {
        const unsigned int entArrayId (g_classMap[classNum].entArrayId);
        if (entArrayId == 0)
        {
          continue;
        }

        const unsigned int base (ChildBase (entArrayId));

        int count (0);
        float usage (0.0f);

        for (unsigned int id (ObjectEntry (entArrayId).nextSibling); id != 0; id = NextChild (id, base))
        {
          const VariableValueInternal& e (variableList[id + base]);

          ++count;

          if (Type (e) == Game::VAR_POINTER)
          {
            usage += Scr_GetObjectUsage (e.u.pointerValue);
          }
        }

        Game::Com_Printf (Game::CON_CHANNEL_PARSERSCRIPT, "ent type '%s'... count: %d, var usage: %d\n", g_classMap[classNum].name, count, static_cast<int> (usage));
      }

      Game::Com_Printf (Game::CON_CHANNEL_PARSERSCRIPT, "********************************\n");
    }
  }

  void Operator::AddServerCommands ()
  {
    // On x86 heartbeat schedules the next heartbeat to the master server,
    // which x86 never actually sends, so all it changes is the time that the
    // server archives for the new host. x64 compiles out both, so we add it as
    // a command that does nothing. Why bother? Because a server config that
    // runs heartbeat would otherwise print an unknown command, and somebody
    // would presumably report that as a bug.
    //
    Command::AddSV ("heartbeat", [] (const Command::Params*) {});

    Command::AddSV ("onlykick", [] (const Command::Params* params) {SV_KickUser (params, nullptr, 0, nullptr);});
    Command::AddSV ("clientkick", [] (const Command::Params* params) {SV_KickClientNum (params, nullptr, 0, nullptr);});
    Command::AddSV ("kick", SV_Ban_f<SV_KickUser>);
    Command::AddSV ("tempBanUser", SV_Ban_f<SV_KickUser>);
    Command::AddSV ("tempBanClient", SV_Ban_f<SV_KickClientNum>);

    Command::AddSV ("dropclientifinactive", [] (const Command::Params* params)
    {
      const int clientNum (std::atoi (params->get (1)));

      if (clientNum < 0 || clientNum >= *Game::svs_clientCount)
      {
        Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "dropclientifinactive: client index %i out of range (max %i)\n", clientNum, *Game::svs_clientCount);
        return;
      }

      SV_DropClientIfInactive (clientNum, "EXE_TIMEDOUT", false);
    });

    Command::AddSV ("status", [] (const Command::Params*) {SV_Status_f ();});

    Command::AddSV ("serverinfo", [] (const Command::Params*)
    {
      Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "Server info settings:\n");
      Info_Print (Dvar_InfoString (0, Game::DVAR_SERVERINFO));
    });

    Command::AddSV ("dumpuser", SV_DumpUser_f);
    Command::AddSV ("map_restart", [] (const Command::Params*) {SV_MapRestart_f ();});

    // Note that x64 still has fast_restart's function, which restarts the map
    // the same way but without reloading the scripts.
    //
    Command::AddSV ("fast_restart", [] (const Command::Params*) {SV_FastRestart_f ();});

    // On x86 gameCompleteStatus does nothing either.
    //
    Command::AddSV ("gameCompleteStatus", [] (const Command::Params*) {});

    Command::AddSV ("killserver", [] (const Command::Params*) {Game::Com_Shutdown ("EXE_SERVERKILLED");});
    Command::AddSV ("scriptUsage", [] (const Command::Params*) {Scr_DumpScriptThreads ();});

    // On x86 dobjList and stringUsage are the stubs of debugging commands,
    // so all they do is print.
    //
    Command::AddSV ("dobjList", [] (const Command::Params*)
    {
      Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "Dobj list is dev-only\n");
    });

    Command::AddSV ("stringUsage", [] (const Command::Params*)
    {
      for (int i (0); i < 3; ++i)
      {
        Game::Com_Printf (Game::CON_CHANNEL_PARSERSCRIPT, "********************************\n");
      }
    });

    Command::AddSV ("dumpHudElems", [] (const Command::Params*) {SV_DumpHudElems_f ();});
    Command::AddSV ("setPerk", SV_SetPerk_f);

    // On x86 connectString prints the client number it's given followed by an
    // empty connect string. Not very useful, but that's what it does.
    //
    Command::AddSV ("connectString", [] (const Command::Params* params)
    {
      Game::Com_Printf (Game::CON_CHANNEL_DONT_FILTER, "%i %s\n", std::atoi (params->get (1)), "");
    });
  }

  Operator::
  Operator ()
  {
    // Reject the challenge of a briefly banned guid, which the client sends as
    // the second argument of getchallenge. We check once SV_GetChallenge() has
    // found the client's challenge slot, which we then clear.
    //
    Utils::Hook (0x140236C0E, Operator_SV_GetChallengeStub, HOOK_JUMP).install ()->quick ();

    // Carry the brief bans over to the new host. They go right after the value
    // that the server archives in place of x86's heartbeat time and its other
    // value. What those two values are on x64 we aren't sure, but as long as
    // we put ours after them, it doesn't really matter.
    //
    Utils::Hook (0x1401A2921, Operator_ArchiveTempBansStub, HOOK_JUMP).install ()->quick ();
    Utils::Hook (0x1401A1B37, Operator_RestoreTempBansStub, HOOK_JUMP).install ()->quick ();

    Events::OnSVInit (AddServerCommands);
  }
}

bool
Operator_SV_GetChallenge (const char* guid, Game::netadr_t* challenge)
{
  using namespace Components;

  if (!SV_IsTempBannedGuid (guid))
  {
    return false;
  }

  Game::Com_Printf (Game::CON_CHANNEL_SERVER, "rejected connection from temporarily banned GUID \"%s\"\n", guid);
  Game::NET_OutOfBandPrint (Game::NS_SERVER, *challenge, "error\nEXE_ERR_BANNED_TEMP");

  std::memset (challenge, 0, 0x40);
  return true;
}

void
Operator_ArchiveTempBans (void* memFile)
{
  using namespace Components;

  MemFile_WriteData (memFile, sizeof (TempBans), TempBans);
}

void
Operator_RestoreTempBans (void* memFile)
{
  using namespace Components;

  MemFile_ReadData (memFile, sizeof (TempBans), TempBans);
}
