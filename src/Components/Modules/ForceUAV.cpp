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

    for (std::size_t i = 0; i < Game::MAX_CLIENTS; ++i)
    {
      if (static_cast<int>(i) == self)
      {
        continue;
      }

      auto& actor = actors[i];
      if ((actor.flags & Game::COMPASS_ACTOR_ENEMY) == 0 || !IsRecentlySeen(actor.lastUpdate, now))
      {
        continue;
      }

      Utils::Maths::VectorCopy(actor.lastPos, actor.lastEnemyPos);
      actor.beginRadarFadeTime = now;
    }
  }

  void ForceUAV::PingAllVisibleVehicles(const int localClientNum)
  {
    const auto now = Game::CL_GetLocalClientGlobals(localClientNum)->time;

    auto* vehicles = &Game::s_compassVehicles[localClientNum * Game::MAX_COMPASS_VEHICLES];

    for (std::size_t i = 0; i < Game::MAX_COMPASS_VEHICLES; ++i)
    {
      auto& vehicle = vehicles[i];
      if (vehicle.isFriendly || !IsRecentlySeen(vehicle.lastUpdate, now))
      {
        continue;
      }

      Utils::Maths::VectorCopy(vehicle.lastPos, vehicle.lastEnemyPos);
      vehicle.lastEnemyYaw = vehicle.lastYaw;
      vehicle.beginRadarFadeTime = now;
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

    // Note that x64 inlines the owner draw thunks that call these into
    // CG_OwnerDraw(), so we hook the calls there instead. There are two of
    // each, presumably one for the compass and one for the full map.
    //
    Utils::Hook(0x1400DEBC5, CG_CompassDrawEnemies_Stub, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400DF2D8, CG_CompassDrawEnemies_Stub, HOOK_CALL).install()->quick();

    Utils::Hook(0x1400DEC79, CG_CompassDrawVehicles_Stub, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400DF356, CG_CompassDrawVehicles_Stub, HOOK_CALL).install()->quick();
  }
}
