#pragma once

namespace Components
{
	class Movement : public Component
	{
	public:
		Movement();

	private:
		enum BouncesSettings { DISABLED, ENABLED, DOUBLE };

		static Dvar::Var PlayerSpectateSpeedScale;
		static Dvar::Var CGUfoScaler;
		static Dvar::Var CGNoclipScaler;
		static Dvar::Var BGBouncesAllAngles;
		static Dvar::Var BGRocketJump;
		static Dvar::Var BGPlayerEjection;
		static Dvar::Var BGPlayerCollision;
		// Can't use Var class inside assembly stubs
		static Game::dvar_t* BGBounces;
		static Game::dvar_t* PlayerDuckedSpeedScale;
		static Game::dvar_t* PlayerProneSpeedScale;

		static void PM_PlayerDuckedSpeedScaleStub();
		static void PM_PlayerProneSpeedScaleStub();

		static float PM_MoveScale(Game::playerState_s* ps, float fmove, float rmove, float umove);
		static void PM_MoveScaleStub();

		// Bounce logic
		static void PM_StepSlideMoveStub();
		static void PM_ProjectVelocityStub(const float* velIn, const float* normal, float* velOut);
		static void Jump_ClearState_Hk(Game::playerState_s* ps);

		static Game::gentity_s* Weapon_RocketLauncher_Fire_Hk(Game::gentity_s* ent, unsigned int weaponIndex, float spread, Game::weaponParms* wp, const float* gunVel, Game::lockonFireParms* lockParms, bool a7);

		// Player collison
		static int StuckInClient_Hk(Game::gentity_s* self);
		static void CM_TransformedCapsuleTrace_Hk(Game::trace_t* results, const float* start, const float* end, const Game::Bounds* bounds, const Game::Bounds* capsule, int contents, const float* origin, const float* angles);

		static Game::dvar_t* Dvar_RegisterSpectateSpeedScale(const char* dvarName, float value, float min, float max, unsigned __int16 flags, const char* description);
	};
}
