#include "ArenaLength.hpp"

namespace Components
{
  namespace
  {
    constexpr int NEW_ARENA_COUNT = 128;

    constexpr uintptr_t ImageBase = 0x140000000;

    // The game's ui_arenaInfos, and sharedUiInfo with its mapList.
    //
    constexpr uintptr_t ArenaInfos = 0x1465BE5C0;
    constexpr uintptr_t SharedUiInfo = 0x1465CE8C0;
    constexpr uintptr_t MapList = SharedUiInfo + 0x1E14;

    // Return where the mapList data at the game's address a lives now. Note
    // that the game's map name is too short, so whatever reaches it reaches
    // ours instead.
    //
    uintptr_t Moved(uintptr_t a)
    {
      assert(a >= MapList && a < MapList + sizeof(Game::mapInfo));

      size_t field = a - MapList;

      if (field == offsetof(Game::newMapArena_t, oldMapName))
      {
        field = offsetof(Game::newMapArena_t, mapName);
      }

      return reinterpret_cast<uintptr_t>(ArenaLength::NewArenas) + field;
    }

    // Recover the entry from the address of its old map name, which is what
    // the game passes where it reaches the name through an 8-bit
    // displacement that can't reach ours.
    //
    const Game::newMapArena_t* Entry(const char* oldMapName)
    {
      return reinterpret_cast<const Game::newMapArena_t*>(oldMapName - offsetof(Game::newMapArena_t, oldMapName));
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

  Game::newMapArena_t* ArenaLength::NewArenas;
  char** ArenaLength::NewArenaInfos;

  void ArenaLength::CopyMapName(char* dest, const char* src, int)
  {
    Game::I_strncpyz(dest, src, sizeof(Game::newMapArena_t::mapName));
  }

  void ArenaLength::SetMapNameDvar(const char* dvarName, const char* oldMapName)
  {
    Game::Dvar_SetStringByName(dvarName, Entry(oldMapName)->mapName);
  }

  int ArenaLength::CompareMapName(const char* name, const char* oldMapName)
  {
    return Game::I_stricmp(name, Entry(oldMapName)->mapName);
  }

  const char* ArenaLength::FormatMapName(const char* fmt, const char* oldMapName)
  {
    return Game::va(fmt, Entry(oldMapName)->mapName);
  }

  ArenaLength::ArenaLength()
  {
    AssertOffset(Game::newMapArena_t, oldMapName, 0x20);
    AssertOffset(Game::newMapArena_t, other, 0xA70);
    AssertOffset(Game::newMapArena_t, mapName, 0xB00);
    AssertSize(Game::newMapArena_t, 0xB20);
    AssertSize(Game::mapInfo, 0xB00);
    AssertOffset(Game::sharedUiInfo_t, mapList, MapList - SharedUiInfo);

    NewArenas = static_cast<Game::newMapArena_t*>(Utils::Hook::AllocateNear(ImageBase, NEW_ARENA_COUNT * sizeof(Game::newMapArena_t)));
    NewArenaInfos = static_cast<char**>(Utils::Hook::AllocateNear(ImageBase, NEW_ARENA_COUNT * sizeof(char*)));

    // Move ui_arenaInfos, which the game fills from mp/basemaps.arena and
    // UI_LoadArenas reads, and raise the count of arenas it fills.
    //
    for (const Reference& r: std::initializer_list<Reference>
    {
      {0x140269A1B, 7, ArenaInfos}, {0x140269D78, 7, ArenaInfos}
    })
    {
      Utils::Hook::Displace(r.place, r.length, r.target, reinterpret_cast<uintptr_t>(NewArenaInfos));
    }

    Utils::Hook::Replace32(0x140269A32, 5, 64, NEW_ARENA_COUNT);

    // Move sharedUiInfo.mapList, which UI_LoadArenas fills and the map
    // lookups, feeders and StartServer read, from the next instruction ...
    //
    for (const Reference& r: std::initializer_list<Reference>
    {
      {0x140269912, 7, MapList + 0x20}, {0x140269989, 7, MapList}, {0x140269C46, 7, MapList},
      {0x140269D6C, 7, MapList}, {0x140269D84, 7, MapList + 0x20}, {0x140269E1F, 7, MapList + 0x30},
      {0x140269E54, 7, MapList + 0x50}, {0x14026A08B, 7, MapList + 0x20}, {0x14026A096, 7, MapList},
      {0x14026D05A, 7, MapList + 0xA70}, {0x14026D1BE, 7, MapList}, {0x14026D786, 7, MapList},
      {0x14026E15B, 7, MapList + 0xAFC}
    })
    {
      Utils::Hook::Displace(r.place, r.length, r.target, Moved(r.target));
    }

    // ... and from sharedUiInfo.
    //
    for (const Reference& r: std::initializer_list<Reference>
    {
      {0x140269EA5, 8, MapList + 0xA78}, {0x140269F24, 8, MapList + 0xA78}, {0x140269F2F, 8, MapList + 0xA78},
      {0x140269F68, 12, MapList + 0xA78}, {0x140269F7B, 7, MapList}, {0x14026D137, 7, MapList + 0x20},
      {0x14026D17E, 7, MapList + 0x50}
    })
    {
      Utils::Hook::Rebase(r.place, r.length, SharedUiInfo, r.target, Moved(r.target));
    }

    // The lookup of a map's custom value walks the entries by their custom
    // key count and reaches the map name relative to it.
    //
    Utils::Hook::Replace32(0x14026D068, 7,
                           static_cast<uint32_t>(offsetof(Game::newMapArena_t, oldMapName) - offsetof(Game::newMapArena_t, other)),
                           static_cast<uint32_t>(offsetof(Game::newMapArena_t, mapName) - offsetof(Game::newMapArena_t, other)));

    // Grow the entries.
    //
    for (const Instruction& i: std::initializer_list<Instruction>
    {
      {0x140269926, 7}, {0x140269990, 7}, {0x140269C53, 7}, {0x140269C88, 7}, {0x140269DC9, 7},
      {0x140269DF7, 7}, {0x140269E26, 7}, {0x140269E5B, 7}, {0x140269E9E, 7}, {0x140269EB4, 7},
      {0x140269F1D, 7}, {0x140269F61, 7}, {0x140269F85, 7}, {0x14026D0B2, 7}, {0x14026D141, 7},
      {0x14026D1CF, 7}, {0x14026D796, 7}, {0x14026E178, 7}
    })
    {
      Utils::Hook::Replace32(i.place, i.length, sizeof(Game::mapInfo), sizeof(Game::newMapArena_t));
    }

    // Copy the whole map name in UI_LoadArenas rather than the game's 16
    // characters.
    //
    Utils::Hook(0x140269DD6, CopyMapName, HOOK_CALL).install()->quick();

    // Read our map name where the game reaches its own through an 8-bit
    // displacement: setting ui_mapname from the map feeder, the lookups by
    // name, and the map command of StartServer.
    //
    Utils::Hook(0x14026C01D, SetMapNameDvar, HOOK_CALL).install()->quick();
    Utils::Hook(0x14026D1E1, CompareMapName, HOOK_CALL).install()->quick();
    Utils::Hook(0x14026D7A4, CompareMapName, HOOK_CALL).install()->quick();
    Utils::Hook(0x1402703A8, FormatMapName, HOOK_CALL).install()->quick();
  }
}
