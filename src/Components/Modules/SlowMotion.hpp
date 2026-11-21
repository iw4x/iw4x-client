#pragma once

namespace Components
{
  class SlowMotion : public Component
  {
  public:
    SlowMotion();

  private:
    static int Delay;

    static float Com_UpdateSlowMotion(float frameTime);

    static void ScrCmd_SetSlowMotion_Stub(int index, const char* string);
  };
}
