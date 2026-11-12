#pragma once

namespace Game
{
  // Jump dvars that x64 compiles out, registered by Components::Dvar.
  //
  extern const dvar_t* jump_height;
  extern const dvar_t* jump_stepSize;
  extern const dvar_t* jump_slowdownEnable;
  extern const dvar_t* jump_ladderPushVel;
  extern const dvar_t* jump_spreadAdd;

  // Jump movement reimplemented against the dvars above.
  //
  // Note that x64 folds each dvar into a constant, so we replace whole
  // functions rather than patch the constants: some of them are shared with
  // unrelated code and the enable checks are gone altogether.
  //
  void Jump_ApplySlowdown(playerState_s* ps);
  bool Jump_Check(pmove_s* pm, pml_t* pml);
  void Jump_ClampVelocity(playerState_s* ps, const float* origin);
  bool Jump_GetStepHeight(playerState_s* ps, const float* origin, float* stepSize);
  bool Jump_IsPlayerAboveMax(playerState_s* ps);
  float Jump_GetLandFactor(playerState_s* ps);
  void Jump_Start(pmove_s* pm, pml_t* pml, float height);
}
