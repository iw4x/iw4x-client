#pragma once

extern "C"
{
  void SlowMotion_Com_UpdateSlowMotion_Stub();

  bool SlowMotion_Com_UpdateSlowMotion(int msec);
}

namespace Components
{
  class SlowMotion : public Component
  {
  public:
    SlowMotion();

  private:
    static int Delay;

    static const Game::dvar_t* cg_drawDisconnect;

    friend bool (::SlowMotion_Com_UpdateSlowMotion)(int msec);

    static bool Com_UpdateSlowMotion(int msec);
    static float CL_UpdateSlowMotion(float frameTime);

    static void ScrCmd_SetSlowMotion_Stub(int index, const char* string);

    static void CG_DrawDisconnect_Stub(int localClientNum);
  };
}
