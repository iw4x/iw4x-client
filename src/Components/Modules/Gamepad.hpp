#pragma once

namespace Components
{
    class Gamepad : public Component
    {
        static constexpr auto MAX_GAMEPADS = 1;
        static constexpr float TRIGGER_THRESHOLD_F = static_cast<float>(XINPUT_GAMEPAD_TRIGGER_THRESHOLD) / static_cast<float>(0xFF);

    public:
        Gamepad();

        enum GamePadStickDir
        {
            GPAD_STICK_POS = 0x0,
            GPAD_STICK_NEG = 0x1,

            GPAD_STICK_DIR_COUNT
        };

        struct GamePad
        {
            bool enabled;
            int portIndex;
            unsigned short digitals;
            unsigned short lastDigitals;
            float analogs[2];
            float lastAnalogs[2];
            float sticks[4];
            float lastSticks[4];
            bool stickDown[4][GPAD_STICK_DIR_COUNT];
            bool stickDownLast[4][GPAD_STICK_DIR_COUNT];
            float lowRumble;
            float highRumble;

            XINPUT_VIBRATION rumble;
            XINPUT_CAPABILITIES caps;
        };

        struct ActionMapping
        {
            int input;
            std::string action;
            bool isReversible;
            bool wasPressed = false;
            bool spamWhenHeld = false;

            ActionMapping(int input, std::string action, bool isReversible = true, bool spamWhenHeld = false)
            {
                this->action = action;
                this->isReversible = isReversible;
                this->input = input;
                this->spamWhenHeld = spamWhenHeld;
            }
        };

        struct MenuMapping
        {
            int input;
            Game::keyNum_t keystroke;
            bool wasPressed = false;

            MenuMapping(int input, Game::keyNum_t keystroke)
            {
                this->keystroke = keystroke;
                this->input = input;
            }
        };

    private:
        static GamePad gamePads[MAX_GAMEPADS];

        static bool isHoldingMaxLookX;
        static std::chrono::milliseconds timeAtFirstHeldMaxLookX;
        static bool isADS;

        static std::chrono::milliseconds lastNavigationTime;
        static std::chrono::milliseconds msBetweenNavigations;
        static GamePadStickDir lastMenuNavigationDirection;

        static Dvar::Var gpad_enabled;
        static Dvar::Var gpad_debug;
        static Dvar::Var gpad_present;
        static Dvar::Var gpad_sticksConfig;
        static Dvar::Var gpad_buttonConfig;
        static Dvar::Var gpad_menu_scroll_delay_first;
        static Dvar::Var gpad_menu_scroll_delay_rest;
        static Dvar::Var gpad_rumble;
        static Dvar::Var gpad_stick_pressed_hysteresis;
        static Dvar::Var gpad_stick_pressed;
        static Dvar::Var gpad_stick_deadzone_max;
        static Dvar::Var gpad_stick_deadzone_min;
        static Dvar::Var gpad_button_deadzone;
        static Dvar::Var gpad_button_rstick_deflect_max;
        static Dvar::Var gpad_button_lstick_deflect_max;

        static Dvar::Var xpadSensitivity;
        static Dvar::Var xpadEarlyTime;
        static Dvar::Var xpadEarlyMultiplier;
        static Dvar::Var xpadHorizontalMultiplier;
        static Dvar::Var xpadVerticalMultiplier;
        static Dvar::Var xpadAdsMultiplier;

        static void CL_GetMouseMovementCl(Game::clientActive_t* result, float* mx, float* my);
        static int unk_CheckKeyHook(int localClientNum, Game::keyNum_t keyCode);

        static void MouseOverride(Game::clientActive_t* clientActive, float* my, float* mx);
        static void Vibrate(int leftVal = 0, int rightVal = 0);

        static void CL_FrameStub();
        static void PollXInputDevices();

        static void CL_CreateCmdStub();
        static void CL_GamepadMove(int, Game::usercmd_s*);
        static void MenuNavigate();

        static void MSG_WriteDeltaUsercmdKeyStub();

        static void ApplyMovement(Game::msg_t* msg, int key, Game::usercmd_s* from, Game::usercmd_s* to);

        static void MSG_ReadDeltaUsercmdKeyStub();
        static void MSG_ReadDeltaUsercmdKeyStub2();

        static void GetLeftStick01Value(XINPUT_STATE* xiState, float& x, float& y);
        static void GetRightStick01Value(XINPUT_STATE* xiState, float& x, float& y);
        static void GamepadStickTo01(SHORT value, SHORT deadzone, float& output01);

        static void GPad_ConvertStickToFloat(short x, short y, float& outX, float& outY);

        static void GPad_UpdateSticksDown(int gamePadIndex);
        static void GPad_UpdateSticks(int gamePadIndex, const XINPUT_GAMEPAD& state);
        static void GPad_UpdateDigitals(int gamePadIndex, const XINPUT_GAMEPAD& state);
        static void GPad_UpdateAnalogs(int gamePadIndex, const XINPUT_GAMEPAD& state);

        static bool GPad_Check(int gamePadIndex, int portIndex);
        static void GPad_RefreshAll();
        static void GPad_UpdateAll();
        static void IN_GamePadsMove();
        static void IN_Frame_Hk();

        static void InitDvars();
        static void IN_Init_Hk();
    };
}
