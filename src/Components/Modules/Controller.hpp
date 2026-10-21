#pragma once

#include "../../Controller/Runtime.hpp"
#include "../../Controller/Haptic/Effect.hpp"

namespace Components
{
  class Controller : public Component
  {
  public:
    static const int RUMBLE_CONFIGSTRINGS_COUNT = 32;

    Controller();
    ~Controller();

    static ::Controller::runtime* Runtime();

    static bool IsControllerInUse();
    static void OnMouseMove(int dx, int dy);

    // The controller version of a PLATFORM_* localized string while the
    // controller is the active input, or nullptr to use the normal string.
    //
    static const char* PlatformString(const char* key);

    // The material hint text draws a button glyph with, or nullptr for the
    // one named.
    //
    static Game::Material* TextGlyphMaterial(const char* name);

    static void PlayHapticEffect(const ::Controller::haptic::effect& effect);
    static void StopHapticEffect(std::uint32_t tag);

    static void GPad_SetLowRumble(int gamePadIndex, double rumble);
    static void GPad_SetHighRumble(int gamePadIndex, double rumble);
    static void GPad_StopRumbles(int gamePadIndex);
    static void GPad_UpdateFeedbacks();

    static Dvar::Var sv_allowAimAssist;

  private:
    static float lowRumble;
    static float highRumble;

    static void SubmitRumble();
  };
}
