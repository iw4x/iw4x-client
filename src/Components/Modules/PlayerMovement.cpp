#include "PlayerMovement.hpp"
#include "Events.hpp"
#include "GSC/Script.hpp"

constexpr int MASK_PLAYER_CLIP = 0x10000;
constexpr int MASK_BARRIER_CLIP = 0x400;

extern "C"
{
  const Game::dvar_t* PlayerMovement_PlayerDuckedSpeedScale;
  const Game::dvar_t* PlayerMovement_PlayerProneSpeedScale;
  const Game::dvar_t* PlayerMovement_CGNoclipScaler;
  const Game::dvar_t* PlayerMovement_CGUfoScaler;
  const Game::dvar_t* PlayerMovement_PlayerSpectateSpeedScale;
  const Game::dvar_t* PlayerMovement_BGBounces;
  const Game::dvar_t* PlayerMovement_BGBouncesAllAngles;
  const Game::dvar_t* PlayerMovement_BGBunnyHopAuto;
  const Game::dvar_t* PlayerMovement_BGLadderFixedInput;
  const Game::dvar_t* PlayerMovement_BGSprintIgnoreRepress;
  const Game::dvar_t* PlayerMovement_BGOmnimovement;
  const Game::dvar_t* PlayerMovement_BGDive;
  const Game::dvar_t* PlayerMovement_BGOmnimovementDive;
}

namespace Components
{
  const Game::dvar_t* PlayerMovement::BGRocketJump;
  const Game::dvar_t* PlayerMovement::BGRocketJumpScale;
  const Game::dvar_t* PlayerMovement::BGPlayerEjection;
  const Game::dvar_t* PlayerMovement::BGPlayerCollision;
  const Game::dvar_t* PlayerMovement::BGClimbAnything;
  const Game::dvar_t* PlayerMovement::BGDisableLandingSlowdown;
  const Game::dvar_t* PlayerMovement::BGDisableBarrierClips;

  Game::dvar_t** PlayerMovement::player_sprintStrafeSpeedScale = reinterpret_cast<Game::dvar_t**>(0x7ADCEC);

  void PlayerMovement::PM_PlayerTraceStub(Game::pmove_s* pm, Game::trace_t* results, const float* start, const float* end, Game::Bounds* bounds, int passEntityNum, int contentMask)
  {
    Game::PM_playerTrace(pm, results, start, end, bounds, passEntityNum, contentMask);

    if (results && BGClimbAnything->current.enabled)
    {
      results[0].surfaceFlags |= SURF_LADDER;
    }
  }

  // Double bounces
  void PlayerMovement::Jump_ClearState_Hk(Game::playerState_s* ps)
  {
    if (PlayerMovement_BGBounces->current.integer == DOUBLE)
    {
      return;
    }

    Game::Jump_ClearState(ps);
  }

  Game::gentity_s* PlayerMovement::Weapon_RocketLauncher_Fire_Hk(Game::gentity_s* ent, unsigned int weaponIndex,
    float spread, Game::weaponParms* wp, const float* gunVel, Game::lockonFireParms* lockParms, bool magicBullet)
  {
    auto* result = Game::Weapon_RocketLauncher_Fire(ent, weaponIndex, spread, wp, gunVel, lockParms, magicBullet);

    if (ent->client && BGRocketJump->current.enabled && wp->weapDef->inventoryType != Game::WEAPINVENTORY_EXCLUSIVE)
    {
      const auto scale = BGRocketJumpScale->current.value;
      ent->client->ps.velocity[0] += (0.0f - wp->forward[0]) * scale;
      ent->client->ps.velocity[1] += (0.0f - wp->forward[1]) * scale;
      ent->client->ps.velocity[2] += (0.0f - wp->forward[2]) * scale;
    }

    return result;
  }

  int PlayerMovement::StuckInClient_Hk(Game::gentity_s* self)
  {
    if (BGPlayerEjection->current.enabled)
    {
      return Utils::Hook::Call<int(Game::gentity_s*)>(0x402D30)(self); // StuckInClient
    }

    return 0;
  }

  void PlayerMovement::CM_TransformedCapsuleTrace_Hk(Game::trace_t* results, const float* start, const float* end,
    const Game::Bounds* bounds, const Game::Bounds* capsule, int contents, const float* origin, const float* angles)
  {
    if (BGPlayerCollision->current.enabled)
    {
      Utils::Hook::Call<void(Game::trace_t*, const float*, const float*,
        const Game::Bounds*, const Game::Bounds*, int, const float*, const float*)>
        (0x478300)
        (results, start, end, bounds, capsule, contents, origin, angles); // CM_TransformedCapsuleTrace
    }
  }

  void PlayerMovement::PM_CrashLand_Stub(const float* v, float scale, const float* result)
  {
    if (!BGDisableLandingSlowdown->current.enabled)
    {
      Utils::Hook::Call<void(const float*, float, const float*)>(0x4C12B0)(v, scale, result);
    }
  }

  void PlayerMovement::GScr_IsSprinting(const Game::scr_entref_t entref)
  {
    const auto* client = Game::GetEntity(entref)->client;
    if (!client)
    {
      Game::Scr_Error("IsSprinting can only be called on a player");
      return;
    }

    Game::Scr_AddBool(Game::PM_IsSprinting(&client->ps));
  }

  const Game::dvar_t* PlayerMovement::Dvar_RegisterSpectateSpeedScale(const char* dvarName, float value,
    float min, float max, unsigned __int16 /*flags*/, const char* description)
  {
    PlayerMovement_PlayerSpectateSpeedScale = Game::Dvar_RegisterFloat(dvarName, value,
      min, max, Game::DVAR_CODINFO, description);

    return PlayerMovement_PlayerSpectateSpeedScale;
  }

  void PlayerMovement::PmoveSingle_Stub(Game::pmove_s* pm)
  {
    if (BGDisableBarrierClips && BGDisableBarrierClips->current.enabled)
    {
      if (pm != nullptr && (pm->ps->pm_flags & Game::PMF_LADDER) == 0)
      {
        pm->tracemask &= ~MASK_PLAYER_CLIP;
        pm->tracemask |= MASK_BARRIER_CLIP;
      }
    }

    Game::PMoveSingle(pm);
  }

  void PlayerMovement::PM_CheckLadderMove_Stub(Game::pmove_s* pm, Game::pml_t* pml)
  {
    const auto should_fix_ladders = (
      BGDisableBarrierClips && 
      BGDisableBarrierClips->current.enabled && 
      pm != nullptr
    );

    if (should_fix_ladders)
    {
      pm->tracemask |= MASK_PLAYER_CLIP;
    }

    Game::PM_CheckLadderMove(pm, pml);

    if (should_fix_ladders && (pm->ps->pm_flags & Game::PMF_LADDER) == 0)
    {
      pm->tracemask &= ~MASK_PLAYER_CLIP;
    }
  }

  // Replaces the camera-relative right-vector projection inside PM_LadderMove
  // (Vec3ProjectOnPlane at 0x4C3130, single call site at 0x574061) with the
  // ladder-locked form: pml->right = (-Ly, Lx, 0) / |L_xy|.
  float* PlayerMovement::PM_LadderMove_RightVector_Hk(float* source, const float* ladderNormal, float* pmlRight)
  {
    if (PlayerMovement_BGLadderFixedInput && PlayerMovement_BGLadderFixedInput->current.enabled)
    {
      const float lx = ladderNormal[0];
      const float ly = ladderNormal[1];
      const float len2 = lx * lx + ly * ly;
      const float invLen = (len2 > 0.0f) ? (1.0f / std::sqrtf(len2)) : 1.0f;
      pmlRight[0] = -ly * invLen;
      pmlRight[1] =  lx * invLen;
      pmlRight[2] = 0.0f;
      return source; // match the original helper's return-first-arg contract
    }

    return Utils::Hook::Call<float*(float*, const float*, float*)>(0x4C3130)(source, ladderNormal, pmlRight);
  }

  // Omnimovement

  int PlayerMovement::ComputeHorizontalIntent(int forwardSpeed, int rightSpeed)
  {
    if (!PlayerMovement_BGOmnimovement || !PlayerMovement_BGOmnimovement->current.enabled)
      return forwardSpeed;
    return std::max(std::abs(forwardSpeed), std::abs(rightSpeed));
  }

  // Stock PM_WalkMove behavior: rightmove *= player_sprintStrafeSpeedScale while sprinting
  void PlayerMovement::ApplyStockSprintStrafeScale(Game::pmove_s* pm)
  {
    if (!pm || !pm->ps)
    {
      return;
    }

    if ((pm->ps->pm_flags & Game::PMF_SPRINTING) == 0)
    {
      return;
    }

    const auto* dvar = *player_sprintStrafeSpeedScale;
    if (!dvar)
    {
      return;
    }

    float scaled = static_cast<float>(pm->cmd.rightmove) * dvar->current.value;
    pm->cmd.rightmove = static_cast<char>(std::clamp(scaled, -127.0f, 127.0f));
  }

  void PlayerMovement::RegisterMovementDvars()
  {
    PlayerMovement_PlayerDuckedSpeedScale = Game::Dvar_RegisterFloat("player_duckedSpeedScale",
      0.65f, 0.0f, 5.0f, Game::DVAR_CHEAT,
      "The scale applied to the player speed when ducking");

    PlayerMovement_PlayerProneSpeedScale = Game::Dvar_RegisterFloat("player_proneSpeedScale",
      0.15f, 0.0f, 5.0f, Game::DVAR_CHEAT,
      "The scale applied to the player speed when crawling");

    // 3arc naming convention
    PlayerMovement_CGUfoScaler = Game::Dvar_RegisterFloat("cg_ufo_scaler",
      6.0f, 0.001f, 1000.0f, Game::DVAR_CHEAT,
      "The speed at which ufo camera moves");

    PlayerMovement_CGNoclipScaler = Game::Dvar_RegisterFloat("cg_noclip_scaler",
      3.0f, 0.001f, 1000.0f, Game::DVAR_CHEAT,
      "The speed at which noclip camera moves");

    BGDisableLandingSlowdown = Game::Dvar_RegisterBool("bg_disableLandingSlowdown",
      false, Game::DVAR_CODINFO, "Toggle landing slowdown");

    PlayerMovement_BGBunnyHopAuto = Game::Dvar_RegisterBool("bg_bunnyHopAuto",
      false, Game::DVAR_CODINFO, "Constantly jump when holding space");

    BGRocketJump = Game::Dvar_RegisterBool("bg_rocketJump",
      false, Game::DVAR_CODINFO, "Enable CoD4 rocket jumps");

    BGRocketJumpScale = Game::Dvar_RegisterFloat("bg_rocketJumpScale",
      64.0f, 1.0f, std::numeric_limits<float>::max(), Game::DVAR_CODINFO,
      "The scale applied to the pushback force of a rocket");

    BGPlayerEjection = Game::Dvar_RegisterBool("bg_playerEjection",
      true, Game::DVAR_CODINFO, "Push intersecting players away from each other");

    BGPlayerCollision = Game::Dvar_RegisterBool("bg_playerCollision",
      true, Game::DVAR_CODINFO, "Push intersecting players away from each other");

    BGClimbAnything = Game::Dvar_RegisterBool("bg_climbAnything",
      false, Game::DVAR_CODINFO, "Treat any surface as a ladder");

    BGDisableBarrierClips = Game::Dvar_RegisterBool("bg_disableBarrierClips",
      false, Game::DVAR_CODINFO, "Disable player collision with out of bound barriers");

    PlayerMovement_BGLadderFixedInput = Game::Dvar_RegisterBool("bg_ladderFixedInput",
      false, Game::DVAR_SYSTEMINFO, "Make ladder climb and strafe independent of view angle");

    PlayerMovement_BGSprintIgnoreRepress = Game::Dvar_RegisterBool("bg_sprintIgnoreRepress",
      false, Game::DVAR_SYSTEMINFO,
      "Ignore sprint-key re-presses while already sprinting (matches console behaviour)");

    PlayerMovement_BGOmnimovement = Game::Dvar_RegisterBool("bg_omnimovement",
      false, Game::DVAR_CODINFO,
      "Toggle omnidirectional sprint (sprint in any direction)");

    PlayerMovement_BGDive = Game::Dvar_RegisterBool("bg_dive",
      false, Game::DVAR_CODINFO,
      "Toggle dive-to-prone (bypasses specialty_jumpdive perk requirement)");

    PlayerMovement_BGOmnimovementDive = Game::Dvar_RegisterBool("bg_omnimovementDive",
      false, Game::DVAR_CODINFO,
      "Disable the backward-dive attenuation (requires bg_dive or specialty_jumpdive perk)");
  }

  PlayerMovement::PlayerMovement()
  {
    // PlayerMovement.asm has its own copy of this offset.
    //
    AssertOffset(Game::dvar_t, current, 0x10);

    AssertOffset(Game::playerState_s, eFlags, 0xB0);
    AssertOffset(Game::playerState_s, pm_flags, 0xC);
    AssertOffset(Game::pmove_s, cmd, 0x4);
    AssertOffset(Game::usercmd_s, forwardmove, 0x1A);
    AssertOffset(Game::usercmd_s, rightmove, 0x1B);

    Events::OnDvarInit([]
    {
      static const char* bg_bouncesValues[] =
      {
        "disabled",
        "enabled",
        "double",
        nullptr,
      };

      static const char* bg_bouncesAllAnglesValues[] =
      {
        "disabled",
        "simple",
        "all surfaces",
        nullptr,
      };

      PlayerMovement_BGBounces = Game::Dvar_RegisterEnum("bg_bounces", bg_bouncesValues, DISABLED, Game::DVAR_CODINFO, "Bounce glitch settings");
      PlayerMovement_BGBouncesAllAngles = Game::Dvar_RegisterEnum("bg_bouncesAllAngles", bg_bouncesAllAnglesValues, DISABLED, Game::DVAR_CODINFO, "Force bounce from all angles");
    });

    // Hook Dvar_RegisterFloat. Only thing that's changed is that the 0x80 flag is not used
    Utils::Hook(0x448990, Dvar_RegisterSpectateSpeedScale, HOOK_CALL).install()->quick();

    // PM_CmdScaleForStance
    Utils::Hook(0x572D9B, PlayerMovement_PM_PlayerDuckedSpeedScaleStub, HOOK_JUMP).install()->quick();
    Utils::Hook(0x572DA5, PlayerMovement_PM_PlayerProneSpeedScaleStub, HOOK_JUMP).install()->quick();

    // Hook PM_MoveScale so we can add custom speed scale for Ufo and Noclip
    Utils::Hook(0x56F42C, PlayerMovement_PM_MoveScale_Noclip, HOOK_JUMP).install()->quick();
    Utils::Hook(0x56F43F, PlayerMovement_PM_MoveScale_Ufo, HOOK_JUMP).install()->quick();
    Utils::Hook(0x56F452, PlayerMovement_PM_MoveScale_Spectate, HOOK_JUMP).install()->quick();

    // Bounce logic
    Utils::Hook(0x4B1B2D, PlayerMovement_PM_StepSlideMoveStub, HOOK_JUMP).install()->quick();
    Utils::Hook(0x57383E, Jump_ClearState_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x417B66, PlayerMovement_PM_ProjectVelocityStub, HOOK_JUMP).install()->quick();

    // Rocket jump
    Utils::Hook(0x4A4F9B, Weapon_RocketLauncher_Fire_Hk, HOOK_CALL).install()->quick(); //  FireWeapon

    // Hook StuckInClient & CM_TransformedCapsuleTrace
    // so we can prevent intersecting players from being pushed away from each other
    Utils::Hook(0x5D8153, StuckInClient_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x45A5BF, CM_TransformedCapsuleTrace_Hk, HOOK_CALL).install()->quick(); // SV_ClipMoveToEntity
    Utils::Hook(0x5A0CAD, CM_TransformedCapsuleTrace_Hk, HOOK_CALL).install()->quick(); // CG_ClipMoveToEntity

    Utils::Hook(0x573F39, PM_PlayerTraceStub, HOOK_CALL).install()->quick();
    Utils::Hook(0x573E93, PM_PlayerTraceStub, HOOK_CALL).install()->quick();

    Utils::Hook(0x570020, PM_CrashLand_Stub, HOOK_CALL).install()->quick(); // Vec3Scale
    Utils::Hook(0x4E9889, PlayerMovement_Jump_Check_Stub, HOOK_JUMP).install()->quick();

    // Disable player collision with out of bound barriers
    Utils::Hook(0x4CFF5C, PmoveSingle_Stub, HOOK_CALL).install()->quick();          // single PmoveSingle call inside Pmove
    Utils::Hook(0x574AF4, PM_CheckLadderMove_Stub, HOOK_CALL).install()->quick();   // single PM_CheckLadderMove call inside PmoveSingle

    // View-independent ladder controls (opt-in via bg_ladderFixedInput)
    //
    // The pitch stub replaces PM_LadderMove's pitch-scaled climb multiplier
    // (clamp((forward[2] + 0.25) * 2.5, -1, +1)) at 0x573FEF with a constant
    // 1.0 so vertical wishvel becomes 0.5 * cmdScale * forwardmove.
    //
    Utils::Hook(0x573FEF, PlayerMovement_PM_LadderMove_PitchStub, HOOK_JUMP).install()->quick();       // pitch-scaled climb rate block
    Utils::Hook(0x574061, PM_LadderMove_RightVector_Hk, HOOK_CALL).install()->quick();  // camera-relative right-vector projection

    // Console-style sprint hold (opt-in via bg_sprintIgnoreRepress)
    //
    // Disables PM_UpdateSprint's PC-only sprint re-press cancel when enabled.
    // Otherwise keeps the original behavior.
    //
    Utils::Hook(0x56EF59, PlayerMovement_PM_UpdateSprint_RepressCallStub, HOOK_JUMP).install()->quick();

    // Omnimovement - allow sprint bit to be set when pressing back
    //
    // Stock CL_KeyMove skips the sprint-bit update whenever the back kbutton's
    // active flag is set, which is what prevents sprint from starting backward
    // even after our PM_UpdateSprint hooks pass.
    //
    Utils::Hook(0x5A605B, PlayerMovement_CL_KeyMove_SprintBit_Stub, HOOK_JUMP).install()->quick();

    // Omnimovement - sprint gate widening
    //
    // Replace the forwardmove argument of the sprint-start and sprint-end
    // gates with max(|forwardmove|, |rightmove|).
    //
    Utils::Hook(0x56EFBF, PlayerMovement_PM_SprintStartInterferingButtons_Stub, HOOK_CALL).install()->quick();
    Utils::Hook(0x56EF29, PlayerMovement_PM_SprintEndingButtons_Stub, HOOK_CALL).install()->quick();

    // Full-speed lateral sprint
    //
    // Skip the player_sprintStrafeSpeedScale attenuation in PM_WalkMove while
    // sprinting.
    //
    Utils::Hook(0x573262, PlayerMovement_PM_WalkMove_SprintStrafeStub, HOOK_JUMP).install()->quick();

    // Widen the movementDir clamp for remote anim
    //
    // Widen the PM_SetMovementDir clamp from +/-90 to +/-127 at the
    // prone/ladder branch and at the generic branch.
    //
    Utils::Hook(0x443408, PlayerMovement_PM_SetMovementDir_ClampProneLadder_Stub, HOOK_JUMP).install()->quick();
    Utils::Hook(0x4435AA, PlayerMovement_PM_SetMovementDir_ClampGeneric_Stub, HOOK_JUMP).install()->quick();

    // Forward-run anim during sprint
    //
    // Force PM_SetStrafeCondition to "not strafing" while sprinting so the
    // forward-run anim plays.
    //
    Utils::Hook(0x571610, PlayerMovement_PM_SetStrafeCondition_Stub, HOOK_JUMP).install()->quick();

    // Full-speed backward sprint
    //
    // Skip the player_backSpeedScale attenuation on diagonal and on pure
    // backward sprint.
    //
    Utils::Hook(0x573893, PlayerMovement_PM_GetMaxSpeed_BackDiagonal_Stub, HOOK_JUMP).install()->quick();
    Utils::Hook(0x5738A9, PlayerMovement_PM_GetMaxSpeed_BackPure_Stub, HOOK_JUMP).install()->quick();

    // Dive unlock
    //
    // Bypass the specialty_jumpdive perk check at the top of Jump_CheckDive.
    //
    Utils::Hook(0x56D7B3, PlayerMovement_Jump_CheckDive_PerkGate_Stub, HOOK_JUMP).install()->quick();

    // Full-velocity backward dive
    //
    // Skip the 0.3x backward-dive attenuation so backward dives launch at full
    // perk_diveVelocity.
    //
    Utils::Hook(0x56D80E, PlayerMovement_Jump_CheckDive_BackDiveVelocity_Stub, HOOK_JUMP).install()->quick();

    GSC::Script::AddMethod("IsSprinting", GScr_IsSprinting);

    RegisterMovementDvars();
  }
}

int PlayerMovement_ComputeHorizontalIntent(int forwardSpeed, int rightSpeed)
{
  return Components::PlayerMovement::ComputeHorizontalIntent(forwardSpeed, rightSpeed);
}

void PlayerMovement_ApplyStockSprintStrafeScale(Game::pmove_s* pm)
{
  Components::PlayerMovement::ApplyStockSprintStrafeScale(pm);
}
