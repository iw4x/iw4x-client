#pragma once

extern "C"
{
  void SlowMotion_Com_UpdateSlowMotion_Stub();

  void SlowMotion_Com_UpdateSlowMotion(int timePassed);
}

namespace Components
{
  class SlowMotion : public Component
  {
  public:
    SlowMotion();

  private:
    friend void (::SlowMotion_Com_UpdateSlowMotion)(int timePassed);

    static int Delay;

    static const Game::dvar_t* cg_drawDisconnect;

    static void Com_UpdateSlowMotion(int timePassed);

    static void ScrCmd_SetSlowMotion_Stub();

    static void CG_DrawDisconnect_Stub(int localClientNum);
  };
}
