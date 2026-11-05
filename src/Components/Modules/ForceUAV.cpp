#include "ForceUAV.hpp"

namespace Components
{
  Dvar::Var ForceUAV::GCompassShowEnemies;

  bool ForceUAV::IsRecentlySeen(const int lastUpdate, const int now)
  {
    return lastUpdate > 0 && lastUpdate <= now && lastUpdate >= now - RECENTLY_SEEN_MS;
  }

  void ForceUAV::PingAllVisibleEnemies(const int localClientNum)
  {
    const auto* cgameGlob = Game::CL_GetLocalClientGlobals(localClientNum);
    const auto now = cgameGlob->time;
    const auto self = cgameGlob->predictedPlayerState.clientNum;

    auto* actors = &Game::s_compassActors[localClientNum * Game::MAX_COMPASS_ACTORS];

    for (auto i = 0; i < static_cast<int>(Game::MAX_CLIENTS); ++i)
    {
      if (i == self)
      {
        continue;
      }

      auto* actor = &actors[i];
      if ((actor->flags & Game::COMPASS_ACTOR_ENEMY) == 0 || !IsRecentlySeen(actor->lastUpdate, now))
      {
        continue;
      }

      std::memcpy(actor->lastEnemyPos, actor->lastPos, sizeof(actor->lastEnemyPos));
      actor->beginRadarFadeTime = now;
    }
  }

  void ForceUAV::PingAllVisibleVehicles(const int localClientNum)
  {
    const auto now = Game::CL_GetLocalClientGlobals(localClientNum)->time;

    auto* vehicles = &Game::s_compassVehicles[localClientNum * Game::MAX_COMPASS_VEHICLES];

    for (auto i = 0; i < Game::MAX_COMPASS_VEHICLES; ++i)
    {
      auto* vehicle = &vehicles[i];
      if (vehicle->isFriendly || !IsRecentlySeen(vehicle->lastUpdate, now))
      {
        continue;
      }

      std::memcpy(vehicle->lastEnemyPos, vehicle->lastPos, sizeof(vehicle->lastEnemyPos));
      vehicle->lastEnemyYaw = vehicle->lastYaw;
      vehicle->beginRadarFadeTime = now;
    }
  }

  void ForceUAV::CG_CompassDrawEnemies_Stub(const int localClientNum, const Game::CompassType compassType, const Game::rectDef_s* parentRect, const Game::rectDef_s* rect, float* color)
  {
    if (localClientNum == 0 && GCompassShowEnemies.get<bool>())
    {
      PingAllVisibleEnemies(localClientNum);
    }

    Game::CG_CompassDrawEnemies(localClientNum, compassType, parentRect, rect, color);
  }

  void ForceUAV::CG_CompassDrawVehicles_Stub(const int localClientNum, const Game::CompassType compassType, const Game::rectDef_s* parentRect, const Game::rectDef_s* rect, float* color)
  {
    if (localClientNum == 0 && GCompassShowEnemies.get<bool>())
    {
      PingAllVisibleVehicles(localClientNum);
    }

    Game::CG_CompassDrawVehicles(localClientNum, compassType, parentRect, rect, color);
  }

  ForceUAV::ForceUAV()
  {
    GCompassShowEnemies = Dvar::Register<bool>("g_compassShowEnemies", false, Game::DVAR_CHEAT, "Whether enemies are visible on the compass at all times");

    Utils::Hook(0x495FF1, CG_CompassDrawEnemies_Stub, HOOK_CALL).install()->quick();
    Utils::Hook(0x496511, CG_CompassDrawEnemies_Stub, HOOK_CALL).install()->quick();

    Utils::Hook(0x49607C, CG_CompassDrawVehicles_Stub, HOOK_CALL).install()->quick();
    Utils::Hook(0x49655A, CG_CompassDrawVehicles_Stub, HOOK_CALL).install()->quick();
  }
}
