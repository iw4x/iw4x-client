#pragma once

extern "C"
{
  extern const Game::dvar_t* PlayerMovement_PlayerDuckedSpeedScale;
  extern const Game::dvar_t* PlayerMovement_PlayerProneSpeedScale;
  extern const Game::dvar_t* PlayerMovement_CGNoclipScaler;
  extern const Game::dvar_t* PlayerMovement_CGUfoScaler;
  extern const Game::dvar_t* PlayerMovement_PlayerSpectateSpeedScale;
  extern const Game::dvar_t* PlayerMovement_BGBounces;
  extern const Game::dvar_t* PlayerMovement_BGBouncesAllAngles;
  extern const Game::dvar_t* PlayerMovement_BGBunnyHopAuto;
  extern const Game::dvar_t* PlayerMovement_BGLadderFixedInput;
  extern const Game::dvar_t* PlayerMovement_BGSprintIgnoreRepress;
  extern const Game::dvar_t* PlayerMovement_BGOmnimovement;
  extern const Game::dvar_t* PlayerMovement_BGDive;
  extern const Game::dvar_t* PlayerMovement_BGOmnimovementDive;

  void PlayerMovement_PM_PlayerDuckedSpeedScaleStub();
  void PlayerMovement_PM_PlayerProneSpeedScaleStub();
  void PlayerMovement_PM_MoveScale_Noclip();
  void PlayerMovement_PM_MoveScale_Ufo();
  void PlayerMovement_PM_MoveScale_Spectate();
  void PlayerMovement_PM_StepSlideMoveStub();
  void PlayerMovement_PM_ProjectVelocityStub();
  void PlayerMovement_Jump_Check_Stub();
  void PlayerMovement_PM_LadderMove_PitchStub();
  void PlayerMovement_PM_UpdateSprint_RepressCallStub();
  void PlayerMovement_CL_KeyMove_SprintBit_Stub();
  void PlayerMovement_PM_SprintStartInterferingButtons_Stub();
  void PlayerMovement_PM_SprintEndingButtons_Stub();
  void PlayerMovement_PM_WalkMove_SprintStrafeStub();
  void PlayerMovement_PM_SetMovementDir_ClampProneLadder_Stub();
  void PlayerMovement_PM_SetMovementDir_ClampGeneric_Stub();
  void PlayerMovement_PM_SetStrafeCondition_Stub();
  void PlayerMovement_Jump_CheckDive_PerkGate_Stub();
  void PlayerMovement_PM_GetMaxSpeed_BackDiagonal_Stub();
  void PlayerMovement_PM_GetMaxSpeed_BackPure_Stub();
  void PlayerMovement_Jump_CheckDive_BackDiveVelocity_Stub();

  int PlayerMovement_ComputeHorizontalIntent(int forwardSpeed, int rightSpeed);
  void PlayerMovement_ApplyStockSprintStrafeScale(Game::pmove_s* pm);
}

namespace Components
{
  class PlayerMovement : public Component
  {
  public:
    PlayerMovement();

  private:
    friend int (::PlayerMovement_ComputeHorizontalIntent)(int forwardSpeed, int rightSpeed);
    friend void (::PlayerMovement_ApplyStockSprintStrafeScale)(Game::pmove_s* pm);

    enum BouncesSettings : int { DISABLED, ENABLED, DOUBLE };

    static constexpr auto SURF_LADDER = 0x8;

    static const Game::dvar_t* BGRocketJump;
    static const Game::dvar_t* BGRocketJumpScale;
    static const Game::dvar_t* BGPlayerEjection;
    static const Game::dvar_t* BGPlayerCollision;
    static const Game::dvar_t* BGClimbAnything;
    static const Game::dvar_t* BGDisableLandingSlowdown;
    static const Game::dvar_t* BGDisableBarrierClips;

    static Game::dvar_t** player_sprintStrafeSpeedScale;

    static void PM_PlayerTraceStub(Game::pmove_s* pm, Game::trace_t* results, const float* start, const float* end, Game::Bounds* bounds, int passEntityNum, int contentMask);

    // Bounce logic
    static void Jump_ClearState_Hk(Game::playerState_s* ps);

    static Game::gentity_s* Weapon_RocketLauncher_Fire_Hk(Game::gentity_s* ent, unsigned int weaponIndex, float spread, Game::weaponParms* wp, const float* gunVel, Game::lockonFireParms* lockParms, bool a7);

    // Player collison
    static int StuckInClient_Hk(Game::gentity_s* self);
    static void CM_TransformedCapsuleTrace_Hk(Game::trace_t* results, const float* start, const float* end, const Game::Bounds* bounds, const Game::Bounds* capsule, int contents, const float* origin, const float* angles);

    static void PM_CrashLand_Stub(const float* v, float scale, const float* result);

    static void GScr_IsSprinting(Game::scr_entref_t entref);

    static const Game::dvar_t* Dvar_RegisterSpectateSpeedScale(const char* dvarName, float value, float min, float max, unsigned __int16 flags, const char* description);

    static void RegisterMovementDvars();

    static void PmoveSingle_Stub(Game::pmove_s* pm);
    static void PM_CheckLadderMove_Stub(Game::pmove_s* pm, Game::pml_t* pml);

    static float* PM_LadderMove_RightVector_Hk(float* source, const float* ladderNormal, float* pmlRight);

    // Omnimovement helpers.
    static int ComputeHorizontalIntent(int forwardSpeed, int rightSpeed);
    static void ApplyStockSprintStrafeScale(Game::pmove_s* pm);
  };
}
