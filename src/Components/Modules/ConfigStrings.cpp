#include "ConfigStrings.hpp"

namespace Components
{
  namespace
  {
    // The game's gameState_t, only with room for our configstrings.
    //
    struct GameState
    {
      int stringOffsets[ConfigStrings::MAX_CONFIGSTRINGS];
      char stringData[0x20000];
      int dataCount;
    };

    constexpr uintptr_t ImageBase = 0x140000000;

    // On x64 the client's gameState_t lives inside clientStatic_t...
    //
    constexpr uintptr_t ClientStatic = 0x140C5AC20;
    constexpr uintptr_t ClientGameState = 0x140C9B960;

    constexpr uint32_t ClientOffsetsSize = ConfigStrings::BASEGAME_MAX_CONFIGSTRINGS * sizeof(int);
    constexpr uint32_t ClientGameStateSize = ClientOffsetsSize + sizeof(GameState::stringData) + sizeof(int);

    // ...and the server's configstrings inside server_t.
    //
    constexpr uintptr_t Server = 0x1464FBD80;
    constexpr uintptr_t ServerConfigStrings = 0x1464FCDA2;

    GameState* clGameState;
    uint16_t* svConfigStrings;

    // Return where the client's gameState_t field at the game's address a
    // lives now. Since the offsets grow, the fields after them don't all move
    // by the same amount, so we have to figure out which of the three a points
    // into.
    //
    uintptr_t Moved(uintptr_t a)
    {
      const uintptr_t data = ClientGameState + ClientOffsetsSize;
      const uintptr_t count = data + sizeof(GameState::stringData);

      if (a < data)
      {
        return reinterpret_cast<uintptr_t>(clGameState->stringOffsets) + (a - ClientGameState);
      }

      if (a < count)
      {
        return reinterpret_cast<uintptr_t>(clGameState->stringData) + (a - data);
      }

      assert(a == count);
      return reinterpret_cast<uintptr_t>(&clGameState->dataCount);
    }

    // An instruction we patch, by its address and length.
    //
    struct Instruction
    {
      uintptr_t place;
      size_t length;
    };

    // An instruction that reaches the game's address target.
    //
    struct Reference
    {
      uintptr_t place;
      size_t length;
      uintptr_t target;
    };
  }

  void ConfigStrings::PatchConfigStrings()
  {
    // The game keeps both the client's and the server's configstrings in
    // fixed-size arrays inside bigger structures (clientStatic_t and server_t),
    // so there is no room to grow them where they are. Instead we move both to
    // our own memory and point every reference at it. We allocate near the
    // image since most references are 32-bit displacements from the next
    // instruction (or offsets from the image base or the enclosing structure),
    // and those have to reach the new place.
    //
    // Fair warning: this means a lot of addresses below, and they are tedious
    // to get right. If you add one, check how the instruction reaches the
    // game's address, which tells you which of the loops it belongs in.
    //
    clGameState = static_cast<GameState*>(Utils::Hook::AllocateNear(ImageBase, sizeof(GameState)));
    svConfigStrings = static_cast<uint16_t*>(Utils::Hook::AllocateNear(ImageBase, MAX_CONFIGSTRINGS * sizeof(uint16_t)));

    // The client's gameState_t, reached from the next instruction. These are in
    // CL_ParseGamestate(), the demo recording (CL_Record_f()), and the client
    // code that clears, reads, and changes a configstring (we haven't bothered
    // naming each of those functions).
    //
    // The same, reached from the image base...
    //
    // ...and from clientStatic_t, all of which are in CL_ParseGamestate().
    //
    // The server's configstrings, reached from the next instruction, in
    // SV_SendClientGameState(), SV_SpawnServer(), and the functions that get
    // and set a configstring...
    //
    // ...and from server_t. Note that the game keeps the empty configstring in
    // the word right before them, which we leave where it is.
    //
    for (const Reference& s: std::initializer_list<Reference>
    {
      {0x1400F3F68, 7, 0x140C9FD2C}, {0x1400F3FB7, 7, 0x140C9B960}, {0x1400F3FD3, 7, 0x140C9B960},
      {0x1400F3FF1, 6, 0x140CBFD2C}, {0x1400F3FF7, 7, 0x140C9FD2C}, {0x1400F404B, 6, 0x140CBFD2C},
      {0x1400F405C, 7, 0x140CBFD2C}, {0x1400F4071, 6, 0x140CBFD2C}, {0x1400F407B, 6, 0x140CBFD2C},
      {0x1400F47A3, 7, 0x140C9B960}, {0x1400F47AE, 7, 0x140C9FD2C}, {0x1400F4C41, 7, 0x140C9B960},
      {0x1400F4C48, 7, 0x140C9FD2C}, {0x1400FC4FA, 7, 0x140C9B960}, {0x1400FCFE8, 7, 0x140C9B960},
      {0x1400FFD2E, 7, 0x140C9B960}, {0x1400FFD65, 10, 0x140CBFD2C}, {0x1400FFD84, 7, 0x140C9FD2C},
      {0x1400FFDDD, 7, 0x140C9FD2C}, {0x1400FFF40, 6, 0x140CBFD2C}, {0x1400FFF53, 7, 0x140CBFD2C},
      {0x1400FFF65, 6, 0x140CBFD2C}, {0x1400FFF72, 6, 0x140CBFD2C}, {0x1400FFFC9, 6, 0x140CBFD2C},
      {0x1400FFFEC, 6, 0x140CBFD2C}, {0x140100006, 7, 0x140CBFD2C}, {0x14010001B, 6, 0x140CBFD2C},
      {0x14010002C, 6, 0x140CBFD2C}, {0x14010004B, 7, 0x140C9FD2C}, {0x14010007B, 6, 0x140CBFD2C},
      {0x14010008E, 7, 0x140CBFD2C}, {0x1401000A0, 6, 0x140CBFD2C}, {0x1401000AD, 6, 0x140CBFD2C}
    })
    {
      Utils::Hook::Displace(s.place, s.length, s.target, Moved(s.target));
    }

    for (const Reference& s: std::initializer_list<Reference>
    {
      {0x1400F3F60, 8, 0x140C9B960}, {0x1400F4051, 8, 0x140C9B960},
      {0x1400FD0E1, 8, 0x140C9B960}, {0x1400FD124, 8, 0x140C9B960}
    })
    {
      Utils::Hook::Rebase(s.place, s.length, ImageBase, s.target, Moved(s.target));
    }

    for (const Reference& s: std::initializer_list<Reference>
    {
      {0x1400FFF49, 7, 0x140C9B960}, {0x1400FFFFC, 7, 0x140C9B960}, {0x140100084, 7, 0x140C9B960}
    })
    {
      Utils::Hook::Rebase(s.place, s.length, ClientStatic, s.target, Moved(s.target));
    }

    for (const Reference& s: std::initializer_list<Reference>
    {
      {0x140237382, 7, 0x1464FCDA2}, {0x14023742E, 7, 0x1464FCDA2}, {0x14023866A, 7, 0x1464FCDA2},
      {0x1402386F0, 7, 0x1464FCDA2}, {0x140238723, 7, 0x1464FCDA2}, {0x140239214, 7, 0x1464FCDA2},
      {0x140239A6E, 7, 0x1464FCDA2}
    })
    {
      Utils::Hook::Displace(s.place, s.length, s.target, reinterpret_cast<uintptr_t>(svConfigStrings));
    }

    for (const Reference& s: std::initializer_list<Reference>
    {
      {0x140238FED, 7, 0x1464FCDA2}, {0x14023904A, 7, 0x1464FCDA2},
      {0x14023909D, 7, 0x1464FCDA2}, {0x1402390FA, 7, 0x1464FCDA2}
    })
    {
      Utils::Hook::Rebase(s.place, s.length, Server, s.target, reinterpret_cast<uintptr_t>(svConfigStrings));
    }

    // SV_SpawnServer() fills the configstrings with the empty string up to
    // their end, which on x64 also happens to be the start of the next global.
    // So this is the one reference to their end rather than their start.
    //
    Utils::Hook::Displace(0x140239BFA, 7,
                          ServerConfigStrings + BASEGAME_MAX_CONFIGSTRINGS * sizeof(uint16_t),
                          reinterpret_cast<uintptr_t>(svConfigStrings + MAX_CONFIGSTRINGS));

    // Now the counts. These are the bounds checks on an index in the client,
    // the server, and what seems to be the save game.
    //
    // Note that CG_ConfigStringModified() keeps bounds of its own and so never
    // sees ours, which is what CG_ConfigStringModified_Hk() is for.
    //
    // The same for the checks against the last index...
    //
    // ...the size of the string offsets, which the client clears and uses to
    // get at the data that follows them...
    //
    // ...and the size of the whole gameState_t, which the client allocates,
    // copies, and clears.
    //
    for (const Instruction& s: std::initializer_list<Instruction>
    {
      {0x1400F4084, 7}, {0x1400FD05A, 6}, {0x1400FD178, 6}, {0x1400FFDB7, 7},
      {0x140100057, 7}, {0x1401A1E10, 6}, {0x1401A2AC0, 6}, {0x140237409, 7},
      {0x140237567, 6}, {0x140238671, 5}, {0x140239A78, 5}
    })
    {
      Utils::Hook::Replace32(s.place, s.length, BASEGAME_MAX_CONFIGSTRINGS, MAX_CONFIGSTRINGS);
    }

    for (const Instruction& s: std::initializer_list<Instruction>
    {
      {0x1400F3F29, 7}, {0x1400FFEFA, 6}, {0x140239026, 7}, {0x1402391F8, 6}
    })
    {
      Utils::Hook::Replace32(s.place, s.length, BASEGAME_MAX_CONFIGSTRINGS - 1, MAX_CONFIGSTRINGS - 1);
    }

    for (const Instruction& s: std::initializer_list<Instruction>
    {
      {0x1400F3FDA, 6}, {0x1400FFD47, 6}, {0x1400F400F, 7}
    })
    {
      Utils::Hook::Replace32(s.place, s.length, ClientOffsetsSize, sizeof(GameState::stringOffsets));
    }

    for (const Instruction& s: std::initializer_list<Instruction>
    {
      {0x1400F3ED5, 5}, {0x1400F3FBE, 6}, {0x1400FC501, 6}
    })
    {
      Utils::Hook::Replace32(s.place, s.length, ClientGameStateSize, sizeof(GameState));
    }

    // The game clears its configstrings as part of clearing server_t, which no
    // longer covers ours, so we clear them along with it.
    //
    Utils::Hook(0x1402386B6, SV_ClearServer, HOOK_CALL).install()->quick();
    Utils::Hook(0x140239AD0, SV_ClearServer, HOOK_CALL).install()->quick();
  }

  void* ConfigStrings::SV_ClearServer(void* dest, int value, size_t size)
  {
    std::fill_n(svConfigStrings, MAX_CONFIGSTRINGS, static_cast<uint16_t>(0));
    return std::memset(dest, value, size);
  }

  unsigned int ConfigStrings::SV_GetConfigString(int index, Game::ConfigString basegameLastPosition, int extendedFirstPosition)
  {
    if (index > basegameLastPosition)
    {
      index = extendedFirstPosition + (index - (basegameLastPosition + 1));
    }

    return Game::SV_GetConfigstringConst(index);
  }

  void ConfigStrings::SV_SetConfigString(int index, const char* data, Game::ConfigString basegameLastPosition, int extendedFirstPosition)
  {
    if (index > basegameLastPosition)
    {
      index = extendedFirstPosition + (index - (basegameLastPosition + 1));
    }

    Game::SV_SetConfigstring(index, data);
  }

  const char* ConfigStrings::CL_GetConfigString(int index, Game::ConfigString basegameLastPosition, int extendedFirstPosition)
  {
    if (index > basegameLastPosition)
    {
      index = extendedFirstPosition + (index - (basegameLastPosition + 1));
    }

    return Game::CL_GetConfigString(index);
  }

  unsigned int ConfigStrings::SV_GetCachedModelConfigStringConst(int index)
  {
    return SV_GetConfigString(index, Game::CS_MODELS_LAST, EXTRA_MODELCACHE_FIRST);
  }

  void ConfigStrings::SV_SetCachedModelConfigString(int index, const char* data)
  {
    SV_SetConfigString(index, data, Game::CS_MODELS_LAST, EXTRA_MODELCACHE_FIRST);
  }

  const char* ConfigStrings::CL_GetCachedModelConfigString(int index)
  {
    return CL_GetConfigString(index, Game::CS_MODELS_LAST, EXTRA_MODELCACHE_FIRST);
  }

  bool ConfigStrings::CG_ParseExtraConfigStrings()
  {
    Command::ClientParams params;

    if (params.size() <= 1)
    {
      return false;
    }

    char* end;
    const char* input (params.get(1));
    int index (std::strtol(input, &end, 10));

    if (input == end)
    {
      Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "{} is not a valid input\nUsage: {} <weapon index>\n", input, params.get(0));
      return false;
    }

    if (index < BASEGAME_MAX_CONFIGSTRINGS)
    {
      return false;
    }

    // Note that the game sets up the weapons whose configstrings come before
    // ours itself, so here we only see the ones past them.
    //
    if (index >= EXTRA_WEAPONS_FIRST && index <= EXTRA_WEAPONS_LAST)
    {
      // Recompute weapon index
      index = index - EXTRA_WEAPONS_FIRST + Weapon::EXTRA_CONFIGSTRING_WEAPONS_FIRST;
      Game::CG_SetupWeaponConfigString(0, index);
      return true;
    }

    if (index >= EXTRA_MODELCACHE_FIRST && index <= EXTRA_MODELCACHE_LAST)
    {
      // If ModelCache left the models where the game keeps them (which it does
      // when we play a demo), then there is nowhere to put the extra ones, so we
      // just drop them.
      //
      if (!ModelCache::modelsHaveBeenReallocated)
      {
        return true;
      }

      const char* name (Game::CL_GetConfigString(index));

      index = index - EXTRA_MODELCACHE_FIRST + ModelCache::BASE_GMODEL_COUNT;
      ModelCache::gameModels_reallocated[index] = Utils::Hook::Call<Game::XModel*(const char*)>(0x140030080)(name); // R_RegisterModel

      return true;
    }

    // There is nothing to do for a rumble, the same as on x86.
    //
    if (index >= RUMBLE_FIRST && index <= RUMBLE_LAST)
    {
      return true;
    }

    return false;
  }

  void ConfigStrings::CG_ConfigStringModified_Hk(int localClientNum)
  {
    if (!CG_ParseExtraConfigStrings())
    {
      Utils::Hook::Call<void(int)>(0x1400E5E10)(localClientNum); // CG_ConfigStringModified
    }
  }

  bool ConfigStrings::IsValidRumbleIndex(int index)
  {
    // Note that RUMBLE_LAST equals MAX_CONFIGSTRINGS because of the off-by-one
    // in EXTRA_MODELCACHE_LAST. The layout is shared with the other end of the
    // connection, so rather than shifting it we just never use the last slot.
    //
    // @@ Fix the layout if we ever change the protocol anyway.
    //
    return index >= 0 && RUMBLE_FIRST + index < MAX_CONFIGSTRINGS;
  }

  const char* ConfigStrings::CL_GetRumbleConfigString(int index)
  {
    if (!IsValidRumbleIndex(index))
    {
      return "";
    }

    return Game::CL_GetConfigString(RUMBLE_FIRST + index);
  }

  unsigned int ConfigStrings::SV_GetRumbleConfigStringConst(int index)
  {
    if (!IsValidRumbleIndex(index))
    {
      return 0;
    }

    return Game::SV_GetConfigstringConst(RUMBLE_FIRST + index);
  }

  void ConfigStrings::SV_SetRumbleConfigString(int index, const char* data)
  {
    if (!IsValidRumbleIndex(index))
    {
      return;
    }

    Game::SV_SetConfigstring(RUMBLE_FIRST + index, data);
  }

  ConfigStrings::ConfigStrings()
  {
    PatchConfigStrings();

    // Note that we hook more calls than on x86, since x64 inlines some of the
    // functions we hook there into their callers.
    //
    for (const uintptr_t place: {0x14019B4AB, 0x14019B6E8, 0x1401AAFD2, 0x1401AB0D2, 0x1401AC036, 0x1401AC0DF})
    {
      Utils::Hook(place, SV_GetCachedModelConfigStringConst, HOOK_CALL).install()->quick();
    }

    Utils::Hook(0x1401AC0C6, SV_GetCachedModelConfigStringConst, HOOK_JUMP).install()->quick();
    Utils::Hook(0x1401AC09A, SV_SetCachedModelConfigString, HOOK_CALL).install()->quick(); // G_ModelIndex

    for (const uintptr_t place: {0x1400D9C8A, 0x1400E9B94, 0x1400E9C2A, 0x1400EA1DA})
    {
      Utils::Hook(place, CL_GetCachedModelConfigString, HOOK_CALL).install()->quick();
    }

    // Handle the extra configstrings before the game gets to them, since it
    // would otherwise just ignore them.
    //
    Utils::Hook(0x1400E65B7, CG_ConfigStringModified_Hk, HOOK_CALL).install()->quick(); // CG_DeployServerCommand
  }
}
