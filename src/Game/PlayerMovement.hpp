#pragma once

namespace Game
{
  typedef void(*Jump_ClearState_t)(playerState_s* ps);
  extern Jump_ClearState_t Jump_ClearState;

  typedef void(*PM_playerTrace_t)(pmove_s* pm, trace_t* results, const float* start, const float* end, const Bounds* bounds, int passEntityNum, int contentMask);
  extern PM_playerTrace_t PM_playerTrace;

  typedef void(*PM_Trace_t)(pmove_s* pm, trace_t* results, const float* start, const float* end, const Bounds* bounds, int passEntityNum, int contentMask);
  extern PM_Trace_t PM_Trace;

  typedef EffectiveStance(*PM_GetEffectiveStance_t)(const playerState_s* ps);
  extern PM_GetEffectiveStance_t PM_GetEffectiveStance;

  typedef void(*PM_UpdateLean_t)(playerState_s* ps, float msec, usercmd_s* cmd, void(*capsuleTrace)(trace_t*, const float*, const float*, const Bounds*, int, int));
  extern PM_UpdateLean_t PM_UpdateLean;

  typedef bool(*PM_IsSprinting_t)(const playerState_s* ps);
  extern PM_IsSprinting_t PM_IsSprinting;

  typedef int(*PM_GroundSurfaceType_t)(pml_t* pml);
  extern PM_GroundSurfaceType_t PM_GroundSurfaceType;

  typedef void(*BG_AddPredictableEventToPlayerstate_t)(int newEvent, unsigned int eventParm, playerState_s* ps);
  extern BG_AddPredictableEventToPlayerstate_t BG_AddPredictableEventToPlayerstate;

  typedef void(*BG_AnimUpdatePlayerStateConditions_t)(pmove_s* pm);
  extern BG_AnimUpdatePlayerStateConditions_t BG_AnimUpdatePlayerStateConditions;

  typedef int(*BG_AnimScriptEvent_t)(playerState_s* ps, int event, int isContinue, int force, int* randSeed);
  extern BG_AnimScriptEvent_t BG_AnimScriptEvent;
}
