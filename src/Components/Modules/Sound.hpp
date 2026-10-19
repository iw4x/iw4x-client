#pragma once

namespace Components
{
  class Sound : public Component
  {
  public:
    Sound();

  private:
    static int  Init();
    static void Loop();

    static void __stdcall ReleaseDirectSound(IUnknown** slot);
    static void PatchMiles();

    static void CL_InitRenderer();
  };
}
