#include "SlowMotion.hpp"

namespace Components
{
  int SlowMotion::Delay = 0;

  const Game::dvar_t* SlowMotion::cg_drawDisconnect;

  float SlowMotion::Com_UpdateSlowMotion(float frameTime)
  {
    // Note that on x86 the game updates the slow motion by the milliseconds
    // that passed. On x64 it does so by the seconds of the frame, which it
    // also returns scaled by the slow motion. So while we hold the slow
    // motion back we scale the frame as the game does without updating it.
    //
    if (Delay <= 0)
    {
      return Game::Com_UpdateSlowMotion(frameTime);
    }

    Delay -= static_cast<int>(frameTime * 1000.0f);

    const auto timescale(*reinterpret_cast<const float*>(0x141BD7978));
    return std::max(frameTime * timescale, 0.001f);
  }

  void SlowMotion::ScrCmd_SetSlowMotion_Stub(int index, const char* string)
  {
    // Note that on x64 we go in where the game sets the slow motion config
    // string as its last call, so we set it first.
    //
    Game::SV_SetConfigstring(index, string);

    auto duration = 1000;
    auto start = Game::Scr_GetFloat(0);
    auto end = 1.0f;

    if (Game::Scr_GetNumParam() >= 2)
    {
      end = Game::Scr_GetFloat(1);
    }

    if (Game::Scr_GetNumParam() >= 3)
    {
      duration = static_cast<int>(Game::Scr_GetFloat(2) * 1000.0f);
    }

    auto delay = 0;

    if (start > end)
    {
      if (duration < 150)
      {
        delay = duration;
      }
      else
      {
        delay = 150;
      }
    }

    duration = duration - delay;

    Game::Com_SetSlowMotion(start, end, duration);
    Delay = delay;

    // Set snapshot num to 1 behind (T6 does this, why shouldn't we?)
    for (auto i = 0; i < *Game::svs_clientCount; ++i)
    {
      Game::svs_clients[i].nextSnapshotTime = *Game::svs_time - 1;
    }
  }

  void SlowMotion::CG_DrawDisconnect_Stub(const int localClientNum)
  {
    if (cg_drawDisconnect->current.enabled)
    {
      Game::CG_DrawDisconnect(localClientNum);
    }
  }

  SlowMotion::SlowMotion()
  {
    cg_drawDisconnect = Game::Dvar_RegisterBool("cg_drawDisconnect", false, Game::DVAR_NONE, "Draw connection interrupted");

    Delay = 0;
    Utils::Hook(0x140178132, ScrCmd_SetSlowMotion_Stub, HOOK_JUMP).install()->quick();
    Utils::Hook(0x1400F7BB7, Com_UpdateSlowMotion, HOOK_CALL).install()->quick(); // CL_CreateCmd

    // Note that on x86 we hook the two calls to CG_DrawDisconnect() in
    // CG_DrawLagometer(), which x64 compiles out along with them (see
    // Game::CG_DrawLagometer()).
    //
    Game::CG_DrawLagometer_DrawDisconnect = CG_DrawDisconnect_Stub;
  }
}
