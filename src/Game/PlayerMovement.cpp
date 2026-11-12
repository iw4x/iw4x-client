#include "PlayerMovement.hpp"

namespace Game
{
  Jump_ClearState_t Jump_ClearState = Jump_ClearState_t(0x1400875E0);
  PM_playerTrace_t PM_playerTrace = PM_playerTrace_t(0x140094000);
  PM_Trace_t PM_Trace = PM_Trace_t(0x140094110);
  PM_GetEffectiveStance_t PM_GetEffectiveStance = PM_GetEffectiveStance_t(0x140090C20);
  PM_UpdateLean_t PM_UpdateLean = PM_UpdateLean_t(0x140092160);
  PM_IsSprinting_t PM_IsSprinting = PM_IsSprinting_t(0x140091320);
}
