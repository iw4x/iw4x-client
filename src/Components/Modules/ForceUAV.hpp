#pragma once

namespace Components
{
  class ForceUAV : public Component
  {
  public:
    ForceUAV();

    static Dvar::Var GCompassShowEnemies;

  private:
    static constexpr auto RECENTLY_SEEN_MS = 500;

    static bool IsRecentlySeen(int lastUpdate, int now);

    static void PingAllVisibleEnemies(int localClientNum);
    static void PingAllVisibleVehicles(int localClientNum);

    static void CG_CompassDrawEnemies_Stub(int localClientNum, Game::CompassType compassType, const Game::rectDef_s* parentRect, const Game::rectDef_s* rect, float* color);
    static void CG_CompassDrawVehicles_Stub(int localClientNum, Game::CompassType compassType, const Game::rectDef_s* parentRect, const Game::rectDef_s* rect, float* color);
  };
}
