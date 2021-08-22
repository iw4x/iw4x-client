#pragma once

namespace Game
{
    static constexpr auto MAX_GAMEPADS = 1;

    static constexpr auto GPAD_VALUE_MASK = 0xFFFFFFFu;
    static constexpr auto GPAD_DPAD_MASK = XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_LEFT | XINPUT_GAMEPAD_DPAD_RIGHT;
    static constexpr auto GPAD_DIGITAL_MASK = 1u << 28;
    static constexpr auto GPAD_ANALOG_MASK = 1u << 29;
    static constexpr auto GPAD_STICK_MASK = 1u << 30;

    enum GamePadButton
    {
        GPAD_NONE = 0,
        GPAD_UP = GPAD_DIGITAL_MASK | (XINPUT_GAMEPAD_DPAD_UP & GPAD_VALUE_MASK),
        GPAD_DOWN = GPAD_DIGITAL_MASK | (XINPUT_GAMEPAD_DPAD_DOWN & GPAD_VALUE_MASK),
        GPAD_LEFT = GPAD_DIGITAL_MASK | (XINPUT_GAMEPAD_DPAD_LEFT & GPAD_VALUE_MASK),
        GPAD_RIGHT = GPAD_DIGITAL_MASK | (XINPUT_GAMEPAD_DPAD_RIGHT & GPAD_VALUE_MASK),
        GPAD_START = GPAD_DIGITAL_MASK | (XINPUT_GAMEPAD_START & GPAD_VALUE_MASK),
        GPAD_BACK = GPAD_DIGITAL_MASK | (XINPUT_GAMEPAD_BACK & GPAD_VALUE_MASK),
        GPAD_L3 = GPAD_DIGITAL_MASK | (XINPUT_GAMEPAD_LEFT_THUMB & GPAD_VALUE_MASK),
        GPAD_R3 = GPAD_DIGITAL_MASK | (XINPUT_GAMEPAD_RIGHT_THUMB & GPAD_VALUE_MASK),
        GPAD_L_SHLDR = GPAD_DIGITAL_MASK | (XINPUT_GAMEPAD_LEFT_SHOULDER & GPAD_VALUE_MASK),
        GPAD_R_SHLDR = GPAD_DIGITAL_MASK | (XINPUT_GAMEPAD_RIGHT_SHOULDER & GPAD_VALUE_MASK),
        GPAD_A = GPAD_DIGITAL_MASK | (XINPUT_GAMEPAD_A & GPAD_VALUE_MASK),
        GPAD_B = GPAD_DIGITAL_MASK | (XINPUT_GAMEPAD_B & GPAD_VALUE_MASK),
        GPAD_X = GPAD_DIGITAL_MASK | (XINPUT_GAMEPAD_X & GPAD_VALUE_MASK),
        GPAD_Y = GPAD_DIGITAL_MASK | (XINPUT_GAMEPAD_Y & GPAD_VALUE_MASK),
        GPAD_L_TRIG = GPAD_ANALOG_MASK | (0 & GPAD_VALUE_MASK),
        GPAD_R_TRIG = GPAD_ANALOG_MASK | (1 & GPAD_VALUE_MASK),
    };

    enum GamePadStick
    {
        GPAD_INVALID = 0x0,
        GPAD_LX = GPAD_STICK_MASK | (0 & GPAD_VALUE_MASK),
        GPAD_LY = GPAD_STICK_MASK | (1 & GPAD_VALUE_MASK),
        GPAD_RX = GPAD_STICK_MASK | (2 & GPAD_VALUE_MASK),
        GPAD_RY = GPAD_STICK_MASK | (3 & GPAD_VALUE_MASK),
    };

    enum GamePadButtonEvent
    {
        GPAD_BUTTON_RELEASED = 0x0,
        GPAD_BUTTON_PRESSED = 0x1,
        GPAD_BUTTON_UPDATE = 0x2,
    };

    enum GamepadPhysicalAxis
    {
        GPAD_PHYSAXIS_NONE = -1,
        GPAD_PHYSAXIS_RSTICK_X = 0x0,
        GPAD_PHYSAXIS_RSTICK_Y = 0x1,
        GPAD_PHYSAXIS_LSTICK_X = 0x2,
        GPAD_PHYSAXIS_LSTICK_Y = 0x3,
        GPAD_PHYSAXIS_RTRIGGER = 0x4,
        GPAD_PHYSAXIS_LTRIGGER = 0x5,

        GPAD_PHYSAXIS_COUNT,
    };

    enum GamePadStickDir
    {
        GPAD_STICK_POS = 0x0,
        GPAD_STICK_NEG = 0x1,

        GPAD_STICK_DIR_COUNT
    };

    enum GamepadMapping
    {
        GPAD_MAP_NONE = -1,
        GPAD_MAP_LINEAR = 0x0,
        GPAD_MAP_SQUARED = 0x1,
        GPAD_MAP_COUNT = 0x2
    };

    struct ButtonToCodeMap_t
    {
        GamePadButton padButton;
        int code;
    };

    struct StickToCodeMap_t
    {
        GamePadStick padStick;
        int posCode;
        int negCode;
    };

    struct GamepadVirtualAxisMapping
    {
        GamepadPhysicalAxis physicalAxis;
        GamepadMapping mapType;
    };

    struct GpadAxesGlob
    {
        float axesValues[GPAD_PHYSAXIS_COUNT];
        GamepadVirtualAxisMapping virtualAxes[GPAD_PHYSAXIS_COUNT];
    };
}

namespace Components
{
    class Gamepad : public Component
    {
        static constexpr float TRIGGER_THRESHOLD_F = static_cast<float>(XINPUT_GAMEPAD_TRIGGER_THRESHOLD) / static_cast<float>(0xFF);

    public:
        Gamepad();

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
            bool stickDown[4][Game::GPAD_STICK_DIR_COUNT];
            bool stickDownLast[4][Game::GPAD_STICK_DIR_COUNT];
            float lowRumble;
            float highRumble;

            XINPUT_VIBRATION rumble;
            XINPUT_CAPABILITIES caps;
        };

        struct GamePadGlobals
        {
            unsigned nextScrollTime;
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

    private:
        static GamePad gamePads[Game::MAX_GAMEPADS];
        static GamePadGlobals gamePadGlobals[Game::MAX_GAMEPADS];

        static bool isHoldingMaxLookX;
        static std::chrono::milliseconds timeAtFirstHeldMaxLookX;
        static bool isADS;

        static std::chrono::milliseconds lastNavigationTime;
        static std::chrono::milliseconds msBetweenNavigations;
        static Game::GamePadStickDir lastMenuNavigationDirection;

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

        static void MSG_WriteDeltaUsercmdKeyStub();

        static void ApplyMovement(Game::msg_t* msg, int key, Game::usercmd_s* from, Game::usercmd_s* to);

        static void MSG_ReadDeltaUsercmdKeyStub();
        static void MSG_ReadDeltaUsercmdKeyStub2();

        static void GetLeftStick01Value(XINPUT_STATE* xiState, float& x, float& y);
        static void GetRightStick01Value(XINPUT_STATE* xiState, float& x, float& y);
        static void GamepadStickTo01(SHORT value, SHORT deadzone, float& output01);

        static void CL_GamepadResetMenuScrollTime(int gamePadIndex, int key, bool down, unsigned int time);
        static bool CL_CheckForIgnoreDueToRepeat(int gamePadIndex, int key, int repeatCount, unsigned int time);
        static void UI_GamepadKeyEvent(int gamePadIndex, int key, bool down);
        static void CL_GamepadGenerateAPad(int gamePadIndex, Game::GamepadPhysicalAxis physicalAxis, unsigned time);
        static void CL_GamepadEvent(int gamePadIndex, Game::GamepadPhysicalAxis physicalAxis, float value, unsigned time);
        static void CL_GamepadButtonEvent(int gamePadIndex, int key, Game::GamePadButtonEvent buttonEvent, unsigned time, Game::GamePadButton button);
        static void CL_GamepadButtonEventForPort(int gamePadIndex, int key, Game::GamePadButtonEvent buttonEvent, unsigned int time, Game::GamePadButton button);

        static void GPad_ConvertStickToFloat(short x, short y, float& outX, float& outY);
        static float GPad_GetStick(int gamePadIndex, Game::GamePadStick stick);
        static float GPad_GetButton(int gamePadIndex, Game::GamePadButton button);
        static bool GPad_IsButtonPressed(int gamePadIndex, Game::GamePadButton button);
        static bool GPad_ButtonRequiresUpdates(int gamePadIndex, Game::GamePadButton button);
        static bool GPad_IsButtonReleased(int gamePadIndex, Game::GamePadButton button);

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

        static void CreateKeyNameMap();
    };
}
