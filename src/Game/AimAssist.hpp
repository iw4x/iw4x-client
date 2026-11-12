#pragma once

namespace Game
{
  // Aim assist dvars that x64 compiles out, registered by Components::Dvar.
  //
  extern const dvar_t* aim_aimAssistRangeScale;
  extern const dvar_t* aim_autoAimRangeScale;
  extern const dvar_t* aim_input_graph_enabled;
  extern const dvar_t* aim_input_graph_debug;
  extern const dvar_t* aim_input_graph_index;
  extern const dvar_t* aim_turnrate_pitch;
  extern const dvar_t* aim_turnrate_pitch_ads;
  extern const dvar_t* aim_turnrate_yaw;
  extern const dvar_t* aim_turnrate_yaw_ads;
  extern const dvar_t* aim_accel_turnrate_enabled;
  extern const dvar_t* aim_accel_turnrate_debug;
  extern const dvar_t* aim_accel_turnrate_lerp;
  extern const dvar_t* aim_slowdown_enabled;
  extern const dvar_t* aim_slowdown_debug;
  extern const dvar_t* aim_slowdown_region_width;
  extern const dvar_t* aim_slowdown_region_height;
  extern const dvar_t* aim_slowdown_pitch_scale;
  extern const dvar_t* aim_slowdown_pitch_scale_ads;
  extern const dvar_t* aim_slowdown_yaw_scale;
  extern const dvar_t* aim_slowdown_yaw_scale_ads;
  extern const dvar_t* aim_autoaim_enabled;
  extern const dvar_t* aim_autoaim_debug;
  extern const dvar_t* aim_autoaim_lerp;
  extern const dvar_t* aim_autoaim_region_width;
  extern const dvar_t* aim_autoaim_region_height;
  extern const dvar_t* aim_automelee_enabled;
  extern const dvar_t* aim_automelee_debug;
  extern const dvar_t* aim_automelee_lerp;
  extern const dvar_t* aim_automelee_region_width;
  extern const dvar_t* aim_automelee_region_height;
  extern const dvar_t* aim_automelee_range;
  extern const dvar_t* aim_lockon_enabled;
  extern const dvar_t* aim_lockon_debug;
  extern const dvar_t* aim_lockon_deflection;
  extern const dvar_t* aim_lockon_strength;
  extern const dvar_t* aim_lockon_region_width;
  extern const dvar_t* aim_lockon_region_height;
  extern const dvar_t* aim_scale_view_axis;
  extern const dvar_t* perk_extendedMeleeRange;

  // Aim assist reimplemented against the dvars above.
  //
  // Note that x64 inlines the tweakables and ADS lerp updates into
  // AimAssist_UpdateMouseInput() and folds the dvars into constants, some of
  // which unrelated code shares.
  //
  void AimAssist_UpdateTweakables(int localClientNum);
  void AimAssist_UpdateAdsLerp(const AimInput* input);
  void AimAssist_ApplyAutoMelee(const AimInput* input, AimOutput* output);
  void AimAssist_UpdateMouseInput(const AimInput* input, AimOutput* output);
}
