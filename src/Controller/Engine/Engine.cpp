#include "Engine.hpp"

#include "../Types.hpp"

namespace Controller
{
  namespace engine
  {
    void aim_assist_begin (const AimInput& input, AimOutput& output) noexcept
    {
      const AimAssistGlobals& aa (aaGlobArray[input.localClientNum]);

      output.pitch = input.pitch;
      output.yaw = input.yaw;

      if (!aa.initialized)
        return;

      AimAssist_UpdateTweakables (input.localClientNum);
      AimAssist_UpdateAdsLerp (&input);
    }

    void aim_assist_end (const AimInput& input, AimOutput& output) noexcept
    {
      AimAssistGlobals& aa (aaGlobArray[input.localClientNum]);

      if (!aa.initialized)
        return;

      AimAssist_ApplyAutoMelee (&input, &output);

      aa.prevButtons = input.buttons;
    }
  }
}
