#pragma once

namespace Components
{
    class Movement : public Component
    {
    public:
        Movement();
        ~Movement();

    private:
        static Dvar::Var PlayerDuckedSpeedScale;
        static Dvar::Var PlayerLastStandCrawlSpeedScale;
        static Dvar::Var PlayerProneSpeedScale;
        static Dvar::Var PlayerSpectateSpeedScale;
        static Dvar::Var CGUfoScaler;
        static Dvar::Var CGNoclipScaler;

        static float PM_CmdScaleForStance(const Game::pmove_s* move);
        static void PM_CmdScaleForStanceStub();

        static float PM_MoveScale(Game::playerState_s* ps, float forwardmove, float rightmove, float upmove);
        static void PM_MoveScaleStub();

        static Game::dvar_t* Dvar_RegisterLastStandSpeedScale(const char* name, float value, float min, float max, int flags, const char* desc);
        static Game::dvar_t* Dvar_RegisterSpectateSpeedScale(const char* name, float value, float min, float max, int flags, const char* desc);
    };
}
