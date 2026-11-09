#pragma once

extern "C"
{
  int Sound_Init();
  void Sound_Loop();
}

namespace Components
{
  class Sound : public Component
  {
  public:
    Sound();

  private:
    static void __stdcall ReleaseDirectSound(IUnknown** slot);
    static void PatchMiles();

    static void CL_InitRenderer();
  };
}
