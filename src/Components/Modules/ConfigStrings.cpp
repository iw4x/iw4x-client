#include "ConfigStrings.hpp"

namespace Components
{
  namespace
  {
    // The game's gameState_t, grown to our configstrings.
    //
    struct GameState
    {
      int stringOffsets[ConfigStrings::MAX_CONFIGSTRINGS];
      char stringData[0x20000];
      int dataCount;
    };

    constexpr uintptr_t ImageBase = 0x140000000;

    // The client's gameState_t and the clientActive_t it is part of.
    //
    constexpr uintptr_t ClientActive = 0x140C5AC20;
    constexpr uintptr_t ClientGameState = 0x140C9B960;

    constexpr uint32_t ClientOffsetsSize = ConfigStrings::BASEGAME_MAX_CONFIGSTRINGS * sizeof(int);
    constexpr uint32_t ClientGameStateSize = ClientOffsetsSize + sizeof(GameState::stringData) + sizeof(int);

    // The server's configstrings and the server_t they are part of.
    //
    constexpr uintptr_t Server = 0x1464FBD80;
    constexpr uintptr_t ServerConfigStrings = 0x1464FCDA2;

    GameState* clGameState;
    uint16_t* svConfigStrings;

    // Return where the client's gameState_t data at the game's address a
    // lives now.
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

    // Instruction to patch: its address and length.
    //
    struct Instruction
    {
      uintptr_t place;
      size_t length;
    };

    // Instruction that reaches the game's address target.
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
    clGameState = static_cast<GameState*>(Utils::Hook::AllocateNear(ImageBase, sizeof(GameState)));
    svConfigStrings = static_cast<uint16_t*>(Utils::Hook::AllocateNear(ImageBase, MAX_CONFIGSTRINGS * sizeof(uint16_t)));

    // Move the client's gameState_t that CL_ParseGamestate, CL_ClearState,
    // CL_GetConfigString, CL_ConfigstringModified and the demo reach, from
    // the next instruction, ...
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

    // ... from the image base, ...
    //
    for (const Reference& s: std::initializer_list<Reference>
    {
      {0x1400F3F60, 8, 0x140C9B960}, {0x1400F4051, 8, 0x140C9B960},
      {0x1400FD0E1, 8, 0x140C9B960}, {0x1400FD124, 8, 0x140C9B960}
    })
    {
      Utils::Hook::Rebase(s.place, s.length, ImageBase, s.target, Moved(s.target));
    }

    // ... and from the clientActive_t, in CL_ParseGamestate.
    //
    for (const Reference& s: std::initializer_list<Reference>
    {
      {0x1400FFF49, 7, 0x140C9B960}, {0x1400FFFFC, 7, 0x140C9B960}, {0x140100084, 7, 0x140C9B960}
    })
    {
      Utils::Hook::Rebase(s.place, s.length, ClientActive, s.target, Moved(s.target));
    }

    // Move the server's configstrings that SV_SendClientGameState,
    // SV_ClearConfigStrings, SV_GetConfigstring(Const), SV_SetConfigstring
    // and SV_SpawnServer reach, from the next instruction and from the
    // server_t. Note that the empty configstring in the word before them
    // stays.
    //
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

    // SV_SpawnServer fills them up to their end, which is the start of the
    // next global.
    //
    Utils::Hook::Displace(0x140239BFA, 7,
                          ServerConfigStrings + BASEGAME_MAX_CONFIGSTRINGS * sizeof(uint16_t),
                          reinterpret_cast<uintptr_t>(svConfigStrings + MAX_CONFIGSTRINGS));

    // Raise the bounds of the loops over every configstring and of the
    // checks on an index, in the client, the server and the save game. Note
    // that CG_ConfigStringModified keeps its own and so ignores ours.
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

    // Raise the sizes: clearing the string offsets, reaching the data of the
    // copy CL_ConfigstringModified makes, and allocating, copying and
    // clearing the whole gameState_t.
    //
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

    // Clear the server's configstrings along with the server_t, in
    // SV_ClearConfigStrings and SV_SpawnServer.
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

    // Note that the game sets up the weapons whose configstrings are
    // before ours itself.
    //
    if (index >= EXTRA_WEAPONS_FIRST && index <= EXTRA_WEAPONS_LAST)
    {
      // Recompute weapon index
      index = index - EXTRA_WEAPONS_FIRST + Weapon::EXTRA_CONFIGSTRING_WEAPONS_FIRST;
      Game::CG_SetupWeaponConfigString(0, index);
      return true;
    }

    // Note that ModelCache leaves the models where the game keeps them when
    // we play a demo, in which case there is nowhere to keep the extra ones.
    //
    if (index >= EXTRA_MODELCACHE_FIRST && index <= EXTRA_MODELCACHE_LAST)
    {
      if (!ModelCache::modelsHaveBeenReallocated)
      {
        return true;
      }

      const char* name (Game::CL_GetConfigString(index));

      index = index - EXTRA_MODELCACHE_FIRST + ModelCache::BASE_GMODEL_COUNT;
      ModelCache::gameModels_reallocated[index] = Utils::Hook::Call<Game::XModel*(const char*)>(0x140030080)(name); // R_RegisterModel

      return true;
    }

    // The game has nothing to do for a rumble, the same as on x86.
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

    // Remap the configstrings of the models past the game's to the extra
    // ones, wherever the game reads or sets the configstring of a model.
    //
    // Note that x64 inlines some of the functions whose calls we hook on x86
    // into their callers, and so we hook more calls.
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

    // Handle the extra configstrings, which the game ignores, before it
    // gets to them.
    //
    Utils::Hook(0x1400E65B7, CG_ConfigStringModified_Hk, HOOK_CALL).install()->quick(); // CG_ParseServerCommand
  }
}
