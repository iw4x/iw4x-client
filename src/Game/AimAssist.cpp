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

  namespace
  {
    constexpr float colorWhite[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    constexpr float colorYellow[4] = {1.0f, 1.0f, 0.0f, 1.0f};

    // Note that x64 compiles out CL_DrawText() along with its callers here.
    //
    void CL_DrawText(const ScreenPlacement* scrPlace, const char* text, const int maxChars, Font_s* font, float x, float y, const int horzAlign, const int vertAlign, float xScale, float yScale, const float* color, const int style)
    {
      ScrPlace_ApplyRect(scrPlace, &x, &y, &xScale, &yScale, horzAlign, vertAlign);
      R_AddCmdDrawText(text, maxChars, font, x, y, xScale, yScale, 0.0f, color, style);
    }

    // Return how far up the crosshair sits, as a fraction of half the
    // screen height.
    //
    float CG_GetCrosshairVerticalOffset(const cg_s* cgameGlob)
    {
      return Utils::Hook::Call<float(const cg_s*)>(0x1400B9150)(cgameGlob);
    }

    // Draw the box of each target within the range of the assistance being
    // debugged, with its rank, entity, distance, speed, and distance from
    // the crosshair above it.
    //
    void AimAssist_DrawTargets(const int localClientNum, const cg_s* cgameGlob, const float* color)
    {
      const auto& aa (aaGlobArray[localClientNum]);

      if (!aa.ps.weapIndex)
      {
        return;
      }

      const auto adsLerp (aa.adsLerp);
      const auto* weapDef (BG_GetWeaponDef(aa.ps.weapIndex));
      auto aimAssistRange (static_cast<float>((1.0 - adsLerp) * weapDef->aimAssistRange + static_cast<double>(weapDef->aimAssistRangeAds) * adsLerp));
      aimAssistRange = aimAssistRange * aim_aimAssistRangeScale->current.value;

      const auto autoAimRange (BG_GetWeaponDef(aa.ps.weapIndex)->autoAimRange * aim_autoAimRangeScale->current.value);

      auto meleeRange (aim_automelee_range->current.value);
      if (cgameGlob->predictedPlayerState.perks[0] & 8)
      {
        meleeRange = perk_extendedMeleeRange->current.value;
      }

      const auto* scrPlace (ScrPlace_GetActivePlacement(localClientNum));

      // Whether the target is out of the range while we debug that
      // assistance.
      //
      const auto outOf ([] (const dvar_t* debug, const float distSqr, const float range)
      {
        return debug->current.enabled && static_cast<float>(range * range) < distSqr;
      });

      for (auto i = 0; i < aa.screenTargetCount; ++i)
      {
        const auto& t (aa.screenTargets[i]);

        if (outOf(aim_autoaim_debug, t.distSqr, autoAimRange)       ||
            outOf(aim_automelee_debug, t.distSqr, meleeRange)       ||
            outOf(aim_slowdown_debug, t.distSqr, aimAssistRange)    ||
            outOf(aim_lockon_debug, t.distSqr, aimAssistRange))
        {
          continue;
        }

        const auto halfWidth (aa.screenWidth * 0.5f);
        const auto halfHeight (aa.screenHeight * 0.5f);

        const auto x (static_cast<float>((t.clipMins[0] + 1.0f) * halfWidth));
        auto y (static_cast<float>((t.clipMins[1] + 1.0f) * halfHeight));
        const auto maxX (static_cast<float>((t.clipMaxs[0] + 1.0f) * halfWidth));
        const auto maxY (static_cast<float>((1.0f + t.clipMaxs[1]) * halfHeight));
        const auto h (maxY - y);

        // Raise the box with the crosshair.
        //
        y = static_cast<float>(y - static_cast<double>(CG_GetCrosshairVerticalOffset(cgameGlob)) * aa.screenHeight * 0.5);

        CL_DrawStretchPicPhysical(x, y, maxX - x, h, 0.0f, 0.0f, 1.0f, 1.0f, color, cgMedia->whiteMaterial);

        // Print the details, one line per 10 units up.
        //
        const auto print ([scrPlace, x, &y] (const char* text, const float* textColor)
        {
          CL_DrawText(scrPlace, text, std::numeric_limits<int>::max(), cgMedia->bigDevFont, x, y, 5, 5, 1.0f, 1.0f, textColor, 0);
          y = static_cast<float>(y - 10.0);
        });

        print(Utils::String::VA("Pri: %i", i), colorYellow);
        print(Utils::String::VA("Ent: %i", t.entIndex), colorYellow);
        print(Utils::String::VA("Dist: %.2f", std::sqrt(t.distSqr)), colorWhite);

        const auto speedSqr (static_cast<float>(static_cast<double>(t.velocity[0]) * t.velocity[0] + static_cast<double>(t.velocity[1]) * t.velocity[1] + static_cast<double>(t.velocity[2]) * t.velocity[2]));
        print(Utils::String::VA("Speed: %.2f", std::sqrt(speedSqr)), colorWhite);
        print(Utils::String::VA("XHairDist: %.4f", std::sqrt(t.crosshairDistSqr)), colorWhite);
      }
    }

    // Draw a box of the region's size in the middle of the screen, raised
    // with the crosshair.
    //
    void AimAssist_DrawCenterBox(const AimAssistGlobals& aa, const float verticalOffset, const float regionWidth, const float regionHeight, const float* color)
    {
      const auto w (static_cast<float>(regionWidth * aa.screenWidth));
      const auto h (static_cast<float>(aa.screenHeight * regionHeight));
      auto y (static_cast<float>((aa.screenHeight - h) * 0.5));
      y = static_cast<float>(y - static_cast<double>(aa.screenHeight) * verticalOffset * 0.5);
      const auto x (static_cast<float>((aa.screenWidth - w) * 0.5));

      CL_DrawStretchPicPhysical(x, y, w, h, 0.0f, 0.0f, 1.0f, 1.0f, color, cgMedia->whiteMaterial);
    }
  }

  void AimAssist_DrawDebugOverlay(const int localClientNum)
  {
    const float colorTargets[4] = {1.0f, 0.0f, 0.0f, 0.25f};
    const float colorRegion[4] = {0.0f, 1.0f, 0.0f, 0.25f};

    const auto& aa (aaGlobArray[localClientNum]);

    if (!aa.initialized)
    {
      return;
    }

    const auto* cg (&cgArray[localClientNum]);
    const auto verticalOffset (CG_GetCrosshairVerticalOffset(cg));

    // Note that x64 drops the auto aim region from AimTweakables along with
    // the auto aim, so we work it out as x86's AimAssist_UpdateTweakables()
    // does.
    //
    const auto* scr (ScrPlace_GetViewPlacement(localClientNum));
    const auto autoAimRegionWidth (static_cast<float>(static_cast<float>(scr->scaleVirtualToReal[0] * aim_autoaim_region_width->current.value) / aa.screenWidth) * aa.fovScaleInv);
    const auto autoAimRegionHeight (static_cast<float>(static_cast<float>(scr->scaleVirtualToReal[1] * aim_autoaim_region_height->current.value) / aa.screenHeight) * aa.fovScaleInv);

    const struct
    {
      const dvar_t* debug;
      float width;
      float height;
    } regions[] =
    {
      {aim_slowdown_debug, aa.tweakables.slowdownRegionWidth, aa.tweakables.slowdownRegionHeight},
      {aim_autoaim_debug, autoAimRegionWidth, autoAimRegionHeight},
      {aim_automelee_debug, aa.tweakables.autoMeleeRegionWidth, aa.tweakables.autoMeleeRegionHeight},
      {aim_lockon_debug, aa.tweakables.lockOnRegionWidth, aa.tweakables.lockOnRegionHeight},
    };

    for (const auto& region : regions)
    {
      if (region.debug->current.enabled)
      {
        AimAssist_DrawTargets(localClientNum, cg, colorTargets);
        AimAssist_DrawCenterBox(aa, verticalOffset, region.width, region.height, colorRegion);
      }
    }
  }
}
