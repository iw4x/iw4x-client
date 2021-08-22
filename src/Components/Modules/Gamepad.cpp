#include "STDInclude.hpp"

#include <limits>

#define XINPUT_SENSITIVITY_MULTIPLIER 4 // Arbitrary value I multiply the xinput senstivity dvar with to get nicer values (0-10 range or something)
#define SIGN(d) ((d > 0) - (d < 0))

namespace Game
{
    ButtonToCodeMap_t buttonList[]
    {
        {GPAD_X, K_BUTTON_X},
        {GPAD_A, K_BUTTON_A},
        {GPAD_B, K_BUTTON_B},
        {GPAD_Y, K_BUTTON_Y},
        {GPAD_L_TRIG, K_BUTTON_LTRIG},
        {GPAD_R_TRIG, K_BUTTON_RTRIG},
        {GPAD_L_SHLDR, K_BUTTON_LSHLDR},
        {GPAD_R_SHLDR, K_BUTTON_RSHLDR},
        {GPAD_START, K_BUTTON_START},
        {GPAD_BACK, K_BUTTON_BACK},
        {GPAD_L3, K_BUTTON_LSTICK},
        {GPAD_R3, K_BUTTON_RSTICK},
        {GPAD_UP, K_DPAD_UP},
        {GPAD_DOWN, K_DPAD_DOWN},
        {GPAD_LEFT, K_DPAD_LEFT},
        {GPAD_RIGHT, K_DPAD_RIGHT}
    };

    StickToCodeMap_t analogStickList[4]
    {
        {GPAD_LX, K_APAD_RIGHT, K_APAD_LEFT},
        {GPAD_LY, K_APAD_UP, K_APAD_DOWN},
        {GPAD_RX, K_APAD_RIGHT, K_APAD_LEFT},
        {GPAD_RY, K_APAD_UP, K_APAD_DOWN},
    };

    GamePadStick stickForAxis[GPAD_PHYSAXIS_COUNT]
    {
        GPAD_RX,
        GPAD_RY,
        GPAD_LX,
        GPAD_LY,
        GPAD_INVALID,
        GPAD_INVALID
    };

    const char* physicalAxisNames[GPAD_PHYSAXIS_COUNT]
    {
        "A_RSTICK_X",
        "A_RSTICK_Y",
        "A_LSTICK_X",
        "A_LSTICK_Y",
        "A_RTRIGGER",
        "A_LTRIGGER"
    };

    const char* virtualAxisNames[GPAD_VIRTAXIS_COUNT]
    {
        "VA_SIDE",
        "VA_FORWARD",
        "VA_UP",
        "VA_YAW",
        "VA_PITCH",
        "VA_ATTACK"
    };

    const char* gamePadMappingTypeNames[GPAD_MAP_COUNT]
    {
        "MAP_LINEAR",
        "MAP_SQUARED"
    };

    keyNum_t menuScrollButtonList[]
    {
        K_APAD_UP,
        K_APAD_DOWN,
        K_APAD_LEFT,
        K_APAD_RIGHT,
        K_DPAD_UP,
        K_DPAD_DOWN,
        K_DPAD_LEFT,
        K_DPAD_RIGHT
    };

    keyname_t extendedKeyNames[]
    {
        {"BUTTON_A", K_BUTTON_A},
        {"BUTTON_B", K_BUTTON_B},
        {"BUTTON_X", K_BUTTON_X},
        {"BUTTON_Y", K_BUTTON_Y},
        {"BUTTON_LSHLDR", K_BUTTON_LSHLDR},
        {"BUTTON_RSHLDR", K_BUTTON_RSHLDR},
        {"BUTTON_START", K_BUTTON_START},
        {"BUTTON_BACK", K_BUTTON_BACK},
        {"BUTTON_LSTICK", K_BUTTON_LSTICK},
        {"BUTTON_RSTICK", K_BUTTON_RSTICK},
        {"BUTTON_LTRIG", K_BUTTON_LTRIG},
        {"BUTTON_RTRIG", K_BUTTON_RTRIG},
        {"DPAD_UP", K_DPAD_UP},
        {"DPAD_DOWN", K_DPAD_DOWN},
        {"DPAD_LEFT", K_DPAD_LEFT},
        {"DPAD_RIGHT", K_DPAD_RIGHT},
    };

    keyname_t extendedLocalizedKeyNames[]
    {
        // Material text icons pattern: 0x01 width height material_name_len
        {"^\x01\x32\x32\x08""button_a", K_BUTTON_A},
        {"^\x01\x32\x32\x08""button_b", K_BUTTON_B},
        {"^\x01\x32\x32\x08""button_x", K_BUTTON_X},
        {"^\x01\x32\x32\x08""button_y", K_BUTTON_Y},
        {"^\x01\x32\x32\x0D""button_lshldr", K_BUTTON_LSHLDR},
        {"^\x01\x32\x32\x0D""button_rshldr", K_BUTTON_RSHLDR},
        {"^\x01\x32\x32\x0C""button_start", K_BUTTON_START},
        {"^\x01\x32\x32\x0B""button_back", K_BUTTON_BACK},
        {"^\x01\x48\x32\x0D""button_lstick", K_BUTTON_LSTICK},
        {"^\x01\x48\x32\x0D""button_rstick", K_BUTTON_RSTICK},
        {"^\x01\x32\x32\x0C""button_ltrig", K_BUTTON_LTRIG},
        {"^\x01\x32\x32\x0C""button_rtrig", K_BUTTON_RTRIG},
        {"^\x01\x32\x32\x07""dpad_up", K_DPAD_UP},
        {"^\x01\x32\x32\x09""dpad_down", K_DPAD_DOWN},
        {"^\x01\x32\x32\x09""dpad_left", K_DPAD_LEFT},
        {"^\x01\x32\x32\x0A""dpad_right", K_DPAD_RIGHT},
    };

    constexpr auto VANILLA_KEY_NAME_COUNT = 95;
    constexpr auto VANILLA_LOCALIZED_KEY_NAME_COUNT = 95;
    keyname_t combinedKeyNames[VANILLA_KEY_NAME_COUNT + std::extent_v<decltype(extendedKeyNames)> + 1];
    keyname_t combinedLocalizedKeyNames[VANILLA_KEY_NAME_COUNT + std::extent_v<decltype(extendedLocalizedKeyNames)> + 1];

    PlayerKeyState* playerKeys = reinterpret_cast<PlayerKeyState*>(0xA1B7D0);
    keyname_t* vanillaKeyNames = reinterpret_cast<keyname_t*>(0x798580);
    keyname_t* vanillaLocalizedKeyNames = reinterpret_cast<keyname_t*>(0x798880);
}

namespace Components
{
    Gamepad::GamePad Gamepad::gamePads[Game::MAX_GAMEPADS]{};
    Gamepad::GamePadGlobals Gamepad::gamePadGlobals[Game::MAX_GAMEPADS]{{}};
    int Gamepad::gamePadBindingsModifiedFlags = 0;
    std::chrono::milliseconds Gamepad::timeAtFirstHeldMaxLookX = 0ms; // "For how much time in milliseconds has the player been holding a horizontal direction on their stick, fully" (-1.0 or 1.0)
    bool Gamepad::isHoldingMaxLookX = false;
    bool Gamepad::isADS;

    Dvar::Var Gamepad::gpad_enabled;
    Dvar::Var Gamepad::gpad_debug;
    Dvar::Var Gamepad::gpad_present;
    Dvar::Var Gamepad::gpad_in_use;
    Dvar::Var Gamepad::gpad_sticksConfig;
    Dvar::Var Gamepad::gpad_buttonConfig;
    Dvar::Var Gamepad::gpad_menu_scroll_delay_first;
    Dvar::Var Gamepad::gpad_menu_scroll_delay_rest;
    Dvar::Var Gamepad::gpad_rumble;
    Dvar::Var Gamepad::gpad_stick_pressed_hysteresis;
    Dvar::Var Gamepad::gpad_stick_pressed;
    Dvar::Var Gamepad::gpad_stick_deadzone_max;
    Dvar::Var Gamepad::gpad_stick_deadzone_min;
    Dvar::Var Gamepad::gpad_button_deadzone;
    Dvar::Var Gamepad::gpad_button_rstick_deflect_max;
    Dvar::Var Gamepad::gpad_button_lstick_deflect_max;

    Dvar::Var Gamepad::xpadSensitivity;
    Dvar::Var Gamepad::xpadEarlyTime;
    Dvar::Var Gamepad::xpadEarlyMultiplier;
    Dvar::Var Gamepad::xpadHorizontalMultiplier;
    Dvar::Var Gamepad::xpadVerticalMultiplier;
    Dvar::Var Gamepad::xpadAdsMultiplier;

    Game::GamePadStickDir Gamepad::lastMenuNavigationDirection = Game::GPAD_STICK_DIR_COUNT;
    std::chrono::milliseconds Gamepad::lastNavigationTime = 0ms;
    std::chrono::milliseconds Gamepad::msBetweenNavigations = 220ms;

    struct ControllerMenuKeyMapping
    {
        Game::keyNum_t controllerKey;
        Game::keyNum_t pcKey;
    };

    ControllerMenuKeyMapping controllerMenuKeyMappings[]
    {
        {Game::K_BUTTON_A, Game::K_ENTER},
        {Game::K_BUTTON_START, Game::K_ENTER},
        {Game::K_BUTTON_B, Game::K_ESCAPE},
        {Game::K_BUTTON_BACK, Game::K_ESCAPE},
        {Game::K_DPAD_UP, Game::K_UPARROW},
        {Game::K_APAD_UP, Game::K_UPARROW},
        {Game::K_DPAD_DOWN, Game::K_DOWNARROW},
        {Game::K_APAD_DOWN, Game::K_DOWNARROW},
        {Game::K_DPAD_LEFT, Game::K_LEFTARROW},
        {Game::K_APAD_LEFT, Game::K_LEFTARROW},
        {Game::K_DPAD_RIGHT, Game::K_RIGHTARROW},
        {Game::K_APAD_RIGHT, Game::K_RIGHTARROW},
    };


    // This should be read from a text file in the players/ folder, most probably / or from config_mp.cfg
    std::vector<Gamepad::ActionMapping> mappings = {
        Gamepad::ActionMapping(XINPUT_GAMEPAD_A, "gostand"),
        Gamepad::ActionMapping(XINPUT_GAMEPAD_B, "stance"),
        Gamepad::ActionMapping(XINPUT_GAMEPAD_X, "usereload"),
        Gamepad::ActionMapping(XINPUT_GAMEPAD_Y, "weapnext", false),
        Gamepad::ActionMapping(XINPUT_GAMEPAD_LEFT_SHOULDER, "smoke"),
        Gamepad::ActionMapping(XINPUT_GAMEPAD_RIGHT_SHOULDER, "frag"),
        Gamepad::ActionMapping(XINPUT_GAMEPAD_LEFT_THUMB, "breath_sprint"),
        Gamepad::ActionMapping(XINPUT_GAMEPAD_RIGHT_THUMB, "melee"),
        Gamepad::ActionMapping(XINPUT_GAMEPAD_START, "togglemenu", false),
        Gamepad::ActionMapping(XINPUT_GAMEPAD_BACK, "scores"),
        Gamepad::ActionMapping(XINPUT_GAMEPAD_DPAD_LEFT, "actionslot 3"),
        Gamepad::ActionMapping(XINPUT_GAMEPAD_DPAD_RIGHT, "actionslot 2"),
        Gamepad::ActionMapping(XINPUT_GAMEPAD_DPAD_DOWN, "actionslot 1"),
        Gamepad::ActionMapping(XINPUT_GAMEPAD_DPAD_UP, "actionslot 4"),
    };

    Gamepad::GamePadGlobals::GamePadGlobals()
        : axes{},
          nextScrollTime(0)
    {
        for (auto& virtualAxis : axes.virtualAxes)
        {
            virtualAxis.physicalAxis = Game::GPAD_PHYSAXIS_NONE;
            virtualAxis.mapType = Game::GPAD_MAP_NONE;
        }
    }


    // Same thing

    //	void Gamepad::Vibrate(int leftVal, int rightVal)
    //	{
    //		// Create a Vibraton State
    //		XINPUT_VIBRATION Vibration;
    //
    //		// Zeroise the Vibration
    //		ZeroMemory(&Vibration, sizeof(XINPUT_VIBRATION));
    //
    //		// Set the Vibration Values
    //		Vibration.wLeftMotorSpeed = leftVal;
    //		Vibration.wRightMotorSpeed = rightVal;
    //
    //		// Vibrate the controller
    //		XInputSetState(xiPlayerNum, &Vibration);
    //	}

    void Gamepad::CL_GamepadMove(int, Game::usercmd_s* cmd)
    {
        auto& gamePad = gamePads[0];

        if (gamePad.enabled)
        {
            if (std::fabs(gamePad.sticks[0]) > 0.0f || std::fabs(gamePad.sticks[1]) > 0.0f)
            {
                // We check for 0:0 again so we don't overwrite keyboard input in case the user doesn't feel like using their gamepad, even though its plugged in
                cmd->rightmove = static_cast<char>(gamePad.sticks[0] * static_cast<float>(std::numeric_limits<char>().max()));
                cmd->forwardmove = static_cast<char>(gamePad.sticks[1] * static_cast<float>(std::numeric_limits<char>().max()));
            }

            const bool pressingLeftTrigger = gamePad.analogs[0] > TRIGGER_THRESHOLD_F;
            const bool previouslyPressingLeftTrigger = gamePad.lastAnalogs[0] > TRIGGER_THRESHOLD_F;
            if (pressingLeftTrigger != previouslyPressingLeftTrigger)
            {
                if (pressingLeftTrigger)
                {
                    Command::Execute("+speed_throw");
                    isADS = true;
                }
                else
                {
                    Command::Execute("-speed_throw");
                    isADS = false;
                }
            }

            const bool pressingRightTrigger = gamePad.analogs[1] > TRIGGER_THRESHOLD_F;
            const bool previouslyPressingRightTrigger = gamePad.lastAnalogs[1] > TRIGGER_THRESHOLD_F;
            if (pressingRightTrigger != previouslyPressingRightTrigger)
            {
                if (pressingRightTrigger)
                {
                    Command::Execute("+attack");
                }
                else
                {
                    Command::Execute("-attack");
                }
            }

            // Buttons (on/off) mappings
            for (auto& i : mappings)
            {
                auto mapping = i;
                auto action = mapping.action;
                auto antiAction = mapping.action;

                if (mapping.isReversible)
                {
                    action = "+" + mapping.action;
                    antiAction = "-" + mapping.action;
                }
                else if (mapping.wasPressed)
                {
                    if (gamePad.digitals & mapping.input)
                    {
                        // Button still pressed, do not send info
                    }
                    else
                    {
                        i.wasPressed = false;
                    }

                    continue;
                }

                if (gamePad.digitals & mapping.input)
                {
                    if (mapping.spamWhenHeld || !i.wasPressed)
                    {
                        Command::Execute(action);
                    }
                    i.wasPressed = true;
                }
                else if (mapping.isReversible && mapping.wasPressed)
                {
                    i.wasPressed = false;
                    Command::Execute(antiAction);
                }
            }
        }
    }

    __declspec(naked) void Gamepad::CL_CreateCmdStub()
    {
        __asm
            {
            // do xinput!
            push esi
            push ebp
            call CL_GamepadMove
            add esp, 8h

            // execute code we patched over
            add esp, 4
            fld st
            pop ebx

            // return back
            push 0x5A6DBF
            retn
            }
    }

    __declspec(naked) void Gamepad::MSG_WriteDeltaUsercmdKeyStub()
    {
        __asm
            {
            // fix stack pointer
            add esp, 0Ch

            // put both forward move and rightmove values in the movement button
            mov dl, byte ptr[edi + 1Ah] // to_forwardMove
            mov dh, byte ptr[edi + 1Bh] // to_rightMove

            mov[esp + 30h], dx // to_buttons

            mov dl, byte ptr[ebp + 1Ah] // from_forwardMove
            mov dh, byte ptr[ebp + 1Bh] // from_rightMove

            mov[esp + 2Ch], dx // from_buttons

            // return back
            push 0x60E40E
            retn
            }
    }

    void Gamepad::ApplyMovement(Game::msg_t* msg, int key, Game::usercmd_s* from, Game::usercmd_s* to)
    {
        char forward;
        char right;

        if (Game::MSG_ReadBit(msg))
        {
            short movementBits = static_cast<short>(key ^ Game::MSG_ReadBits(msg, 16));

            forward = static_cast<char>(movementBits);
            right = static_cast<char>(movementBits >> 8);
        }
        else
        {
            forward = from->forwardmove;
            right = from->rightmove;
        }

        to->forwardmove = forward;
        to->rightmove = right;
    }

    __declspec(naked) void Gamepad::MSG_ReadDeltaUsercmdKeyStub()
    {
        __asm
            {
            push ebx // to
            push ebp // from
            push edi // key
            push esi // msg
            call ApplyMovement
            add esp, 10h

            // return back
            push 0x4921BF
            ret
            }
    }

    __declspec(naked) void Gamepad::MSG_ReadDeltaUsercmdKeyStub2()
    {
        __asm
            {
            push ebx // to
            push ebp // from
            push edi // key
            push esi // msg
            call ApplyMovement
            add esp, 10h

            // return back
            push 3
            push esi
            push 0x492085
            ret
            }
    }

    int Gamepad::unk_CheckKeyHook(int localClientNum, Game::keyNum_t keyCode)
    {
        const auto& gamePad = gamePads[0];

        if (gamePad.enabled)
        {
            if (keyCode == Game::keyNum_t::K_MOUSE2)
            {
                const bool pressingLeftTrigger = gamePad.analogs[0] > TRIGGER_THRESHOLD_F;
                const bool previouslyPressingLeftTrigger = gamePad.lastAnalogs[0] > TRIGGER_THRESHOLD_F;
                if (pressingLeftTrigger != previouslyPressingLeftTrigger)
                {
                    if (pressingLeftTrigger)
                    {
                        return 1;
                    }
                    else
                    {
                        return 0;
                    }
                }
            }
        }

        return Utils::Hook::Call<int(int, Game::keyNum_t)>(0x48B2D0)(localClientNum, keyCode);
    }

    void Gamepad::MouseOverride(Game::clientActive_t* clientActive, float* mx, float* my)
    {
        CL_GetMouseMovementCl(clientActive, mx, my);

        const auto& gamePad = gamePads[0];

        if (gamePad.enabled)
        {
            float viewSensitivityMultiplier = xpadSensitivity.get<float>() * XINPUT_SENSITIVITY_MULTIPLIER;

            float lockedSensitivityMultiplier = xpadEarlyMultiplier.get<float>();
            float generalXSensitivityMultiplier = xpadHorizontalMultiplier.get<float>();
            float generalYSensitivityMultiplier = xpadVerticalMultiplier.get<float>();
            std::chrono::milliseconds msBeforeUnlockingSensitivity = std::chrono::milliseconds(xpadEarlyTime.get<int>());

            float viewStickX = gamePad.sticks[2];
            float viewStickY = gamePad.sticks[3];

            // Gamepad horizontal acceleration on view
            if (abs(viewStickX) > 0.80f)
            {
                if (!isHoldingMaxLookX)
                {
                    isHoldingMaxLookX = true;
                    timeAtFirstHeldMaxLookX = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
                }
                else
                {
                    std::chrono::milliseconds hasBeenHoldingLeftXForMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) -
                        timeAtFirstHeldMaxLookX;
#ifdef STEP_SENSITIVITY
					if (hasBeenHoldingLeftXForMs < msBeforeUnlockingSensitivity) {
						viewStickX *= lockedSensitivityMultiplier;
					}
#else
                    float coeff = std::clamp(hasBeenHoldingLeftXForMs.count() / (float)msBeforeUnlockingSensitivity.count(), 0.0F, 1.0F);
                    viewStickX *= lockedSensitivityMultiplier + coeff * (1.0f - lockedSensitivityMultiplier);
#endif
                }
            }
            else
            {
                isHoldingMaxLookX = false;
                timeAtFirstHeldMaxLookX = 0ms;
                viewStickX *= lockedSensitivityMultiplier;
            }

            float adsMultiplier = 1.0f;

            auto ps = &clientActive->snap.ps;

            // DO NOT use clientActive->usingAds ! It only works for toggle ADS
            if (PM_IsAdsAllowed(ps) && isADS)
            {
                adsMultiplier = xpadAdsMultiplier.get<float>();
            }

            if (viewStickX != 0 || viewStickY != 0)
            {
                *(my) = viewStickX * viewSensitivityMultiplier * generalXSensitivityMultiplier * adsMultiplier;
                *(mx) = -viewStickY * viewSensitivityMultiplier * generalYSensitivityMultiplier * adsMultiplier;
            }

            // Handling killstreaks
            const bool pressingRightTrigger = gamePad.analogs[1] > TRIGGER_THRESHOLD_F;
            const bool previouslyPressingRightTrigger = gamePad.lastAnalogs[1] > TRIGGER_THRESHOLD_F;
            if (pressingRightTrigger != previouslyPressingRightTrigger)
            {
                bool* isInPredator = reinterpret_cast<bool*>(0x8EE3B8);

                if (pressingRightTrigger)
                {
                    Utils::Hook::Set(0xA1C4F4, Game::LOC_SEL_INPUT_CONFIRM);
                    if (*isInPredator)
                    {
                        // Yea, that's how we boost
                        // Command::execute is sync by default so the predator event gets fired properly
                        Command::Execute("+attack");
                        Command::Execute("-attack");
                    }
                }
            }
        }
    }

    // Game -> Client DLL
    __declspec(naked) void CL_GetMouseMovementStub()
    {
        __asm
            {
            push edx;
            push ecx;
            push eax;
            call Gamepad::MouseOverride;
            add esp, 0xC;
            ret;
            }
    }

    // Client DLL -> Game
    void Gamepad::CL_GetMouseMovementCl(Game::clientActive_t* result, float* mx, float* my)
    {
        __asm
            {
            push ebx;
            push ecx;
            push edx;
            mov eax, result;
            mov ecx, mx;
            mov edx, my;
            mov ebx, 5A60E0h;
            call ebx;
            pop edx;
            pop ecx;
            pop ebx;
            }
    }

    bool Gamepad::GPad_Check(const int gamePadIndex, const int portIndex)
    {
        assert(gamePadIndex < Game::MAX_GAMEPADS);
        auto& gamePad = gamePads[gamePadIndex];

        if (XInputGetCapabilities(portIndex, XINPUT_FLAG_GAMEPAD, &gamePad.caps) == ERROR_SUCCESS)
        {
            gamePad.enabled = true;
            gamePad.portIndex = portIndex;
            return true;
        }

        gamePad.enabled = false;
        return false;
    }

    void Gamepad::GPad_RefreshAll()
    {
        auto currentGamePadNum = 0;

        for (auto currentPort = 0; currentPort < XUSER_MAX_COUNT && currentGamePadNum < Game::MAX_GAMEPADS; currentPort++)
        {
            if (GPad_Check(currentGamePadNum, currentPort))
                currentGamePadNum++;
        }
    }

    bool Gamepad::Key_IsValidGamePadChar(const int key)
    {
        return key >= Game::K_FIRSTGAMEPADBUTTON_RANGE_1 && key <= Game::K_LASTGAMEPADBUTTON_RANGE_1
            || key >= Game::K_FIRSTGAMEPADBUTTON_RANGE_2 && key <= Game::K_LASTGAMEPADBUTTON_RANGE_2
            || key >= Game::K_FIRSTGAMEPADBUTTON_RANGE_3 && key <= Game::K_LASTGAMEPADBUTTON_RANGE_3;
    }

    void Gamepad::CL_GamepadResetMenuScrollTime(const int gamePadIndex, const int key, const bool down, const unsigned time)
    {
        assert(gamePadIndex < Game::MAX_GAMEPADS);
        auto& gamePadGlobal = gamePadGlobals[gamePadIndex];

        if (!down)
            return;

        const auto scrollDelayFirst = gpad_menu_scroll_delay_first.get<int>();
        for (const auto scrollButton : Game::menuScrollButtonList)
        {
            if (key == scrollButton)
            {
                gamePadGlobal.nextScrollTime = scrollDelayFirst + time;
                return;
            }
        }
    }

    void Gamepad::CL_GamepadGenerateAPad(const int gamePadIndex, const Game::GamepadPhysicalAxis physicalAxis, unsigned time)
    {
        assert(gamePadIndex < Game::MAX_GAMEPADS);
        assert(physicalAxis < Game::GPAD_PHYSAXIS_COUNT && physicalAxis >= 0);

        auto& gamePad = gamePads[gamePadIndex];

        const auto stick = Game::stickForAxis[physicalAxis];
        const auto stickIndex = stick & Game::GPAD_VALUE_MASK;
        if (stick != Game::GPAD_INVALID)
        {
            assert(stickIndex < 4);
            const auto& mapping = Game::analogStickList[stickIndex];

            if (gamePad.stickDown[stickIndex][Game::GPAD_STICK_POS])
            {
                const Game::GamePadButtonEvent event = gamePad.stickDownLast[stickIndex][Game::GPAD_STICK_POS] ? Game::GPAD_BUTTON_UPDATE : Game::GPAD_BUTTON_PRESSED;
                CL_GamepadButtonEvent(gamePadIndex, mapping.posCode, event, time, Game::GPAD_NONE);
            }
            else if (gamePad.stickDown[stickIndex][Game::GPAD_STICK_NEG])
            {
                const Game::GamePadButtonEvent event = gamePad.stickDownLast[stickIndex][Game::GPAD_STICK_NEG] ? Game::GPAD_BUTTON_UPDATE : Game::GPAD_BUTTON_PRESSED;
                CL_GamepadButtonEvent(gamePadIndex, mapping.negCode, event, time, Game::GPAD_NONE);
            }
            else if (gamePad.stickDownLast[stickIndex][Game::GPAD_STICK_POS])
            {
                CL_GamepadButtonEvent(gamePadIndex, mapping.posCode, Game::GPAD_BUTTON_RELEASED, time, Game::GPAD_NONE);
            }
            else if (gamePad.stickDownLast[stickIndex][Game::GPAD_STICK_NEG])
            {
                CL_GamepadButtonEvent(gamePadIndex, mapping.negCode, Game::GPAD_BUTTON_RELEASED, time, Game::GPAD_NONE);
            }
        }
    }

    void Gamepad::CL_GamepadEvent(const int gamePadIndex, const Game::GamepadPhysicalAxis physicalAxis, const float value, const unsigned time)
    {
        assert(gamePadIndex < Game::MAX_GAMEPADS);
        assert(physicalAxis < Game::GPAD_PHYSAXIS_COUNT && physicalAxis >= 0);

        auto& gamePad = gamePads[gamePadIndex];
        auto& gamePadGlobal = gamePadGlobals[gamePadIndex];

        gamePadGlobal.axes.axesValues[physicalAxis] = value;
        CL_GamepadGenerateAPad(gamePadIndex, physicalAxis, time);

        if (std::fabs(value) > 0.0f)
        {
            gamePad.inUse = true;
            gpad_in_use.setRaw(true);
        }
    }

    void Gamepad::UI_GamepadKeyEvent(const int gamePadIndex, const int key, const bool down)
    {
        for (const auto& mapping : controllerMenuKeyMappings)
        {
            if (mapping.controllerKey == key)
            {
                Game::UI_KeyEvent(gamePadIndex, mapping.pcKey, down);
                return;
            }
        }

        // No point in sending unmapped controller keystrokes to the key event handler since it doesn't know how to use it anyway
        // Game::UI_KeyEvent(gamePadIndex, key, down);
    }

    bool Gamepad::CL_CheckForIgnoreDueToRepeat(const int gamePadIndex, const int key, const int repeatCount, const unsigned time)
    {
        assert(gamePadIndex < Game::MAX_GAMEPADS);
        auto& gamePadGlobal = gamePadGlobals[gamePadIndex];

        if (Game::Key_IsKeyCatcherActive(gamePadIndex, Game::KEYCATCH_UI))
        {
            const int scrollDelayFirst = gpad_menu_scroll_delay_first.get<int>();
            const int scrollDelayRest = gpad_menu_scroll_delay_rest.get<int>();

            for (const auto menuScrollButton : Game::menuScrollButtonList)
            {
                if (key == menuScrollButton)
                {
                    if (repeatCount == 1)
                    {
                        gamePadGlobal.nextScrollTime = time + scrollDelayFirst;
                        return false;
                    }

                    if (time > gamePadGlobal.nextScrollTime)
                    {
                        gamePadGlobal.nextScrollTime = time + scrollDelayRest;
                        return false;
                    }
                    break;
                }
            }
        }

        return repeatCount > 1;
    }

    void Gamepad::CL_GamepadButtonEvent(const int gamePadIndex, const int key, const Game::GamePadButtonEvent buttonEvent, const unsigned time, const Game::GamePadButton button)
    {
        assert(gamePadIndex < Game::MAX_GAMEPADS);

        const auto pressed = buttonEvent == Game::GPAD_BUTTON_PRESSED;
        const auto pressedOrUpdated = pressed || buttonEvent == Game::GPAD_BUTTON_UPDATE;

        auto& keyState = Game::playerKeys[gamePadIndex];
        keyState.keys[key].down = pressedOrUpdated;

        if (pressedOrUpdated)
        {
            if (++keyState.keys[key].repeats == 1)
                keyState.anyKeyDown++;
        }
        else if (buttonEvent == Game::GPAD_BUTTON_RELEASED && keyState.keys[key].repeats > 0)
        {
            keyState.keys[key].repeats = 0;
            if (--keyState.anyKeyDown < 0)
                keyState.anyKeyDown = 0;
        }

        if (pressedOrUpdated && CL_CheckForIgnoreDueToRepeat(gamePadIndex, key, keyState.keys[key].repeats, time))
            return;

        if (Game::Key_IsKeyCatcherActive(gamePadIndex, Game::KEYCATCH_LOCATION_SELECTION) && pressedOrUpdated)
        {
            if (key == Game::K_BUTTON_B || keyState.keys[key].binding && strcmp(keyState.keys[key].binding, "+actionslot 4") == 0)
            {
                keyState.locSelInputState = Game::LOC_SEL_INPUT_CANCEL;
            }
            else if (key == Game::K_BUTTON_A || keyState.keys[key].binding && strcmp(keyState.keys[key].binding, "+attack") == 0)
            {
                keyState.locSelInputState = Game::LOC_SEL_INPUT_CONFIRM;
            }
            return;
        }

        keyState.locSelInputState = Game::LOC_SEL_INPUT_NONE;

        const auto* keyBinding = keyState.keys[key].binding;

        char cmd[1024];
        if (pressedOrUpdated)
        {
            if (Game::Key_IsKeyCatcherActive(gamePadIndex, Game::KEYCATCH_UI))
            {
                UI_GamepadKeyEvent(gamePadIndex, key, pressedOrUpdated);
                return;
            }

            if (keyBinding)
            {
                if (keyBinding[0] == '+')
                {
                    float floatValue;
                    if (button)
                        floatValue = GPad_GetButton(gamePadIndex, button);
                    else
                        floatValue = 0.0f;

                    sprintf_s(cmd, "%s %i %i %0.3f\n", keyBinding, key, time, floatValue);
                    Game::Cbuf_AddText(gamePadIndex, cmd);
                }
                else
                {
                    Game::Cbuf_AddText(gamePadIndex, keyBinding);
                    Game::Cbuf_AddText(gamePadIndex, "\n");
                }
            }
        }
        else
        {
            if (keyBinding && keyBinding[0] == '+')
            {
                float floatValue;
                if (button)
                    floatValue = GPad_GetButton(gamePadIndex, button);
                else
                    floatValue = 0.0f;

                sprintf_s(cmd, "-%s %i %i %0.3f\n", &keyBinding[1], key, time, floatValue);
                Game::Cbuf_AddText(gamePadIndex, cmd);
            }

            if (Game::Key_IsKeyCatcherActive(gamePadIndex, Game::KEYCATCH_UI))
            {
                UI_GamepadKeyEvent(gamePadIndex, key, pressedOrUpdated);
            }
        }
    }

    void Gamepad::CL_GamepadButtonEventForPort(const int gamePadIndex, const int key, const Game::GamePadButtonEvent buttonEvent, const unsigned time, const Game::GamePadButton button)
    {
        assert(gamePadIndex < Game::MAX_GAMEPADS);
        auto& gamePad = gamePads[gamePadIndex];

        gamePad.inUse = true;
        gpad_in_use.setRaw(true);

        if (Game::Key_IsKeyCatcherActive(gamePadIndex, Game::KEYCATCH_UI))
            CL_GamepadResetMenuScrollTime(gamePadIndex, key, buttonEvent == Game::GPAD_BUTTON_PRESSED, time);


        CL_GamepadButtonEvent(gamePadIndex, key, buttonEvent, time, button);
    }

    void Gamepad::GPad_ConvertStickToFloat(const short x, const short y, float& outX, float& outY)
    {
        Game::vec2_t stickVec;
        stickVec[0] = static_cast<float>(x) / static_cast<float>(std::numeric_limits<short>::max());
        stickVec[1] = static_cast<float>(y) / static_cast<float>(std::numeric_limits<short>::max());

        const auto deadZoneTotal = gpad_stick_deadzone_min.get<float>() + gpad_stick_deadzone_max.get<float>();
        auto len = Game::Vec2Normalize(stickVec);

        if (gpad_stick_deadzone_min.get<float>() <= len)
        {
            if (1.0f - gpad_stick_deadzone_max.get<float>() >= len)
                len = (len - gpad_stick_deadzone_min.get<float>()) / (1.0f - deadZoneTotal);
            else
                len = 1.0f;
        }
        else
            len = 0.0f;

        outX = stickVec[0] * len;
        outY = stickVec[1] * len;
    }

    float Gamepad::GPad_GetStick(const int gamePadIndex, const Game::GamePadStick stick)
    {
        assert(gamePadIndex < Game::MAX_GAMEPADS);
        auto& gamePad = gamePads[gamePadIndex];

        return gamePad.sticks[stick];
    }

    float Gamepad::GPad_GetButton(const int gamePadIndex, Game::GamePadButton button)
    {
        assert(gamePadIndex < Game::MAX_GAMEPADS);
        auto& gamePad = gamePads[gamePadIndex];

        float value = 0.0f;

        if (button & Game::GPAD_DIGITAL_MASK)
        {
            const auto buttonValue = button & Game::GPAD_VALUE_MASK;
            value = buttonValue & gamePad.digitals ? 1.0f : 0.0f;
        }
        else if (button & Game::GPAD_ANALOG_MASK)
        {
            const auto analogIndex = button & Game::GPAD_VALUE_MASK;
            if (analogIndex < std::extent_v<decltype(gamePad.analogs)>)
            {
                value = gamePad.analogs[analogIndex];
            }
        }

        return value;
    }

    bool Gamepad::GPad_IsButtonPressed(const int gamePadIndex, Game::GamePadButton button)
    {
        assert(gamePadIndex < Game::MAX_GAMEPADS);
        auto& gamePad = gamePads[gamePadIndex];

        bool down = false;
        bool lastDown = false;

        if (button & Game::GPAD_DIGITAL_MASK)
        {
            const auto buttonValue = button & Game::GPAD_VALUE_MASK;
            down = (buttonValue & gamePad.digitals) != 0;
            lastDown = (buttonValue & gamePad.lastDigitals) != 0;
        }
        else if (button & Game::GPAD_ANALOG_MASK)
        {
            const auto analogIndex = button & Game::GPAD_VALUE_MASK;
            assert(analogIndex < std::extent_v<decltype(gamePad.analogs)>);

            if (analogIndex < std::extent_v<decltype(gamePad.analogs)>)
            {
                down = gamePad.analogs[analogIndex] > 0.0f;
                lastDown = gamePad.lastAnalogs[analogIndex] > 0.0f;
            }
        }

        return down && !lastDown;
    }

    bool Gamepad::GPad_ButtonRequiresUpdates(const int gamePadIndex, Game::GamePadButton button)
    {
        return (button & Game::GPAD_ANALOG_MASK || button & Game::GPAD_DPAD_MASK) && GPad_GetButton(gamePadIndex, button) > 0.0f;
    }

    bool Gamepad::GPad_IsButtonReleased(int gamePadIndex, Game::GamePadButton button)
    {
        assert(gamePadIndex < Game::MAX_GAMEPADS);
        auto& gamePad = gamePads[gamePadIndex];

        bool down = false;
        bool lastDown = false;

        if (button & Game::GPAD_DIGITAL_MASK)
        {
            const auto buttonValue = button & Game::GPAD_VALUE_MASK;

            down = (gamePad.digitals & buttonValue) != 0;
            lastDown = (gamePad.lastDigitals & buttonValue) != 0;
        }
        else if (button & Game::GPAD_ANALOG_MASK)
        {
            const auto analogIndex = button & Game::GPAD_VALUE_MASK;
            assert(analogIndex < std::extent_v<decltype(gamePad.analogs)>);

            if (analogIndex < std::extent_v<decltype(gamePad.analogs)>)
            {
                down = gamePad.analogs[analogIndex] > 0.0f;
                lastDown = gamePad.lastAnalogs[analogIndex] > 0.0f;
            }
        }

        return !down && lastDown;
    }

    void Gamepad::GPad_UpdateSticksDown(const int gamePadIndex)
    {
        assert(gamePadIndex < Game::MAX_GAMEPADS);
        auto& gamePad = gamePads[gamePadIndex];

        for (auto stickIndex = 0u; stickIndex < std::extent_v<decltype(GamePad::sticks)>; stickIndex++)
        {
            for (auto dir = 0; dir < Game::GPAD_STICK_DIR_COUNT; dir++)
            {
                gamePad.stickDownLast[stickIndex][dir] = gamePad.stickDown[stickIndex][dir];

                auto threshold = gpad_stick_pressed.get<float>();

                if (gamePad.stickDownLast[stickIndex][dir])
                    threshold -= gpad_stick_pressed_hysteresis.get<float>();
                else
                    threshold += gpad_stick_pressed_hysteresis.get<float>();

                if (dir == Game::GPAD_STICK_POS)
                {
                    gamePad.stickDown[stickIndex][dir] = gamePad.sticks[stickIndex] > threshold;
                }
                else
                {
                    assert(dir == Game::GPAD_STICK_NEG);
                    gamePad.stickDown[stickIndex][dir] = gamePad.sticks[stickIndex] < -threshold;
                }
            }
        }
    }

    void Gamepad::GPad_UpdateSticks(const int gamePadIndex, const XINPUT_GAMEPAD& state)
    {
        assert(gamePadIndex < Game::MAX_GAMEPADS);

        auto& gamePad = gamePads[gamePadIndex];

        Game::vec2_t lVec, rVec;
        GPad_ConvertStickToFloat(state.sThumbLX, state.sThumbLY, lVec[0], lVec[1]);
        GPad_ConvertStickToFloat(state.sThumbRX, state.sThumbRY, rVec[0], rVec[1]);

        gamePad.lastSticks[0] = gamePad.sticks[0];
        gamePad.sticks[0] = lVec[0];
        gamePad.lastSticks[1] = gamePad.sticks[1];
        gamePad.sticks[1] = lVec[1];
        gamePad.lastSticks[2] = gamePad.sticks[2];
        gamePad.sticks[2] = rVec[0];
        gamePad.lastSticks[3] = gamePad.sticks[3];
        gamePad.sticks[3] = rVec[1];

        GPad_UpdateSticksDown(gamePadIndex);

#ifdef DEBUG
        if (gpad_debug.get<bool>())
        {
            Logger::Print("Left: X: %f Y: %f\n", lVec[0], lVec[1]);
            Logger::Print("Right: X: %f Y: %f\n", rVec[0], rVec[1]);
            Logger::Print("Down: %i:%i %i:%i %i:%i %i:%i\n", gamePad.stickDown[0][Game::GPAD_STICK_POS], gamePad.stickDown[0][Game::GPAD_STICK_NEG],
                          gamePad.stickDown[1][Game::GPAD_STICK_POS], gamePad.stickDown[1][Game::GPAD_STICK_NEG],
                          gamePad.stickDown[2][Game::GPAD_STICK_POS], gamePad.stickDown[2][Game::GPAD_STICK_NEG],
                          gamePad.stickDown[3][Game::GPAD_STICK_POS], gamePad.stickDown[3][Game::GPAD_STICK_NEG]);
        }
#endif
    }

    void Gamepad::GPad_UpdateDigitals(const int gamePadIndex, const XINPUT_GAMEPAD& state)
    {
        assert(gamePadIndex < Game::MAX_GAMEPADS);

        auto& gamePad = gamePads[gamePadIndex];

        gamePad.lastDigitals = gamePad.digitals;
        gamePad.digitals = state.wButtons;

        const auto leftDeflect = gpad_button_lstick_deflect_max.get<float>();
        if (std::fabs(gamePad.sticks[0]) > leftDeflect || std::fabs(gamePad.sticks[1]) > leftDeflect)
            gamePad.digitals &= ~static_cast<short>(XINPUT_GAMEPAD_LEFT_THUMB);
        const auto rightDeflect = gpad_button_rstick_deflect_max.get<float>();
        if (std::fabs(gamePad.sticks[2]) > leftDeflect || std::fabs(gamePad.sticks[3]) > rightDeflect)
            gamePad.digitals &= ~static_cast<short>(XINPUT_GAMEPAD_RIGHT_THUMB);

#ifdef DEBUG
        if (gpad_debug.get<bool>())
        {
            Logger::Print("Buttons: %x\n", gamePad.digitals);
        }
#endif
    }

    void Gamepad::GPad_UpdateAnalogs(const int gamePadIndex, const XINPUT_GAMEPAD& state)
    {
        assert(gamePadIndex < Game::MAX_GAMEPADS);

        auto& gamePad = gamePads[gamePadIndex];

        const auto buttonDeadZone = gpad_button_deadzone.get<float>();

        gamePad.lastAnalogs[0] = gamePad.analogs[0];
        gamePad.analogs[0] = static_cast<float>(state.bLeftTrigger) / static_cast<float>(std::numeric_limits<unsigned char>::max());
        if (gamePad.analogs[0] < buttonDeadZone)
            gamePad.analogs[0] = 0.0f;


        gamePad.lastAnalogs[1] = gamePad.analogs[1];
        gamePad.analogs[1] = static_cast<float>(state.bRightTrigger) / static_cast<float>(std::numeric_limits<unsigned char>::max());
        if (gamePad.analogs[1] < buttonDeadZone)
            gamePad.analogs[1] = 0.0f;

#ifdef DEBUG
        if (gpad_debug.get<bool>())
        {
            Logger::Print("Triggers: %f %f\n", gamePad.analogs[0], gamePad.analogs[1]);
        }
#endif
    }

    void Gamepad::GPad_UpdateAll()
    {
        GPad_RefreshAll();

        for (auto currentGamePadIndex = 0; currentGamePadIndex < Game::MAX_GAMEPADS; currentGamePadIndex++)
        {
            const auto& gamePad = gamePads[currentGamePadIndex];
            if (!gamePad.enabled)
                continue;

            XINPUT_STATE inputState;
            if (XInputGetState(gamePad.portIndex, &inputState) != ERROR_SUCCESS)
                continue;

            GPad_UpdateSticks(currentGamePadIndex, inputState.Gamepad);
            GPad_UpdateDigitals(currentGamePadIndex, inputState.Gamepad);
            GPad_UpdateAnalogs(currentGamePadIndex, inputState.Gamepad);
        }
    }

    void Gamepad::IN_GamePadsMove()
    {
        GPad_UpdateAll();
        const auto time = Game::Sys_Milliseconds();

        bool gpadPresent = false;
        for (auto gamePadIndex = 0; gamePadIndex < Game::MAX_GAMEPADS; gamePadIndex++)
        {
            const auto& gamePad = gamePads[gamePadIndex];

            if (gamePad.enabled)
            {
                gpadPresent = true;
                const auto lx = GPad_GetStick(gamePadIndex, Game::GPAD_LX);
                const auto ly = GPad_GetStick(gamePadIndex, Game::GPAD_LY);
                const auto rx = GPad_GetStick(gamePadIndex, Game::GPAD_RX);
                const auto ry = GPad_GetStick(gamePadIndex, Game::GPAD_RY);
                const auto leftTrig = GPad_GetButton(gamePadIndex, Game::GPAD_L_TRIG);
                const auto rightTrig = GPad_GetButton(gamePadIndex, Game::GPAD_R_TRIG);

                CL_GamepadEvent(gamePadIndex, Game::GPAD_PHYSAXIS_LSTICK_X, lx, time);
                CL_GamepadEvent(gamePadIndex, Game::GPAD_PHYSAXIS_LSTICK_Y, ly, time);
                CL_GamepadEvent(gamePadIndex, Game::GPAD_PHYSAXIS_RSTICK_X, rx, time);
                CL_GamepadEvent(gamePadIndex, Game::GPAD_PHYSAXIS_RSTICK_Y, ry, time);
                CL_GamepadEvent(gamePadIndex, Game::GPAD_PHYSAXIS_LTRIGGER, leftTrig, time);
                CL_GamepadEvent(gamePadIndex, Game::GPAD_PHYSAXIS_RTRIGGER, rightTrig, time);

                for (const auto& buttonMapping : Game::buttonList)
                {
                    if (GPad_IsButtonPressed(gamePadIndex, buttonMapping.padButton))
                    {
                        CL_GamepadButtonEventForPort(
                            gamePadIndex,
                            buttonMapping.code,
                            Game::GPAD_BUTTON_PRESSED,
                            time,
                            buttonMapping.padButton);
                    }
                    else if (GPad_ButtonRequiresUpdates(gamePadIndex, buttonMapping.padButton))
                    {
                        CL_GamepadButtonEventForPort(
                            gamePadIndex,
                            buttonMapping.code,
                            Game::GPAD_BUTTON_UPDATE,
                            time,
                            buttonMapping.padButton);
                    }
                    else if (GPad_IsButtonReleased(gamePadIndex, buttonMapping.padButton))
                    {
                        CL_GamepadButtonEventForPort(
                            gamePadIndex,
                            buttonMapping.code,
                            Game::GPAD_BUTTON_RELEASED,
                            time,
                            buttonMapping.padButton);
                    }
                }
            }
        }

        gpad_present.setRaw(gpadPresent);
    }


    void Gamepad::IN_Frame_Hk()
    {
        // Call original method
        Utils::Hook::Call<void()>(0x64C490)();

        IN_GamePadsMove();
    }

    void Gamepad::Gamepad_WriteBindings(const int gamePadIndex, const int handle)
    {
        assert(gamePadIndex < Game::MAX_GAMEPADS);
        auto& gamePadGlobal = gamePadGlobals[gamePadIndex];

        Game::FS_Printf(handle, "unbindallaxis\n");

        for(auto virtualAxisIndex = 0u; virtualAxisIndex < Game::GPAD_VIRTAXIS_COUNT; virtualAxisIndex++)
        {
            const auto& axisMapping = gamePadGlobal.axes.virtualAxes[virtualAxisIndex];
            if (axisMapping.physicalAxis <= Game::GPAD_PHYSAXIS_NONE || axisMapping.physicalAxis >= Game::GPAD_PHYSAXIS_COUNT
                || axisMapping.mapType <= Game::GPAD_MAP_NONE || axisMapping.mapType >= Game::GPAD_MAP_COUNT)
            {
                continue;
            }

            const auto* physicalAxisName = Game::physicalAxisNames[axisMapping.physicalAxis];
            const auto* virtualAxisName = Game::virtualAxisNames[virtualAxisIndex];
            const auto* mappingName = Game::gamePadMappingTypeNames[axisMapping.mapType];

            Game::FS_Printf(handle, "bindaxis %s %s %s\n", physicalAxisName, virtualAxisName, mappingName);
        }
    }

    void Gamepad::Key_WriteBindings_Hk(const int localClientNum, const int handle)
    {
        // Call original function
        Utils::Hook::Call<void(int, int)>(0x4A5A20)(localClientNum, handle);

        Gamepad_WriteBindings(0, handle);
    }

    void __declspec(naked) Gamepad::Com_WriteConfiguration_Modified_Stub()
    {
        __asm
        {
            mov eax, [ecx + 0x18]
            or eax, gamePadBindingsModifiedFlags // Also check for gamePadBindingsModifiedFlags
            test al, 1
            jz endMethod
            mov gamePadBindingsModifiedFlags, 0 // Reset gamePadBindingsModifiedFlags
            mov eax, [ecx + 0x18] // Restore eax to dvar_modified_flags

            push 0x60B26E
            retn

        endMethod:
            push 0x60B298
            retn
        }
    }


    void Gamepad::Gamepad_BindAxis(const int gamePadIndex, const Game::GamepadPhysicalAxis realIndex, const Game::GamepadVirtualAxis axisIndex, const Game::GamepadMapping mapType)
    {
        assert(gamePadIndex < Game::MAX_GAMEPADS);
        assert(realIndex > Game::GPAD_PHYSAXIS_NONE && realIndex < Game::GPAD_PHYSAXIS_COUNT);
        assert(axisIndex > Game::GPAD_VIRTAXIS_NONE && axisIndex < Game::GPAD_VIRTAXIS_COUNT);
        assert(mapType > Game::GPAD_MAP_NONE && mapType < Game::GPAD_MAP_COUNT);

        auto& gamePadGlobal = gamePadGlobals[gamePadIndex];
        gamePadGlobal.axes.virtualAxes[axisIndex].physicalAxis = realIndex;
        gamePadGlobal.axes.virtualAxes[axisIndex].mapType = mapType;

        gamePadBindingsModifiedFlags |= 1;
    }

    Game::GamepadPhysicalAxis Gamepad::StringToPhysicalAxis(const char* str)
    {
        for (auto i = 0u; i < std::extent_v<decltype(Game::physicalAxisNames)>; i++)
        {
            if (strcmp(str, Game::physicalAxisNames[i]) == 0)
                return static_cast<Game::GamepadPhysicalAxis>(i);
        }

        return Game::GPAD_PHYSAXIS_NONE;
    }

    Game::GamepadVirtualAxis Gamepad::StringToVirtualAxis(const char* str)
    {
        for (auto i = 0u; i < std::extent_v<decltype(Game::virtualAxisNames)>; i++)
        {
            if (strcmp(str, Game::virtualAxisNames[i]) == 0)
                return static_cast<Game::GamepadVirtualAxis>(i);
        }

        return Game::GPAD_VIRTAXIS_NONE;
    }

    Game::GamepadMapping Gamepad::StringToGamePadMapping(const char* str)
    {
        for (auto i = 0u; i < std::extent_v<decltype(Game::gamePadMappingTypeNames)>; i++)
        {
            if (strcmp(str, Game::gamePadMappingTypeNames[i]) == 0)
                return static_cast<Game::GamepadMapping>(i);
        }

        return Game::GPAD_MAP_NONE;
    }

    void Gamepad::Axis_Bind_f(Command::Params* params)
    {
        if (params->length() < 4)
        {
            Logger::Print("bindaxis <real axis> <virtual axis> <input type>\n");
            return;
        }

        const auto* physicalAxisText = params->get(1);
        const auto* virtualAxisText = params->get(2);
        const auto* mappingText = params->get(3);

        const Game::GamepadPhysicalAxis physicalAxis = StringToPhysicalAxis(physicalAxisText);
        if (physicalAxis == Game::GPAD_PHYSAXIS_NONE)
        {
            Logger::Print("\"%s\" isn't a valid physical axis\n", physicalAxisText);
            return;
        }

        const Game::GamepadVirtualAxis virtualAxis = StringToVirtualAxis(virtualAxisText);
        if (virtualAxis == Game::GPAD_VIRTAXIS_NONE)
        {
            Logger::Print("\"%s\" isn't a valid virtual axis\n", virtualAxisText);
            return;
        }

        const Game::GamepadMapping mapping = StringToGamePadMapping(mappingText);
        if (mapping == Game::GPAD_MAP_NONE)
        {
            Logger::Print("\"%s\" isn't a valid input type\n", mappingText);
            return;
        }

        Gamepad_BindAxis(0, physicalAxis, virtualAxis, mapping);
    }

    void Gamepad::Axis_Unbindall_f(Command::Params*)
    {
        auto& gamePadGlobal = gamePadGlobals[0];

        for (auto& virtualAxis : gamePadGlobal.axes.virtualAxes)
        {
            virtualAxis.physicalAxis = Game::GPAD_PHYSAXIS_NONE;
            virtualAxis.mapType = Game::GPAD_MAP_NONE;
        }
    }

    void Gamepad::Bind_GP_SticksConfigs_f(Command::Params* params)
    {
        const auto* stickConfigName = gpad_sticksConfig.get<const char*>();
        Game::Cbuf_AddText(0, Utils::String::VA("exec %s\n", stickConfigName));
    }

    void Gamepad::Bind_GP_ButtonsConfigs_f(Command::Params* params)
    {
        const auto* buttonConfigName = gpad_buttonConfig.get<const char*>();
        Game::Cbuf_AddText(0, Utils::String::VA("exec %s\n", buttonConfigName));
    }

    void Gamepad::InitDvars()
    {
        gpad_enabled = Dvar::Register<bool>("gpad_enabled", false, Game::DVAR_FLAG_SAVED, "Game pad enabled");
        gpad_debug = Dvar::Register<bool>("gpad_debug", false, 0, "Game pad debugging");
        gpad_present = Dvar::Register<bool>("gpad_present", false, 0, "Game pad present");
        gpad_in_use = Dvar::Register<bool>("gpad_in_use", false, 0, "A game pad is in use");
        gpad_sticksConfig = Dvar::Register<const char*>("gpad_sticksConfig", "thumbstick_default", Game::DVAR_FLAG_SAVED, "Game pad stick configuration");
        gpad_buttonConfig = Dvar::Register<const char*>("gpad_buttonConfig", "buttons_default", Game::DVAR_FLAG_SAVED, "Game pad button configuration");
        gpad_menu_scroll_delay_first = Dvar::Register<int>("gpad_menu_scroll_delay_first", 420, 0, 1000, Game::DVAR_FLAG_SAVED, "Menu scroll key-repeat delay, for the first repeat, in milliseconds");
        gpad_menu_scroll_delay_rest = Dvar::Register<int>("gpad_menu_scroll_delay_rest", 210, 0, 1000, Game::DVAR_FLAG_SAVED,
                                                          "Menu scroll key-repeat delay, for repeats after the first, in milliseconds");
        gpad_rumble = Dvar::Register<bool>("gpad_rumble", true, Game::DVAR_FLAG_SAVED, "Enable game pad rumble");
        gpad_stick_pressed_hysteresis = Dvar::Register<float>("gpad_stick_pressed_hysteresis", 0.1f, 0.0f, 1.0f, 0,
                                                              "Game pad stick pressed no-change-zone around gpad_stick_pressed to prevent bouncing");
        gpad_stick_pressed = Dvar::Register<float>("gpad_stick_pressed", 0.4f, 0.0, 1.0, 0, "Game pad stick pressed threshhold");
        gpad_stick_deadzone_max = Dvar::Register<float>("gpad_stick_deadzone_max", 0.01f, 0.0f, 1.0f, 0, "Game pad maximum stick deadzone");
        gpad_stick_deadzone_min = Dvar::Register<float>("gpad_stick_deadzone_min", 0.2f, 0.0f, 1.0f, 0, "Game pad minimum stick deadzone");
        gpad_button_deadzone = Dvar::Register<float>("gpad_button_deadzone", 0.13f, 0.0f, 1.0f, 0, "Game pad button deadzone threshhold");
        gpad_button_lstick_deflect_max = Dvar::Register<float>("gpad_button_lstick_deflect_max", 1.0f, 0.0f, 1.0f, 0, "Game pad maximum pad stick pressed value");
        gpad_button_rstick_deflect_max = Dvar::Register<float>("gpad_button_rstick_deflect_max", 1.0f, 0.0f, 1.0f, 0, "Game pad maximum pad stick pressed value");
    }

    void Gamepad::IN_Init_Hk()
    {
        // Call original method
        Utils::Hook::Call<void()>(0x45D620)();

        InitDvars();
    }

    int Gamepad::Key_GetCommandAssignmentInternal_Hk(const char* cmd, int (*keys)[2])
    {
        auto keyCount = 0;

        if (gamePads[0].inUse)
        {
            for (auto keyNum = 0; keyNum < Game::K_LAST_KEY; keyNum++)
            {
                if(!Key_IsValidGamePadChar(keyNum))
                    continue;

                if(Game::playerKeys[0].keys[keyNum].binding && strcmp(Game::playerKeys[0].keys[keyNum].binding, cmd) == 0)
                {
                    (*keys)[keyCount++] = keyNum;

                    if (keyCount >= 2)
                        return keyNum;
                }
            }
        }
        else
        {
            for (auto keyNum = 0; keyNum < Game::K_LAST_KEY; keyNum++)
            {
                if (Key_IsValidGamePadChar(keyNum))
                    continue;

                if (Game::playerKeys[0].keys[keyNum].binding && strcmp(Game::playerKeys[0].keys[keyNum].binding, cmd) == 0)
                {
                    (*keys)[keyCount++] = keyNum;

                    if (keyCount >= 2)
                        return keyNum;
                }
            }
        }

        return keyCount;
    }

    void Gamepad::CL_KeyEvent_Hk(const int localClientNum, const int key, const int down, const unsigned time)
    {
        // A keyboard key has been pressed. Mark controller as unused.
        gamePads[0].inUse = false;
        gpad_in_use.setRaw(false);

        // Call original function
        Utils::Hook::Call<void(int, int, int, unsigned)>(0x4F6480)(localClientNum, key, down, time);
    }

    void Gamepad::CreateKeyNameMap()
    {
        memcpy(Game::combinedKeyNames, Game::vanillaKeyNames, sizeof(Game::keyname_t) * Game::VANILLA_KEY_NAME_COUNT);
        memcpy(&Game::combinedKeyNames[Game::VANILLA_KEY_NAME_COUNT], Game::extendedKeyNames, sizeof(Game::keyname_t) * std::extent_v<decltype(Game::extendedKeyNames)>);
        Game::combinedKeyNames[std::extent_v<decltype(Game::combinedKeyNames)> -1] = { nullptr, 0 };

        memcpy(Game::combinedLocalizedKeyNames, Game::vanillaLocalizedKeyNames, sizeof(Game::keyname_t) * Game::VANILLA_LOCALIZED_KEY_NAME_COUNT);
        memcpy(&Game::combinedLocalizedKeyNames[Game::VANILLA_LOCALIZED_KEY_NAME_COUNT], Game::extendedLocalizedKeyNames, sizeof(Game::keyname_t) * std::extent_v<decltype(Game::extendedLocalizedKeyNames)>);
        Game::combinedLocalizedKeyNames[std::extent_v<decltype(Game::combinedLocalizedKeyNames)> -1] = { nullptr, 0 };

        Utils::Hook::Set<Game::keyname_t*>(0x4A780A, Game::combinedKeyNames);
        Utils::Hook::Set<Game::keyname_t*>(0x4A7810, Game::combinedKeyNames);
        Utils::Hook::Set<Game::keyname_t*>(0x435C9F, Game::combinedKeyNames);
        Utils::Hook::Set<Game::keyname_t*>(0x435C98, Game::combinedLocalizedKeyNames);
    }

    Gamepad::Gamepad()
    {
        if (ZoneBuilder::IsEnabled())
            return;

        // use the xinput state when creating a usercmd
        //Utils::Hook(0x5A6DB9, CL_CreateCmdStub, HOOK_JUMP).install()->quick();

        // package the forward and right move components in the move buttons
        Utils::Hook(0x60E38D, MSG_WriteDeltaUsercmdKeyStub, HOOK_JUMP).install()->quick();

        // send two bytes for sending movement data
        Utils::Hook::Set<BYTE>(0x60E501, 16);
        Utils::Hook::Set<BYTE>(0x60E5CD, 16);

        // make sure to parse the movement data properly and apply it
        Utils::Hook(0x492127, MSG_ReadDeltaUsercmdKeyStub, HOOK_JUMP).install()->quick();
        Utils::Hook(0x492009, MSG_ReadDeltaUsercmdKeyStub2, HOOK_JUMP).install()->quick();

        // Also rewrite configuration when gamepad config is dirty
        Utils::Hook(0x60B264, Com_WriteConfiguration_Modified_Stub, HOOK_JUMP).install()->quick();

        Utils::Hook(0x60B223, Key_WriteBindings_Hk, HOOK_CALL).install()->quick();

        CreateKeyNameMap();

        Command::Add("bindaxis", Axis_Bind_f);
        Command::Add("unbindallaxis", Axis_Unbindall_f);
        Command::Add("bindgpsticksconfigs", Bind_GP_SticksConfigs_f);
        Command::Add("bindgpbuttonsconfigs", Bind_GP_ButtonsConfigs_f);

        if (Dedicated::IsEnabled())
            return;

        Utils::Hook(0x467C03, IN_Init_Hk, HOOK_CALL).install()->quick();

        Utils::Hook(0x475E9E, IN_Frame_Hk, HOOK_CALL).install()->quick();

        // Mark controller as unused when keyboard key is pressed
        Utils::Hook(0x43D179, CL_KeyEvent_Hk, HOOK_CALL).install()->quick();

        // Only return gamepad keys when gamepad enabled and only non gamepad keys when not
        Utils::Hook(0x5A7A23, Key_GetCommandAssignmentInternal_Hk, HOOK_CALL).install()->quick();

        //Utils::Hook(0x5A617D, CL_GetMouseMovementStub, HOOK_CALL).install()->quick();
        //Utils::Hook(0x5A6816, CL_GetMouseMovementStub, HOOK_CALL).install()->quick();
        //Utils::Hook(0x5A6829, unk_CheckKeyHook, HOOK_CALL).install()->quick();

        xpadSensitivity = Dvar::Register<float>("xpad_sensitivity", 1.9f, 0.1f, 10.0f, Game::DVAR_FLAG_SAVED, "View sensitivity for XInput-compatible gamepads");
        xpadEarlyTime = Dvar::Register<int>("xpad_early_time", 130, 0, 1000, Game::DVAR_FLAG_SAVED, "Time (in milliseconds) of reduced view sensitivity");
        xpadEarlyMultiplier = Dvar::Register<float>("xpad_early_multiplier", 0.25f, 0.01f, 1.0f, Game::DVAR_FLAG_SAVED,
                                                    "By how much the view sensitivity is multiplied during xpad_early_time when moving the view stick");
        xpadHorizontalMultiplier = Dvar::Register<float>("xpad_horizontal_multiplier", 1.5f, 1.0f, 20.0f, Game::DVAR_FLAG_SAVED, "Horizontal view sensitivity multiplier");
        xpadVerticalMultiplier = Dvar::Register<float>("xpad_vertical_multiplier", 0.8f, 1.0f, 20.0f, Game::DVAR_FLAG_SAVED, "Vertical view sensitivity multiplier");
        xpadAdsMultiplier = Dvar::Register<float>("xpad_ads_multiplier", 0.7f, 0.1f, 1.0f, Game::DVAR_FLAG_SAVED, "By how much the view sensitivity is multiplied when aiming down the sights.");
    }
}
