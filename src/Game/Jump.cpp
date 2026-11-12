#include "Jump.hpp"

namespace Game
{
  const dvar_t* jump_height;
  const dvar_t* jump_stepSize;
  const dvar_t* jump_slowdownEnable;
  const dvar_t* jump_ladderPushVel;
  const dvar_t* jump_spreadAdd;

  namespace
  {
    // Predictable event a jump plays, parameterized with the surface type.
    //
    constexpr int EV_JUMP = 0x6F;

    // Animation script events for jumping forward and backward.
    //
    constexpr int ANIM_ET_JUMP = 3;
    constexpr int ANIM_ET_JUMPBK = 4;

    // Return the factor by which a landing that is still recovering slows
    // down the next jump.
    //
    float LandFactor(const playerState_s* ps)
    {
      if (!jump_slowdownEnable->current.enabled || (ps->pm_flags & PMF_DIVING) != 0)
      {
        return 1.0f;
      }

      if (ps->pm_time >= 1700)
      {
        return 2.5f;
      }

      return static_cast<float>(ps->pm_time) * 1.5f * 0.00058823527f + 1.0f;
    }

    void Jump_AddSurfaceEvent(playerState_s* ps, pml_t* pml)
    {
      if ((ps->pm_flags & PMF_LADDER) != 0)
      {
        BG_AddPredictableEventToPlayerstate(EV_JUMP, SURF_TYPE_WOOD, ps);
        return;
      }

      const int surface = PM_GroundSurfaceType(pml);

      if (surface != 0)
      {
        BG_AddPredictableEventToPlayerstate(EV_JUMP, surface, ps);
      }
    }

    void Jump_PushOffLadder(playerState_s* ps, pml_t* pml)
    {
      ps->velocity[2] *= 0.75f;

      float dir[3] = {pml->forward[0], pml->forward[1], 0.0f};
      Vec3Normalize(dir);

      // Push off the ladder face rather than into it.
      //
      const float* ladder = ps->vLadderVec;
      const float dot = ladder[1] * pml->forward[1] + ladder[0] * pml->forward[0] + ladder[2] * pml->forward[2];

      if (0.0f > dot)
      {
        const float d = (ladder[1] * dir[1] + ladder[0] * dir[0] + ladder[2] * dir[2]) * -2.0f;

        dir[0] = ladder[0] * d + dir[0];
        dir[1] = ladder[1] * d + dir[1];
        dir[2] = ladder[2] * d + dir[2];

        Vec3Normalize(dir);
      }

      ps->pm_flags &= ~PMF_LADDER;

      const float push = jump_ladderPushVel->current.value;
      ps->velocity[0] = dir[0] * push;
      ps->velocity[1] = dir[1] * push;
    }

    // Start a dive if the player has the perk, moves in some direction, and
    // there is room for it. Return true if the dive started.
    //
    bool Jump_StartDive(pmove_s* pm, pml_t* pml)
    {
      playerState_s* ps = pm->ps;

      if ((ps->perks[0] & (1 << PERK_JUMPDIVE)) == 0)
      {
        return false;
      }

      if ((ps->pm_flags & PMF_JUMPING) != 0 && ps->pm_time >= 800)
      {
        return false;
      }

      float forward = static_cast<float>(pm->cmd.forwardmove) / 127.0f;
      const float right = static_cast<float>(pm->cmd.rightmove) / 127.0f;

      if (0.0f > forward)
      {
        forward *= 0.3f;
      }

      const float dir[3] =
      {
        right * pml->right[0] + forward * pml->forward[0],
        right * pml->right[1] + forward * pml->forward[1],
        0.0f
      };

      float normal[3];
      if (0.1f > Vec3NormalizeTo(dir, normal))
      {
        return false;
      }

      const float distance = (*perk_diveDistanceCheck)->current.value;
      const float end[3] =
      {
        normal[0] * distance + ps->origin[0],
        normal[1] * distance + ps->origin[1],
        normal[2] * distance + ps->origin[2]
      };

      const Bounds bounds = {{0.0f, 0.0f, 44.0f}, {15.0f, 15.0f, 26.0f}};

      trace_t trace;
      PM_Trace(pm, &trace, ps->origin, end, &bounds, ps->clientNum, pm->tracemask);

      if (1.0f > trace.fraction)
      {
        return false;
      }

      const float velocity = (*perk_diveVelocity)->current.value;
      ps->velocity[0] = dir[0] * velocity;
      ps->velocity[1] = dir[1] * velocity;

      return true;
    }
  }

  void Jump_ApplySlowdown(playerState_s* ps)
  {
    float scale = 1.0f;

    if (ps->pm_time > 1800)
    {
      Jump_ClearState(ps);
      scale = 0.65f;
    }
    else if (ps->pm_time == 0)
    {
      if (ps->jumpOriginZ + 18.0f > ps->origin[2])
      {
        ps->pm_time = 1800;
        scale = 0.65f;
      }
      else
      {
        ps->pm_time = 1200;
        scale = 0.5f;
      }
    }

    if ((ps->pm_flags & PMF_DIVING) == 0 && jump_slowdownEnable->current.enabled)
    {
      ps->velocity[0] *= scale;
      ps->velocity[1] *= scale;
      ps->velocity[2] *= scale;
    }
  }

  bool Jump_Check(pmove_s* pm, pml_t* pml)
  {
    playerState_s* ps = pm->ps;

    if ((ps->pm_flags & PMF_NO_JUMP) != 0)
    {
      return false;
    }

    if (pm->cmd.serverTime - ps->jumpTime < 500)
    {
      return false;
    }

    if ((ps->pm_flags & (PMF_RESPAWNED | PMF_MANTLE)) != 0 || ps->pm_type >= PM_DEAD)
    {
      return false;
    }

    if (PM_GetEffectiveStance(ps) != PM_EFF_STANCE_DEFAULT || (pm->cmd.buttons & CMD_BUTTON_UP) == 0)
    {
      return false;
    }

    // Require the jump button to be released before the next jump.
    //
    if ((pm->oldcmd.buttons & CMD_BUTTON_UP) != 0)
    {
      pm->cmd.buttons &= ~CMD_BUTTON_UP;
      return false;
    }

    Jump_Start(pm, pml, jump_height->current.value);
    Jump_AddSurfaceEvent(ps, pml);

    if ((ps->pm_flags & PMF_LADDER) != 0)
    {
      Jump_PushOffLadder(ps, pml);
    }

    BG_AnimUpdatePlayerStateConditions(pm);
    BG_AnimScriptEvent(ps, pm->cmd.forwardmove < 0 ? ANIM_ET_JUMPBK : ANIM_ET_JUMP, 0, 1, &pml->holdrand);

    return true;
  }

  void Jump_ClampVelocity(playerState_s* ps, const float* origin)
  {
    if (!(ps->origin[2] - origin[2] > 0.0f))
    {
      return;
    }

    const float height = ps->jumpOriginZ + jump_height->current.value - ps->origin[2];

    if (0.1f > height)
    {
      ps->velocity[2] = 0.0f;
      return;
    }

    const float velocity = std::sqrt(height * 2.0f * static_cast<float>(ps->gravity));

    if (ps->velocity[2] > velocity)
    {
      ps->velocity[2] = velocity;
    }
  }

  bool Jump_GetStepHeight(playerState_s* ps, const float* origin, float* stepSize)
  {
    const float top = ps->jumpOriginZ + jump_height->current.value;

    if (!(top > origin[2]))
    {
      return false;
    }

    *stepSize = jump_stepSize->current.value;

    if (origin[2] + *stepSize > top)
    {
      *stepSize = top - origin[2];
    }

    return true;
  }

  bool Jump_IsPlayerAboveMax(playerState_s* ps)
  {
    return ps->origin[2] >= ps->jumpOriginZ + jump_height->current.value;
  }

  float Jump_GetLandFactor(playerState_s* ps)
  {
    if (ps->pm_time > 1800)
    {
      Jump_ClearState(ps);
      return 1.0f;
    }

    return LandFactor(ps);
  }

  void Jump_Start(pmove_s* pm, pml_t* pml, float height)
  {
    playerState_s* ps = pm->ps;

    float velocitySqr = static_cast<float>(ps->gravity) * (height + height);
    const bool dive = Jump_StartDive(pm, pml);

    if ((ps->pm_flags & PMF_JUMPING) != 0 && ps->pm_time <= 1800 && !dive)
    {
      velocitySqr /= LandFactor(ps);
    }

    pml->groundPlane = 0;
    pml->almostGroundPlane = 0;
    pml->walking = 0;

    ps->groundEntityNum = 0x7FF;
    ps->jumpTime = pm->cmd.serverTime;
    ps->jumpOriginZ = ps->origin[2];
    ps->velocity[2] = std::sqrt(velocitySqr);

    const float spread = ps->weapCommon.aimSpreadScale + jump_spreadAdd->current.value;
    ps->weapCommon.aimSpreadScale = spread < 255.0f ? spread : 255.0f;

    ps->pm_flags = (ps->pm_flags & ~(PMF_TIME_HARDLANDING | PMF_TIME_KNOCKBACK)) | PMF_JUMPING;
    ps->pm_time = 0;
    ps->sprintState.sprintButtonUpRequired = 0;

    if (!dive)
    {
      ps->pm_flags &= ~PMF_DIVING;
      return;
    }

    ps->pm_flags |= PMF_DIVING;

    if (ps->velocity[0] == 0.0f && ps->velocity[1] == 0.0f)
    {
      ps->diveDirection = 43;
      return;
    }

    // Divide the dive direction relative to the view into eight sectors.
    //
    const float yaw = std::atan2(ps->velocity[1], ps->velocity[0]) * 57.295776f - ps->viewangles[1];
    ps->diveDirection = static_cast<int>((AngleNormalize360(yaw) + 22.5f) / 45.0f) % 8;
  }
}
