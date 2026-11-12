#include "PlayerMovement.hpp"

namespace Game
{
  Jump_ClearState_t Jump_ClearState = Jump_ClearState_t(0x1400875E0);
  PM_playerTrace_t PM_playerTrace = PM_playerTrace_t(0x140094000);
  PM_Trace_t PM_Trace = PM_Trace_t(0x140094110);
  PM_GetEffectiveStance_t PM_GetEffectiveStance = PM_GetEffectiveStance_t(0x140090C20);
  PM_UpdateLean_t PM_UpdateLean = PM_UpdateLean_t(0x140092160);
  PM_IsSprinting_t PM_IsSprinting = PM_IsSprinting_t(0x140091320);
  PM_GroundSurfaceType_t PM_GroundSurfaceType = PM_GroundSurfaceType_t(0x140090D60);
  BG_AddPredictableEventToPlayerstate_t BG_AddPredictableEventToPlayerstate = BG_AddPredictableEventToPlayerstate_t(0x140089050);
  BG_AnimUpdatePlayerStateConditions_t BG_AnimUpdatePlayerStateConditions = BG_AnimUpdatePlayerStateConditions_t(0x140083450);
  BG_AnimScriptEvent_t BG_AnimScriptEvent = BG_AnimScriptEvent_t(0x140083390);
}
