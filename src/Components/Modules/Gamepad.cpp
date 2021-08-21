#include "STDInclude.hpp"

#include <limits>

#define XINPUT_SENSITIVITY_MULTIPLIER 4 // Arbitrary value I multiply the xinput senstivity dvar with to get nicer values (0-10 range or something)
#define SIGN(d) ((d > 0) - (d < 0))

namespace Components
{
    Gamepad::GamePad Gamepad::gamePads[1]{};
    std::chrono::milliseconds Gamepad::timeAtFirstHeldMaxLookX = 0ms; // "For how much time in milliseconds has the player been holding a horizontal direction on their stick, fully" (-1.0 or 1.0)
    bool Gamepad::isHoldingMaxLookX = false;
    bool Gamepad::isADS;

    Dvar::Var Gamepad::gpad_enabled;
    Dvar::Var Gamepad::gpad_debug;
    Dvar::Var Gamepad::gpad_present;
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

    Gamepad::GamePadStickDir Gamepad::lastMenuNavigationDirection = GPAD_STICK_DIR_COUNT;
    std::chrono::milliseconds Gamepad::lastNavigationTime = 0ms;
    std::chrono::milliseconds Gamepad::msBetweenNavigations = 220ms;

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

    // Same thing
    std::vector<Gamepad::MenuMapping> menuMappings = {
        Gamepad::MenuMapping(XINPUT_GAMEPAD_A, Game::keyNum_t::K_KP_ENTER),
        Gamepad::MenuMapping(XINPUT_GAMEPAD_B, Game::keyNum_t::K_ESCAPE),
        Gamepad::MenuMapping(XINPUT_GAMEPAD_DPAD_RIGHT, Game::keyNum_t::K_KP_RIGHTARROW),
        Gamepad::MenuMapping(XINPUT_GAMEPAD_DPAD_LEFT, Game::keyNum_t::K_KP_LEFTARROW),
        Gamepad::MenuMapping(XINPUT_GAMEPAD_DPAD_UP, Game::keyNum_t::K_KP_UPARROW),
        Gamepad::MenuMapping(XINPUT_GAMEPAD_DPAD_DOWN, Game::keyNum_t::K_KP_DOWNARROW)
    };

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

    void Gamepad::MenuNavigate()
    {
        auto& gamePad = gamePads[0];
        Game::menuDef_t* menuDef = Game::Menu_GetFocused(Game::uiContext);

        if (menuDef)
        {
            if (gamePad.enabled)
            {
                std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
                std::chrono::milliseconds timeSinceLastNavigation = now - lastNavigationTime;
                bool canNavigate = timeSinceLastNavigation > msBetweenNavigations;

                if (gamePad.stickDown[1][GPAD_STICK_POS])
                {
                    if (canNavigate)
                    {
                        Game::Menu_SetPrevCursorItem(Game::uiContext, menuDef, 1);
                        lastMenuNavigationDirection = GPAD_STICK_POS;
                        lastNavigationTime = now;
                    }
                }
                else if (gamePad.stickDown[1][GPAD_STICK_NEG])
                {
                    if (canNavigate)
                    {
                        Game::Menu_SetNextCursorItem(Game::uiContext, menuDef, 1);
                        lastMenuNavigationDirection = GPAD_STICK_NEG;
                        lastNavigationTime = now;
                    }
                }
                else
                {
                    lastMenuNavigationDirection = GPAD_STICK_DIR_COUNT;
                }

                for (auto& mapping : menuMappings)
                {
                    if (mapping.wasPressed)
                    {
                        if (gamePad.digitals & mapping.input)
                        {
                            // Button still pressed, do not send info
                        }
                        else
                        {
                            mapping.wasPressed = false;
                        }
                    }
                    else if (gamePad.digitals & mapping.input)
                    {
                        Game::UI_KeyEvent(0, mapping.keystroke, 1);
                        mapping.wasPressed = true;
                    }
                }
            }
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
        Gamepad::CL_GetMouseMovementCl(clientActive, mx, my);

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
                if (!Gamepad::isHoldingMaxLookX)
                {
                    Gamepad::isHoldingMaxLookX = true;
                    Gamepad::timeAtFirstHeldMaxLookX = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
                }
                else
                {
                    std::chrono::milliseconds hasBeenHoldingLeftXForMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) -
                        Gamepad::timeAtFirstHeldMaxLookX;
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
                Gamepad::isHoldingMaxLookX = false;
                Gamepad::timeAtFirstHeldMaxLookX = 0ms;
                viewStickX *= lockedSensitivityMultiplier;
            }

            float adsMultiplier = 1.0f;

            auto ps = &clientActive->snap.ps;

            // DO NOT use clientActive->usingAds ! It only works for toggle ADS
            if (Game::PM_IsAdsAllowed(ps) && Gamepad::isADS)
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

        for (auto currentPort = 0; currentPort < XUSER_MAX_COUNT && currentGamePadNum < MAX_GAMEPADS; currentPort++)
        {
            if (GPad_Check(currentGamePadNum, currentPort))
                currentGamePadNum++;
        }
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

    void Gamepad::GPad_UpdateSticksDown(int gamePadIndex)
    {
        assert(gamePadIndex < MAX_GAMEPADS);

        auto& gamePad = gamePads[gamePadIndex];

        for (auto stickIndex = 0u; stickIndex < std::extent_v<decltype(GamePad::sticks)>; stickIndex++)
        {
            for (auto dir = 0; dir < GPAD_STICK_DIR_COUNT; dir++)
            {
                gamePad.stickDownLast[stickIndex][dir] = gamePad.stickDown[stickIndex][dir];

                auto threshold = gpad_stick_pressed.get<float>();

                if (gamePad.stickDownLast[stickIndex][dir])
                    threshold -= gpad_stick_pressed_hysteresis.get<float>();
                else
                    threshold += gpad_stick_pressed_hysteresis.get<float>();

                if (dir == GPAD_STICK_POS)
                {
                    gamePad.stickDown[stickIndex][dir] = gamePad.sticks[stickIndex] > threshold;
                }
                else
                {
                    assert(dir == GPAD_STICK_NEG);
                    gamePad.stickDown[stickIndex][dir] = gamePad.sticks[stickIndex] < -threshold;
                }
            }
        }
    }

    void Gamepad::GPad_UpdateSticks(const int gamePadIndex, const XINPUT_GAMEPAD& state)
    {
        assert(gamePadIndex < MAX_GAMEPADS);

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
            Logger::Print("Down: %i:%i %i:%i %i:%i %i:%i\n", gamePad.stickDown[0][GPAD_STICK_POS], gamePad.stickDown[0][GPAD_STICK_NEG],
                          gamePad.stickDown[1][GPAD_STICK_POS], gamePad.stickDown[1][GPAD_STICK_NEG],
                          gamePad.stickDown[2][GPAD_STICK_POS], gamePad.stickDown[2][GPAD_STICK_NEG],
                          gamePad.stickDown[3][GPAD_STICK_POS], gamePad.stickDown[3][GPAD_STICK_NEG]);
        }
#endif
    }

    void Gamepad::GPad_UpdateDigitals(const int gamePadIndex, const XINPUT_GAMEPAD& state)
    {
        assert(gamePadIndex < MAX_GAMEPADS);

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
        assert(gamePadIndex < MAX_GAMEPADS);

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

        for (auto currentGamePadIndex = 0; currentGamePadIndex < MAX_GAMEPADS; currentGamePadIndex++)
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
    }


    void Gamepad::IN_Frame_Hk()
    {
        // Call original method
        Utils::Hook::Call<void()>(0x64C490)();

        IN_GamePadsMove();
    }

    void Gamepad::InitDvars()
    {
        gpad_enabled = Dvar::Register<bool>("gpad_enabled", false, Game::DVAR_FLAG_SAVED, "Game pad enabled");
        gpad_debug = Dvar::Register<bool>("gpad_debug", false, 0, "Game pad debugging");
        gpad_present = Dvar::Register<bool>("gpad_present", false, 0, "Game pad present");
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

    Gamepad::Gamepad()
    {
        if (ZoneBuilder::IsEnabled())
            return;

        // use the xinput state when creating a usercmd
        Utils::Hook(0x5A6DB9, Gamepad::CL_CreateCmdStub, HOOK_JUMP).install()->quick();

        // package the forward and right move components in the move buttons
        Utils::Hook(0x60E38D, Gamepad::MSG_WriteDeltaUsercmdKeyStub, HOOK_JUMP).install()->quick();

        // send two bytes for sending movement data
        Utils::Hook::Set<BYTE>(0x60E501, 16);
        Utils::Hook::Set<BYTE>(0x60E5CD, 16);

        // make sure to parse the movement data properly and apply it
        Utils::Hook(0x492127, Gamepad::MSG_ReadDeltaUsercmdKeyStub, HOOK_JUMP).install()->quick();
        Utils::Hook(0x492009, Gamepad::MSG_ReadDeltaUsercmdKeyStub2, HOOK_JUMP).install()->quick();

        if (Dedicated::IsEnabled())
            return;

        Utils::Hook(0x467C03, IN_Init_Hk, HOOK_CALL).install()->quick();

        Utils::Hook(0x475E9E, IN_Frame_Hk, HOOK_CALL).install()->quick();

        Utils::Hook(0x5A617D, CL_GetMouseMovementStub, HOOK_CALL).install()->quick();
        Utils::Hook(0x5A6816, CL_GetMouseMovementStub, HOOK_CALL).install()->quick();
        Utils::Hook(0x5A6829, unk_CheckKeyHook, HOOK_CALL).install()->quick();

        Scheduler::OnFrame(MenuNavigate);

        xpadSensitivity = Dvar::Register<float>("xpad_sensitivity", 1.9f, 0.1f, 10.0f, Game::DVAR_FLAG_SAVED, "View sensitivity for XInput-compatible gamepads");
        xpadEarlyTime = Dvar::Register<int>("xpad_early_time", 130, 0, 1000, Game::DVAR_FLAG_SAVED, "Time (in milliseconds) of reduced view sensitivity");
        xpadEarlyMultiplier = Dvar::Register<float>("xpad_early_multiplier", 0.25f, 0.01f, 1.0f, Game::DVAR_FLAG_SAVED,
                                                    "By how much the view sensitivity is multiplied during xpad_early_time when moving the view stick");
        xpadHorizontalMultiplier = Dvar::Register<float>("xpad_horizontal_multiplier", 1.5f, 1.0f, 20.0f, Game::DVAR_FLAG_SAVED, "Horizontal view sensitivity multiplier");
        xpadVerticalMultiplier = Dvar::Register<float>("xpad_vertical_multiplier", 0.8f, 1.0f, 20.0f, Game::DVAR_FLAG_SAVED, "Vertical view sensitivity multiplier");
        xpadAdsMultiplier = Dvar::Register<float>("xpad_ads_multiplier", 0.7f, 0.1f, 1.0f, Game::DVAR_FLAG_SAVED, "By how much the view sensitivity is multiplied when aiming down the sights.");
    }
}
