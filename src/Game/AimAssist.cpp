#include "AimAssist.hpp"

namespace Game
{
  const dvar_t* aim_aimAssistRangeScale;
  const dvar_t* aim_autoAimRangeScale;
  const dvar_t* aim_input_graph_enabled;
  const dvar_t* aim_input_graph_debug;
  const dvar_t* aim_input_graph_index;
  const dvar_t* aim_turnrate_pitch;
  const dvar_t* aim_turnrate_pitch_ads;
  const dvar_t* aim_turnrate_yaw;
  const dvar_t* aim_turnrate_yaw_ads;
  const dvar_t* aim_accel_turnrate_enabled;
  const dvar_t* aim_accel_turnrate_debug;
  const dvar_t* aim_accel_turnrate_lerp;
  const dvar_t* aim_slowdown_enabled;
  const dvar_t* aim_slowdown_debug;
  const dvar_t* aim_slowdown_region_width;
  const dvar_t* aim_slowdown_region_height;
  const dvar_t* aim_slowdown_pitch_scale;
  const dvar_t* aim_slowdown_pitch_scale_ads;
  const dvar_t* aim_slowdown_yaw_scale;
  const dvar_t* aim_slowdown_yaw_scale_ads;
  const dvar_t* aim_autoaim_enabled;
  const dvar_t* aim_autoaim_debug;
  const dvar_t* aim_autoaim_lerp;
  const dvar_t* aim_autoaim_region_width;
  const dvar_t* aim_autoaim_region_height;
  const dvar_t* aim_automelee_enabled;
  const dvar_t* aim_automelee_debug;
  const dvar_t* aim_automelee_lerp;
  const dvar_t* aim_automelee_region_width;
  const dvar_t* aim_automelee_region_height;
  const dvar_t* aim_automelee_range;
  const dvar_t* aim_lockon_enabled;
  const dvar_t* aim_lockon_debug;
  const dvar_t* aim_lockon_deflection;
  const dvar_t* aim_lockon_strength;
  const dvar_t* aim_lockon_region_width;
  const dvar_t* aim_lockon_region_height;
  const dvar_t* aim_scale_view_axis;
  const dvar_t* perk_extendedMeleeRange;

  namespace
  {
    // Weapon states during which auto melee doesn't apply and those during
    // which it tracks its target.
    //
    constexpr int WEAPON_STATE_NO_MELEE_FIRST = 0x1A;
    constexpr int WEAPON_STATE_MELEE_CHARGE = 0xD;
    constexpr int WEAPON_STATE_MELEE_FIRE = 0xF;

    constexpr int BUTTON_MELEE = 0x4;
    constexpr int BUTTON_ADS = 0x800;

    // Turret and vehicle eFlags.
    //
    constexpr int EF_TURRET_ACTIVE = 0xC00;

    constexpr int ENTITY_NONE = 0x7FF;
    constexpr int ET_PLAYER = 1;

    void AimAssist_ClearAutoMeleeTarget(AimAssistGlobals& aa)
    {
      aa.autoMeleeState = AIM_MELEE_STATE_OFF;
      aa.autoMeleeTargetEnt = ENTITY_NONE;
      aa.autoMeleePitch = 0.0f;
      aa.autoMeleePitchTarget = 0.0f;
      aa.autoMeleeYaw = 0.0f;
      aa.autoMeleeYawTarget = 0.0f;
    }

    const AimScreenTarget* AimAssist_GetScreenTargetForEntity(const AimAssistGlobals& aa, int entIndex)
    {
      if (entIndex == ENTITY_NONE)
      {
        return nullptr;
      }

      for (int i = 0; i < aa.screenTargetCount; ++i)
      {
        if (aa.screenTargets[i].entIndex == entIndex)
        {
          return &aa.screenTargets[i];
        }
      }

      return nullptr;
    }

    // Return the first screen target within range and the auto melee region.
    //
    const AimScreenTarget* AimAssist_GetBestMeleeTarget(const AimAssistGlobals& aa, float range)
    {
      const float rangeSqr = range * range;
      const float width = aa.tweakables.autoMeleeRegionWidth;
      const float height = aa.tweakables.autoMeleeRegionHeight;

      for (int i = 0; i < aa.screenTargetCount; ++i)
      {
        const AimScreenTarget& t = aa.screenTargets[i];

        if (t.distSqr > rangeSqr)
        {
          continue;
        }

        const bool inX = width >= t.clipMins[0] && t.clipMaxs[0] >= -width;
        const bool inY = height >= t.clipMins[1] && t.clipMaxs[1] >= -height;

        if (inX && inY)
        {
          return &t;
        }
      }

      return nullptr;
    }

    bool AimAssist_GetTargetPos(const AimAssistGlobals& aa, const AimInput* input, float* pos)
    {
      if (const AimScreenTarget* t = AimAssist_GetScreenTargetForEntity(aa, aa.autoMeleeTargetEnt))
      {
        pos[0] = t->aimPos[0];
        pos[1] = t->aimPos[1];
        pos[2] = t->aimPos[2];
        return true;
      }

      const centity_s* cent = &cg_entitiesArray[aa.autoMeleeTargetEnt].entity;

      if (cent->nextState.eType != ET_PLAYER)
      {
        return false;
      }

      const scr_string_t tag = scr_const->aim_bone;
      const DObj* obj = Com_GetClientDObj(cent->nextState.number, input->localClientNum);

      if (obj == nullptr)
      {
        return false;
      }

      if (!CG_DObjGetWorldTagPos(cent, obj, tag, pos))
      {
        Com_Error(ERR_DROP, "AimAssist_GetTagPos: Cannot find tag [%s] on entity %i.\n", SL_ConvertToString(tag), cent->nextState.number);
      }

      return true;
    }

    // Return the difference between two angles in [-180, 180).
    //
    float AngleDelta(float a)
    {
      const float t = a * 0.0027777778f;
      return (t - std::floor(t + 0.5f)) * 360.0f;
    }
  }

  void AimAssist_UpdateTweakables(int localClientNum)
  {
    AimAssistGlobals& aa = aaGlobArray[localClientNum];
    AimTweakables& t = aa.tweakables;
    const ScreenPlacement* scr = ScrPlace_GetViewPlacement(localClientNum);

    t.slowdownRegionWidth = scr->scaleVirtualToReal[0] * aim_slowdown_region_width->current.value / aa.screenWidth;
    t.slowdownRegionHeight = scr->scaleVirtualToReal[1] * aim_slowdown_region_height->current.value / aa.screenHeight;

    t.autoMeleeRegionWidth = scr->scaleVirtualToReal[0] * aim_automelee_region_width->current.value / aa.screenWidth * aa.fovScaleInv;
    t.autoMeleeRegionHeight = scr->scaleVirtualToReal[1] * aim_automelee_region_height->current.value / aa.screenHeight * aa.fovScaleInv;

    t.lockOnRegionWidth = scr->scaleVirtualToReal[0] * aim_lockon_region_width->current.value / aa.screenWidth;
    t.lockOnRegionHeight = scr->scaleVirtualToReal[1] * aim_lockon_region_height->current.value / aa.screenHeight;
  }

  void AimAssist_UpdateAdsLerp(const AimInput* input)
  {
    AimAssistGlobals& aa = aaGlobArray[input->localClientNum];

    aa.adsLerp = aa.ps.fWeaponPosFrac;

    // Treat a turret as fully aimed down the sight while aiming with it or
    // while its overlay is up.
    //
    if (((aa.ps.eFlags & EF_TURRET_ACTIVE) != 0 && (input->buttons & BUTTON_ADS) != 0) || CG_UsingTurretOverlay(input->localClientNum))
    {
      aa.adsLerp = 1.0f;
    }
  }

  void AimAssist_ApplyAutoMelee(const AimInput* input, AimOutput* output)
  {
    AimAssistGlobals& aa = aaGlobArray[input->localClientNum];

    output->meleeChargeYaw = 0.0f;
    output->meleeChargeDist = 0;

    if (!aim_automelee_enabled->current.enabled || aa.ps.weapIndex == 0 || static_cast<unsigned int>(aa.ps.weaponState - WEAPON_STATE_NO_MELEE_FIRST) <= 1)
    {
      AimAssist_ClearAutoMeleeTarget(aa);
      return;
    }

    const bool pressed = (aa.prevButtons & BUTTON_MELEE) == 0 && (input->buttons & BUTTON_MELEE) != 0;

    if (aa.autoMeleeState != AIM_MELEE_STATE_UPDATING && pressed)
    {
      const float range = aa.ps.isExtendedMelee ? perk_extendedMeleeRange->current.value : aim_automelee_range->current.value;

      const AimScreenTarget* target = AimAssist_GetScreenTargetForEntity(aa, aa.autoMeleeTargetEnt);

      if (target == nullptr)
      {
        target = AimAssist_GetBestMeleeTarget(aa, range);
      }

      if (target == nullptr)
      {
        AimAssist_ClearAutoMeleeTarget(aa);
        return;
      }

      aa.autoMeleeState = AIM_MELEE_STATE_TARGETED;
      aa.autoMeleeTargetEnt = target->entIndex;
      aa.autoMeleePitch = aa.viewAngles[0];
      aa.autoMeleePitchTarget = 0.0f;
      aa.autoMeleeYaw = aa.viewAngles[1];
      aa.autoMeleeYawTarget = 0.0f;

      // Charge toward the target unless prone.
      //
      if ((aa.ps.pm_flags & PMF_PRONE) == 0 && target->distSqr > 0.0f)
      {
        const float dist = std::sqrt(target->distSqr) - 30.0f;

        if (dist > 0.0f)
        {
          output->meleeChargeDist = dist > 255.0f ? static_cast<char>(0xFF) : static_cast<char>(static_cast<int>(dist));

          vec2_t delta = {target->aimPos[0] - aa.eyeOrigin[0], target->aimPos[1] - aa.eyeOrigin[1]};
          output->meleeChargeYaw = vectoryaw(&delta);
        }
      }
    }

    if (aa.autoMeleeState == AIM_MELEE_STATE_TARGETED)
    {
      if (aa.ps.weaponState != WEAPON_STATE_MELEE_CHARGE)
      {
        return;
      }

      aa.autoMeleeState = AIM_MELEE_STATE_UPDATING;
    }
    else if (aa.autoMeleeState != AIM_MELEE_STATE_UPDATING)
    {
      return;
    }

    // Track the target while the melee plays.
    //
    if ((aa.ps.weaponState != WEAPON_STATE_MELEE_CHARGE && aa.ps.weaponState != WEAPON_STATE_MELEE_FIRE) || aa.autoMeleeTargetEnt == ENTITY_NONE)
    {
      AimAssist_ClearAutoMeleeTarget(aa);
      return;
    }

    float pos[3];
    if (!AimAssist_GetTargetPos(aa, input, pos))
    {
      AimAssist_ClearAutoMeleeTarget(aa);
      return;
    }

    const float delta[3] = {pos[0] - aa.eyeOrigin[0], pos[1] - aa.eyeOrigin[1], pos[2] - aa.eyeOrigin[2]};

    float angles[3];
    vectoangles(delta, angles);

    aa.autoMeleePitchTarget = angles[0];
    aa.autoMeleeYawTarget = angles[1];

    if (aa.ps.isThirdPerson)
    {
      aa.autoMeleePitchTarget -= 15.0f;
    }

    const float lerp = aim_automelee_lerp->current.value;
    const float pitch = DiffTrackAngle(aa.autoMeleePitchTarget, aa.autoMeleePitch, lerp, input->deltaTime);
    const float yaw = DiffTrackAngle(aa.autoMeleeYawTarget, aa.autoMeleeYaw, lerp, input->deltaTime);

    const float pitchDelta = pitch - aa.autoMeleePitch;
    const float yawDelta = yaw - aa.autoMeleeYaw;

    aa.autoMeleeYaw = yaw;
    aa.autoMeleePitch = pitch;

    output->pitch = AngleDelta(pitchDelta) + output->pitch;
    output->yaw = AngleDelta(yawDelta) + output->yaw;
  }

  void AimAssist_UpdateMouseInput(const AimInput* input, AimOutput* output)
  {
    AimAssistGlobals& aa = aaGlobArray[input->localClientNum];

    output->pitch = input->pitch;
    output->yaw = input->yaw;

    if (!aa.initialized)
    {
      return;
    }

    AimAssist_UpdateTweakables(input->localClientNum);
    AimAssist_UpdateAdsLerp(input);
    AimAssist_ApplyAutoMelee(input, output);

    aa.prevButtons = input->buttons;
  }
}
