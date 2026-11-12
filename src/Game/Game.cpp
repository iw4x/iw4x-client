#include "Game.hpp"

namespace Game
{
  G_LogPrintf_t G_LogPrintf = G_LogPrintf_t(0x14019D7C0);
  G_GetWeaponIndexForName_t G_GetWeaponIndexForName = G_GetWeaponIndexForName_t(0x1401884B0);
  G_SpawnEntitiesFromString_t G_SpawnEntitiesFromString = G_SpawnEntitiesFromString_t(0x1401A91C0);
  G_Spawn_t G_Spawn = G_Spawn_t(0x1401AC420);
  G_GivePlayerWeapon_t G_GivePlayerWeapon = G_GivePlayerWeapon_t(0x1401884D0);
  G_FreeEntity_t G_FreeEntity = G_FreeEntity_t(0x1401AB650);
  G_SpawnItem_t G_SpawnItem = G_SpawnItem_t(0x14016D530);
  G_TempEntity_t G_TempEntity = G_TempEntity_t(0x1401AC800);
  G_AddEvent_t G_AddEvent = G_AddEvent_t(0x1401AA250);
  G_GetItemClassname_t G_GetItemClassname = G_GetItemClassname_t(0x14016C2E0);

  G_LocalizedStringIndex_t G_LocalizedStringIndex = G_LocalizedStringIndex_t(0x1401ABD60);

  G_PointContents_t G_PointContents = G_PointContents_t(0x140232AA0);
  G_TraceCapsule_t G_TraceCapsule = G_TraceCapsule_t(0x14019F540);
  G_RadiusDamage_t G_RadiusDamage = G_RadiusDamage_t(0x14019BB90);

  gentity_s* g_entities = reinterpret_cast<gentity_s*>(0x1418679C0);
  bool* g_quitRequested = reinterpret_cast<bool*>(0x146787378);

  Bounds* bounds_origin = reinterpret_cast<Bounds*>(0x1403A3190);

  char(*g_cmdlineCopy)[1024] = reinterpret_cast<char(*)[1024]>(0x141BD7450);

  NetField* clientStateFields = reinterpret_cast<Game::NetField*>(0x140394940);
  int clientStateFieldsCount = Utils::Hook::Get<int>(0x140393CD8);

  WinVars_t* g_wv = reinterpret_cast<WinVars_t*>(0x14678C000);
  WinConData* s_wcd = reinterpret_cast<WinConData*>(0x14678B730);

  char* cinematicCurrentName = reinterpret_cast<char*>(0x1493C7804);
  int* cinematicPendingStart = reinterpret_cast<int*>(0x1493C7800);
  CinematicHunk* cinematicHunk = reinterpret_cast<CinematicHunk*>(0x1493C7B98);
  void** cinematicMemory = reinterpret_cast<void**>(0x1493C7DA0);

  const char* origErrorMsg = reinterpret_cast<const char*>(0x140422B48);

  XModel* G_GetModel(const int index)
  {
    assert(index > 0);
    assert(index < MAX_MODELS);
    return cached_models[index];
  }
}
