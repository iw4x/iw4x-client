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
  const Game::dvar_t* PlayerMovement_BGDisableLandingSlowdown;
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
  const Game::dvar_t* PlayerMovement::BGDisableBarrierClips;

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
      return Utils::Hook::Call<int(Game::gentity_s*)>(0x140195A00)(self); // StuckInClient
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
        (0x1401EFBB0)
        (results, start, end, bounds, capsule, contents, origin, angles); // CM_TransformedCapsuleTrace
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

  // Replace the camera-relative projection of the right vector in
  // PM_LadderMove() (the one call of ProjectPointOnPlane() there) with one
  // locked to the ladder: pml->right = (-Ly, Lx, 0) / |L_xy|. This way
  // strafing on a ladder no longer depends on where the player looks, which
  // is probably what most people expected it to do in the first place.
  //
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

    return Utils::Hook::Call<float*(float*, const float*, float*)>(0x14027A9B0)(source, ladderNormal, pmlRight);
  }

  // Point the RIP-relative float operand of the instruction at place (length
  // bytes long, with the displacement in its last 4) at the dvar's current
  // value. The game keeps its dvars in its own image, so the displacement
  // always fits. It's a bit of a hack, admittedly, but it saves us a stub
  // for every constant.
  //
  void PlayerMovement::RepointFloatOperand(std::uintptr_t place, std::size_t length, const Game::dvar_t* dvar)
  {
    const auto next = static_cast<std::intptr_t>(place + length);
    const auto displacement = reinterpret_cast<std::intptr_t>(&dvar->current.value) - next;

    assert(displacement == static_cast<std::int32_t>(displacement));

    Utils::Hook::Set<std::int32_t>(place + length - 4, static_cast<std::int32_t>(displacement));
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

    PlayerMovement_BGDisableLandingSlowdown = Game::Dvar_RegisterBool("bg_disableLandingSlowdown",
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
    AssertOffset(Game::pmove_s, cmd, 0x8);
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

    // Register our dvars first, since we point the game's code at some of
    // them below. Note that on x86 we register them last, after the hooks,
    // which would leave us nothing to point at here.
    //
    RegisterMovementDvars();

    // Hook Dvar_RegisterFloat. Only thing that's changed is that the 0x80 flag is not used
    Utils::Hook(0x14008C11A, Dvar_RegisterSpectateSpeedScale, HOOK_CALL).install()->quick();

    // PM_CmdScaleForStance
    //
    // On x86 we hook the loads of the ducked and prone scales. x64 loads them
    // from constants through RIP-relative operands instead (presumably the
    // later revision turned those dvars into constants), so we just point
    // those at our dvars.
    //
    RepointFloatOperand(0x14008F6AA, 8, PlayerMovement_PlayerDuckedSpeedScale);
    RepointFloatOperand(0x14008F6A1, 8, PlayerMovement_PlayerProneSpeedScale);

    // Hook PM_MoveScale so we can add custom speed scale for Ufo and Noclip
    //
    // Note that on x86 we also hook the spectate scale. On x64 the game reads
    // player_spectateSpeedScale through the dvar that our registration above
    // returns, so only the noclip and ufo constants need repointing.
    //
    RepointFloatOperand(0x14008F566, 8, PlayerMovement_CGNoclipScaler);
    RepointFloatOperand(0x14008F586, 8, PlayerMovement_CGUfoScaler);

    // Bounce logic
    Utils::Hook(0x140095F76, PlayerMovement_PM_StepSlideMoveStub, HOOK_JUMP).install()->quick();
    Utils::Hook(0x140091283, Jump_ClearState_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x140091C23, PlayerMovement_PM_ProjectVelocityStub, HOOK_JUMP).install()->quick();

    // Rocket jump
    Utils::Hook(0x140190DFA, Weapon_RocketLauncher_Fire_Hk, HOOK_CALL).install()->quick(); //  FireWeapon

    // Hook StuckInClient & CM_TransformedCapsuleTrace
    // so we can prevent intersecting players from being pushed away from each other
    Utils::Hook(0x140192EDA, StuckInClient_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x1402322C6, CM_TransformedCapsuleTrace_Hk, HOOK_CALL).install()->quick(); // SV_ClipMoveToEntity
    Utils::Hook(0x1400C9D3E, CM_TransformedCapsuleTrace_Hk, HOOK_CALL).install()->quick(); // CG_ClipMoveToEntity

    Utils::Hook(0x14008F2B2, PM_PlayerTraceStub, HOOK_CALL).install()->quick();
    Utils::Hook(0x14008F375, PM_PlayerTraceStub, HOOK_CALL).install()->quick();

    // PM_CrashLand
    //
    // On x86 we hook the Vec3Scale() call that applies the landing slowdown.
    // x64 inlines it, twice even (why twice we are not sure), so we jump over
    // each copy.
    //
    Utils::Hook(0x14008FED6, PlayerMovement_PM_CrashLand_Stub1, HOOK_JUMP).install()->quick();
    Utils::Hook(0x14008FF78, PlayerMovement_PM_CrashLand_Stub2, HOOK_JUMP).install()->quick();

    Utils::Hook(0x140087337, PlayerMovement_Jump_Check_Stub, HOOK_JUMP).install()->quick();

    // Disable player collision with out of bound barriers
    Utils::Hook(0x140094315, PmoveSingle_Stub, HOOK_CALL).install()->quick();          // single PmoveSingle call inside Pmove
    Utils::Hook(0x140094EE3, PM_CheckLadderMove_Stub, HOOK_CALL).install()->quick();   // single PM_CheckLadderMove call inside PmoveSingle

    // View-independent ladder controls (opt-in via bg_ladderFixedInput)
    //
    // The pitch stub replaces PM_LadderMove()'s climb multiplier, which
    // scales with the pitch (clamp((forward[2] + 0.25) * 2.5, -1, +1)), with
    // a constant 1.0, so the vertical wishvel becomes
    // 0.5 * cmdScale * forwardmove whichever way the player looks.
    //
    // Note that x64 keeps the multiplier in xmm7, so we take the place of the
    // Vec3Normalize() call that follows and set it there. A bit roundabout,
    // but there is no call to hook where the multiplier is computed.
    //
    Utils::Hook(0x1400913DA, PlayerMovement_PM_LadderMove_PitchStub, HOOK_CALL).install()->quick();  // pitch-scaled climb rate block
    Utils::Hook(0x1400913FE, PM_LadderMove_RightVector_Hk, HOOK_CALL).install()->quick();    // camera-relative right-vector projection

    // Console-style sprint hold (opt-in via bg_sprintIgnoreRepress)
    //
    // Disables PM_UpdateSprint's PC-only sprint re-press cancel when enabled.
    // Otherwise keeps the original behavior.
    //
    Utils::Hook(0x1400927EA, PlayerMovement_PM_UpdateSprint_RepressCallStub, HOOK_JUMP).install()->quick();

    // Omnimovement - allow sprint bit to be set when pressing back
    //
    // Stock CL_KeyMove skips the sprint-bit update whenever the back kbutton's
    // active flag is set, which is what prevents sprint from starting backward
    // even after our PM_UpdateSprint hooks pass.
    //
    Utils::Hook(0x1400F6F44, PlayerMovement_CL_KeyMove_SprintBit_Stub, HOOK_JUMP).install()->quick();

    // Omnimovement - sprint gate widening
    //
    // Replace the forwardmove argument of the sprint-start and sprint-end
    // gates with max(|forwardmove|, |rightmove|).
    //
    // Note that x64 inlines both gates into PM_UpdateSprint(), so we replace
    // the forwardmove they compare there.
    //
    Utils::Hook(0x140092883, PlayerMovement_PM_SprintStartInterferingButtons_Stub, HOOK_JUMP).install()->quick();
    Utils::Hook(0x14009279E, PlayerMovement_PM_SprintEndingButtons_Stub, HOOK_JUMP).install()->quick();

    // Full-speed lateral sprint
    //
    // Skip the player_sprintStrafeSpeedScale attenuation in PM_WalkMove while
    // sprinting.
    //
    // Note that on x86 we redo the attenuation ourselves when we don't skip
    // it. On x64 the stub can simply let the game carry on, which seems a lot
    // less error-prone.
    //
    Utils::Hook(0x140093D05, PlayerMovement_PM_WalkMove_SprintStrafeStub, HOOK_JUMP).install()->quick();

    // Widen the movementDir clamp for remote anim
    //
    // Widen the PM_SetMovementDir clamp from +/-90 to +/-127 at the
    // prone/ladder branch and at the generic branch.
    //
    // Note that x64 shares a single clamp between the two branches, so one
    // stub does for both.
    //
    Utils::Hook(0x140091E16, PlayerMovement_PM_SetMovementDir_ClampStub, HOOK_JUMP).install()->quick();

    // Forward-run anim during sprint
    //
    // Force PM_SetStrafeCondition to "not strafing" while sprinting so the
    // forward-run anim plays.
    //
    // Note that x64 inlines PM_SetStrafeCondition(), so we take the place of
    // its BG_SetConditionValue() call.
    //
    Utils::Hook(0x14009053B, PlayerMovement_PM_SetStrafeCondition_Stub, HOOK_CALL).install()->quick();

    // Full-speed backward sprint
    //
    // Skip the player_backSpeedScale attenuation on diagonal and on pure
    // backward sprint.
    //
    // Note that x64 inlines PM_GetMaxSpeed() and keeps whether the player is
    // sprinting in ebp, which the stubs go by.
    //
    Utils::Hook(0x14009060D, PlayerMovement_PM_GetMaxSpeed_BackDiagonal_Stub, HOOK_JUMP).install()->quick();
    Utils::Hook(0x14009062B, PlayerMovement_PM_GetMaxSpeed_BackPure_Stub, HOOK_JUMP).install()->quick();

    // Dive unlock
    //
    // Bypass the specialty_jumpdive perk check at the top of Jump_CheckDive.
    //
    // Note that x64 inlines Jump_CheckDive() into Jump_Check().
    //
    Utils::Hook(0x14008773A, PlayerMovement_Jump_CheckDive_PerkGate_Stub, HOOK_JUMP).install()->quick();

    // Full-velocity backward dive
    //
    // Skip the 0.3x backward-dive attenuation so backward dives launch at full
    // perk_diveVelocity.
    //
    Utils::Hook(0x140087788, PlayerMovement_Jump_CheckDive_BackDiveVelocity_Stub, HOOK_JUMP).install()->quick();

    GSC::Script::AddMethod("IsSprinting", GScr_IsSprinting);
  }
}
