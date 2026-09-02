#pragma once

#include "../Types.hpp"

namespace Controller
{
  namespace engine
  {
    using Game::dvar_t;
    using Game::usercmd_s;
    using Game::keyname_t;
    using Game::PlayerKeyState;
    using Game::AimInput;
    using Game::AimOutput;
    using Game::AimAssistGlobals;
    using Game::AimAssistPlayerState;
    using Game::AimScreenTarget;
    using Game::GraphFloat;
    using Game::WeaponDef;
    using Game::playerState_s;
    using Game::PlayerEquippedWeaponState;

    inline Game::PlayerKeyState* const& playerKeys{Game::playerKeys};
    inline Game::AimAssistGlobals* const& aaGlobArray{Game::aaGlobArray};
    inline Game::GraphFloat* const& aaInputGraph{Game::aaInputGraph};
    inline int* const& g_waitingForKey{Game::g_waitingForKey};

    inline constexpr unsigned AIM_ASSIST_GRAPH_COUNT{Game::AIM_ASSIST_GRAPH_COUNT};

    using Game::Dvar_RegisterBool;
    using Game::Dvar_RegisterFloat;
    using Game::Dvar_RegisterInt;
    using Game::Dvar_RegisterString;
    using Game::Dvar_FindVar;
    using Game::Dvar_SetBool;
    using Game::Dvar_SetInt;
    using Game::Dvar_SetFloat;
    using Game::Dvar_SetString;

    inline constexpr auto DVAR_NONE{Game::DVAR_NONE};
    inline constexpr auto DVAR_ARCHIVE{Game::DVAR_ARCHIVE};
    inline constexpr auto DVAR_CHEAT{Game::DVAR_CHEAT};
    inline constexpr auto DVAR_ROM{Game::DVAR_ROM};

    using Game::Key_IsCatcherActive;
    using Game::Key_SetBinding;
    using Game::UI_GetActiveMenu;
    using Game::UI_KeyEvent;
    using Game::Cbuf_AddText;
    using Game::Cbuf_InsertText;
    using Game::CG_ScrollScoreboardUp;
    using Game::CG_ScrollScoreboardDown;
    using Game::Sys_Milliseconds;

    inline constexpr auto KEYCATCH_UI{Game::KEYCATCH_UI};
    inline constexpr auto KEYCATCH_MASK_ANY{Game::KEYCATCH_MASK_ANY};
    inline constexpr auto KEYCATCH_LOCATION_SELECTION{Game::KEYCATCH_LOCATION_SELECTION};
    inline constexpr auto UIMENU_SCOREBOARD{Game::UIMENU_SCOREBOARD};
    inline constexpr auto LOC_SEL_INPUT_NONE{Game::LOC_SEL_INPUT_NONE};
    inline constexpr auto LOC_SEL_INPUT_CONFIRM{Game::LOC_SEL_INPUT_CONFIRM};
    inline constexpr auto LOC_SEL_INPUT_CANCEL{Game::LOC_SEL_INPUT_CANCEL};

    inline constexpr auto K_ENTER{Game::K_ENTER};
    inline constexpr auto K_ESCAPE{Game::K_ESCAPE};
    inline constexpr auto K_UPARROW{Game::K_UPARROW};
    inline constexpr auto K_DOWNARROW{Game::K_DOWNARROW};
    inline constexpr auto K_LEFTARROW{Game::K_LEFTARROW};
    inline constexpr auto K_RIGHTARROW{Game::K_RIGHTARROW};

    using Game::BG_GetWeaponDef;
    using Game::BG_GetViewModelWeaponIndex;
    using Game::BG_GetEquippedWeaponState;
    using Game::GraphFloat_GetValue;
    using Game::AimAssist_UpdateTweakables;
    using Game::AimAssist_UpdateAdsLerp;
    using Game::AimAssist_ApplyAutoMelee;

    inline constexpr auto PMF_FROZEN{Game::PMF_FROZEN};
    inline constexpr auto PWF_USING_OFFHAND{Game::PWF_USING_OFFHAND};
    inline constexpr auto OFFHAND_CLASS_NONE{Game::OFFHAND_CLASS_NONE};
    inline constexpr auto WEAPON_HAND_LEFT{Game::WEAPON_HAND_LEFT};
    inline constexpr auto CMD_BUTTON_ATTACK{Game::CMD_BUTTON_ATTACK};
    inline constexpr auto CMD_BUTTON_THROW{Game::CMD_BUTTON_THROW};

    inline float& view_pitch (int client) noexcept
    {
      return Game::clients[client].clViewangles[0];
    }

    inline float& view_yaw (int client) noexcept
    {
      return Game::clients[client].clViewangles[1];
    }

    inline float max_pitch_speed (int client) noexcept
    {
      return Game::clients[client].cgameMaxPitchSpeed;
    }

    inline float max_yaw_speed (int client) noexcept
    {
      return Game::clients[client].cgameMaxYawSpeed;
    }

    inline int pm_flags (int client) noexcept
    {
      return Game::clients[client].snap.ps.pm_flags;
    }

    inline int last_weapon_hand (int client) noexcept
    {
      return Game::clients[client].snap.ps.weapCommon.lastWeaponHand;
    }

    inline float client_frame_time () noexcept
    {
      return static_cast<float> (Game::cls->frametime) * 0.001f;
    }

    void aim_assist_update (const AimInput& input, AimOutput& output) noexcept;
  }
}
