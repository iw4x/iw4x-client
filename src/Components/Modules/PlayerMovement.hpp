#pragma once

namespace Components
{
	class PlayerMovement : public Component
	{
	public:
		PlayerMovement();

	private:
		enum BouncesSettings : int { DISABLED, ENABLED, DOUBLE };

		static constexpr auto SURF_LADDER = 0x8;

		static const Game::dvar_t* BGRocketJump;
		static const Game::dvar_t* BGRocketJumpScale;
		static const Game::dvar_t* BGPlayerEjection;
		static const Game::dvar_t* BGPlayerCollision;
		static const Game::dvar_t* BGClimbAnything;
		static const Game::dvar_t* CGNoclipScaler;
		static const Game::dvar_t* CGUfoScaler;
		static const Game::dvar_t* PlayerSpectateSpeedScale;
		static const Game::dvar_t* BGBounces;
		static const Game::dvar_t* BGBouncesAllAngles;
		static const Game::dvar_t* BGDisableLandingSlowdown;
		static const Game::dvar_t* BGBunnyHopAuto;
		static const Game::dvar_t* PlayerDuckedSpeedScale;
		static const Game::dvar_t* PlayerProneSpeedScale;
		
		static void PM_PlayerTraceStub(Game::pmove_s* pm, Game::trace_t* results, const float* start, const float* end, Game::Bounds* bounds, int passEntityNum, int contentMask);
		static void PM_PlayerDuckedSpeedScaleStub();
		static void PM_PlayerProneSpeedScaleStub();

		static void PM_MoveScale_Noclip();
		static void PM_MoveScale_Ufo();
		static void PM_MoveScale_Spectate();

		// Bounce logic
		static void PM_StepSlideMoveStub();
		static void PM_ProjectVelocityStub();
		static void Jump_ClearState_Hk(Game::playerState_s* ps);

		static Game::gentity_s* Weapon_RocketLauncher_Fire_Hk(Game::gentity_s* ent, unsigned int weaponIndex, float spread, Game::weaponParms* wp, const float* gunVel, Game::lockonFireParms* lockParms, bool a7);

		// Player collison
		static int StuckInClient_Hk(Game::gentity_s* self);
		static void CM_TransformedCapsuleTrace_Hk(Game::trace_t* results, const float* start, const float* end, const Game::Bounds* bounds, const Game::Bounds* capsule, int contents, const float* origin, const float* angles);

		static void PM_CrashLand_Stub(const float* v, float scale, const float* result);
		static void Jump_Check_Stub();

		static void GScr_IsSprinting(Game::scr_entref_t entref);

		static const Game::dvar_t* Dvar_RegisterSpectateSpeedScale(const char* dvarName, float value, float min, float max, unsigned __int16 flags, const char* description);

		static void RegisterMovementDvars();
	};
}
