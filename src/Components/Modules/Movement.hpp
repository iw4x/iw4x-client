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

        static int PMGetEffectiveStance(Game::playerState_s* ps);
        static float PMCmdScaleForStance(Game::pmove_s* move);
        static void PMCmdScaleForStanceStub();

        static Game::dvar_t* Dvar_RegisterLastStandSpeedScale(const char* name, float defaultVal, float min, float max, int flags, const char* desc);
    };
}
