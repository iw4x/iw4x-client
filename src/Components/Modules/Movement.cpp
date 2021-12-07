#include "STDInclude.hpp"

namespace Components
{
    Dvar::Var Movement::PlayerDuckedSpeedScale;
    Dvar::Var Movement::PlayerLastStandCrawlSpeedScale;
    Dvar::Var Movement::PlayerProneSpeedScale;

    int Movement::PMGetEffectiveStance(Game::playerState_s* ps)
    {
        auto heightTarget = ps->viewHeightTarget;

        if (heightTarget == 0x16)
            return Game::PM_EFF_STANCE_LASTSTANDCRAWL;

        if (heightTarget == 0x28)
            return Game::PM_EFF_STANCE_DUCKED;

        if (heightTarget == 0xB)
            return Game::PM_EFF_STANCE_PRONE;

        return Game::PM_EFF_STANCE_DEFAULT;
    }

    float Movement::PMCmdScaleForStance(Game::pmove_s* move)
    {
        auto* playerState = move->ps;
        float scale;

        if (playerState->viewHeightLerpTime != 0 && playerState->viewHeightLerpTarget == 0xB)
        {
            scale = move->cmd.serverTime - playerState->viewHeightLerpTime / 400.0f;

            if (0.0f <= scale)
            {
                auto flags = 0;

                if (scale < 1.0f)
                {
                    flags |= 1 << 8;
                }

                if (scale == 1.0f)
                {
                    flags |= 1 << 14;
                }

                if (flags == 0)
                {
                    scale = 1.0f;
                    return scale * 0.15f + (1.0f - scale) * 0.65f;
                }

                if (scale != 0.0f)
                {
                    return scale * 0.15f + (1.0f - scale) * 0.65f;
                }
            }
        }

        if ((playerState->viewHeightLerpTime != 0 && playerState->viewHeightLerpTarget == 0x28) &&
            playerState->viewHeightLerpDown == 0)
        {
            scale = 400.0f / move->cmd.serverTime - playerState->viewHeightLerpTime;

            if (0.0f <= scale)
            {
                auto flags = 0;

                if (scale < 1.0f)
                {
                    flags |= 1 << 8;
                }

                if (scale == 1.0f)
                {
                    flags |= 1 << 14;
                }

                if (flags == 0)
                {
                    scale = 1.0f;
                }
                else if (scale != 0.0f)
                {
                    return scale * 0.65f + (1.0f - scale) * 0.15f;
                }
            }
        }

        scale = 1.0f;
        auto stance = Movement::PMGetEffectiveStance(playerState);

        if (stance == Game::PM_EFF_STANCE_PRONE)
        {
            scale = Movement::PlayerProneSpeedScale.get<float>();
        }

        else if (stance == Game::PM_EFF_STANCE_DUCKED)
        {
            scale = Movement::PlayerDuckedSpeedScale.get<float>();
        }

        else if (stance == Game::PM_EFF_STANCE_LASTSTANDCRAWL)
        {
            scale = Movement::PlayerLastStandCrawlSpeedScale.get<float>();
        }

        return scale;
    }

    __declspec(naked) void Movement::PMCmdScaleForStanceStub()
    {
        __asm
        {
            pushad

            push edx
            call Movement::PMCmdScaleForStance
            add esp, 4

            popad
            ret
        }
    }

    Game::dvar_t* Movement::Dvar_RegisterLastStandSpeedScale(const char* name, float defaultVal, float min, float max, int, const char* desc)
    {
        Movement::PlayerLastStandCrawlSpeedScale = Dvar::Register<float>(name, defaultVal,
            min, max, Game::DVAR_FLAG_CHEAT | Game::DVAR_FLAG_REPLICATED, desc);

        return Movement::PlayerLastStandCrawlSpeedScale.get<Game::dvar_t*>();
    }

    Movement::Movement()
    {
        Dvar::OnInit([]
        {
            Movement::PlayerDuckedSpeedScale = Dvar::Register<float>("player_duckedSpeedScale",
                0.65f, 0.0f, 5.0f, Game::DVAR_FLAG_CHEAT | Game::DVAR_FLAG_REPLICATED,
                "The scale applied to the player speed when ducking");

            Movement::PlayerProneSpeedScale = Dvar::Register<float>("player_proneSpeedScale",
                0.15f, 0.0f, 5.0f, Game::DVAR_FLAG_CHEAT | Game::DVAR_FLAG_REPLICATED,
                "The scale applied to the player speed when crawling");
        });

        // Hook PM_CmdScaleForStance in PM_CmdScale_Walk
        Utils::Hook(0x572F34, Movement::PMCmdScaleForStanceStub, HOOK_CALL).install()->quick();

        //Hook PM_CmdScaleForStance in PM_GetMaxSpeed
        Utils::Hook(0x57395F, Movement::PMCmdScaleForStanceStub, HOOK_CALL).install()->quick();

        // Hook Dvar_RegisterFloat. Only thing that's changed is that the 0x80 flag is not used.
        Utils::Hook(0x448B66, Movement::Dvar_RegisterLastStandSpeedScale, HOOK_CALL).install()->quick();
    }

    Movement::~Movement()
    {
    }
}
