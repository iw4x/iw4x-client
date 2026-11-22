#pragma once

namespace Game
{
  // The aim assist dvars that x64 compiles out, which Components::Dvar
  // registers for us.
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

  // Aim assist, reimplemented against the dvars above.
  //
  // Why reimplement it at all? Because x64 inlines the tweakables and the
  // ADS lerp updates into AimAssist_UpdateMouseInput() and folds the dvars
  // into constants, some of which unrelated code shares. So there is nothing
  // left to point at our dvars, and patching the constants would change
  // that other code too. Rewriting the functions seemed like the lesser
  // evil, even if it means we now have to keep them in step with x86.
  //
  void AimAssist_UpdateTweakables(int localClientNum);
  void AimAssist_UpdateAdsLerp(const AimInput* input);
  void AimAssist_ApplyAutoMelee(const AimInput* input, AimOutput* output);
  void AimAssist_UpdateMouseInput(const AimInput* input, AimOutput* output);

  // Draw the targets and regions of the aim assistance that the aim_*_debug
  // dvars debug.
  //
  // Note that x64 compiles out AimAssist_DrawDebugOverlay() along with what
  // it calls, so this is x86's, more or less.
  //
  void AimAssist_DrawDebugOverlay(int localClientNum);
}
