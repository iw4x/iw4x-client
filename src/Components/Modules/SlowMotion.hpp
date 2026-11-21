#pragma once

namespace Components
{
  class SlowMotion : public Component
  {
  public:
    SlowMotion();

  private:
    static int Delay;

    static const Game::dvar_t* cg_drawDisconnect;

    static float Com_UpdateSlowMotion(float frameTime);

    static void ScrCmd_SetSlowMotion_Stub(int index, const char* string);

    static void CG_DrawDisconnect_Stub(int localClientNum);
  };
}
