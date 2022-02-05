#pragma once

namespace Components
{
    class Movement : public Component
    {
    public:
        Movement();

    private:
        enum BouncesSettings { DISABLED, ENABLED, DOUBLE };

        static Dvar::Var PlayerDuckedSpeedScale;
        static Dvar::Var PlayerLastStandCrawlSpeedScale;
        static Dvar::Var PlayerProneSpeedScale;
        static Dvar::Var PlayerSpectateSpeedScale;
        static Dvar::Var CGUfoScaler;
        static Dvar::Var CGNoclipScaler;
        static Dvar::Var BGBouncesAllAngles;
        static Dvar::Var BGRocketJump;
        // Can't use Var class inside assembly stubs
        static Game::dvar_t* BGBounces;

        static float PM_CmdScaleForStance(const Game::pmove_s* move);
        static void PM_CmdScaleForStanceStub();

        static float PM_MoveScale(Game::playerState_s* ps, float forwardmove, float rightmove, float upmove);
        static void PM_MoveScaleStub();

        // Bounce logic
        static void PM_StepSlideMoveStub();
        static void PM_ProjectVelocityStub(const float* velIn, const float* normal, float* velOut);
        static void Jump_ClearStateHook(Game::playerState_s* ps);

        static Game::gentity_s* Weapon_RocketLauncher_Fire_Hk(Game::gentity_s* ent, unsigned int weaponIndex, float spread, Game::weaponParms* wp, const float* gunVel, Game::lockonFireParms* lockParms, bool a7);

        static Game::dvar_t* Dvar_RegisterLastStandSpeedScale(const char* name, float value, float min, float max, int flags, const char* desc);
        static Game::dvar_t* Dvar_RegisterSpectateSpeedScale(const char* name, float value, float min, float max, int flags, const char* desc);
    };
}
