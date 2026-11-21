#include "SlowMotion.hpp"

namespace Components
{
  int SlowMotion::Delay = 0;

  const Game::dvar_t* SlowMotion::cg_drawDisconnect;

  bool SlowMotion::Com_UpdateSlowMotion(int msec)
  {
    if (Delay <= 0)
    {
      return false;
    }

    Delay -= msec;
    return true;
  }

  float SlowMotion::CL_UpdateSlowMotion(float frameTime)
  {
    // While we hold the slow motion back, scale the frame as the game does
    // without updating the slow motion.
    //
    if (Delay <= 0)
    {
      return Game::Com_UpdateSlowMotion(frameTime);
    }

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

    // Note that x64 keeps the slow motion twice, which Com_SetSlowMotion()
    // starts together. Com_Frame() inlines the update of the one x86 has,
    // by the milliseconds that passed, which we skip as x86 does the call
    // (see SlowMotion.asm for the registers there). The other, which the
    // client updates by the seconds of each command's frame, we hold back for
    // the same delay.
    //
    Utils::Hook(0x1401F4253, SlowMotion_Com_UpdateSlowMotion_Stub, HOOK_JUMP).install()->quick();
    Utils::Hook(0x1400F7BB7, CL_UpdateSlowMotion, HOOK_CALL).install()->quick(); // CL_CreateCmd

    // Note that on x86 we hook the two calls to CG_DrawDisconnect() in
    // CG_DrawLagometer(), which x64 compiles out along with them (see
    // Game::CG_DrawLagometer()).
    //
    Game::CG_DrawLagometer_DrawDisconnect = CG_DrawDisconnect_Stub;
  }
}

bool SlowMotion_Com_UpdateSlowMotion(int msec)
{
  return Components::SlowMotion::Com_UpdateSlowMotion(msec);
}
