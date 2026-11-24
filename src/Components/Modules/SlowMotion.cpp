#include "SlowMotion.hpp"

namespace Components
{
  int SlowMotion::Delay = 0;

  const Game::dvar_t* SlowMotion::cg_drawDisconnect;

  bool SlowMotion::Com_UpdateSlowMotion(int msec)
  {
    // While we hold the slow motion back, scale the frame the way the game
    // does, only without updating the slow motion.
    //
    if (Delay <= 0)
    {
      return false;
    }

    Delay -= msec;
    return true;
  }

  float SlowMotion::CL_UpdateSlowMotion(float frameTime)
  {
    if (Delay <= 0)
    {
      return Game::Com_UpdateSlowMotion(frameTime);
    }

    const auto timescale(*reinterpret_cast<const float*>(0x141BD7978));
    return std::max(frameTime * timescale, 0.001f);
  }

  void SlowMotion::ScrCmd_SetSlowMotion_Stub(int index, const char* string)
  {
    // On x64 we go in where the game sets the slow motion configstring, which
    // is its last call, so we set it first.
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

    // This one is a bit more involved than on x86, since x64 keeps the slow
    // motion twice, and Com_SetSlowMotion() starts both together.
    //
    // Com_Frame() inlines the update of the one that x86 also has, by the
    // milliseconds that passed. On x86 we skip that call while we hold the
    // slow motion back, so here we jump to the stub where the inlined update
    // starts. The stub saves every volatile register, since presumably the
    // game keeps a few of them live across this point (xmm ones included).
    // It passes the milliseconds from r9d and then either skips the update
    // or goes on with the game's own check, which it has to redo since we
    // replaced it.
    //
    // The other copy, which the client updates by the seconds of each
    // command's frame, we hold back for the same delay (see
    // CL_UpdateSlowMotion()).
    //
    Utils::Hook(0x1401F4253, SlowMotion_Com_UpdateSlowMotion_Stub, HOOK_JUMP).install()->quick();
    Utils::Hook(0x1400F7BB7, CL_UpdateSlowMotion, HOOK_CALL).install()->quick(); // CL_CreateCmd

    // On x86 we hook the two CG_DrawDisconnect() calls in CG_DrawLagometer(),
    // which x64 compiles out along with the lagometer (see
    // Game::CG_DrawLagometer()).
    //
    Game::CG_DrawLagometer_DrawDisconnect = CG_DrawDisconnect_Stub;
  }
}

bool SlowMotion_Com_UpdateSlowMotion(int msec)
{
  return Components::SlowMotion::Com_UpdateSlowMotion(msec);
}
