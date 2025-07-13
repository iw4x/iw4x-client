#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "Gamepad/Controller.hpp"

namespace Components
{
	class Gamepad : public Component
	{
		struct ControllerMenuKeyMapping
		{
			Game::keyNum_t controllerKey;
			Game::keyNum_t pcKey;
		};

		struct GamePadGlobals
		{
			Game::GpadAxesGlob axes;
			unsigned nextScrollTime;

			GamePadGlobals();
		};

	public:
		static const int RUMBLE_CONFIGSTRINGS_COUNT = 32;

		Gamepad();
		~Gamepad();

		static void OnMouseMove(int x, int y, int dx, int dy);

		static void GPad_SetLowRumble(int gamePadIndex, double rumble);
		static void GPad_SetHighRumble(int gamePadIndex, double rumble);
		static void GPad_StopRumbles(int gamePadIndex);
		static void GPad_UpdateFeedbacks();

		static Dvar::Var sv_allowAimAssist;

		static bool IsGamePadInUse();

	private:
		enum TriggerRole
		{
			None,
			Shooting,
			Aiming,
			PrimaryOffHand,
			SecondaryOffHand
		};

		static Game::ButtonToCodeMap_t buttonList[];
		static Game::StickToCodeMap_t analogStickList[4];
		static Game::GamePadStick stickForAxis[];
		static Game::GamepadPhysicalAxis axisSameStick[];
		static const char* physicalAxisNames[];
		static const char* virtualAxisNames[];
		static const char* gamePadMappingTypeNames[];
		static Game::keyNum_t menuScrollButtonList[];
		static Game::keyname_t extendedKeyNames[];
		static Game::keyname_t extendedLocalizedKeyNamesXenon[];
		static Game::keyname_t extendedLocalizedKeyNamesPs3[];
		static Game::keyname_t combinedKeyNames[];
		static Game::keyname_t combinedLocalizedKeyNamesXenon[];
		static Game::keyname_t combinedLocalizedKeyNamesPs3[];
		static ControllerMenuKeyMapping controllerMenuKeyMappings[];

		static GamepadControls::Controller gamePads[Game::MAX_GPAD_COUNT];
		static GamePadGlobals gamePadGlobals[Game::MAX_GPAD_COUNT];
		static std::mutex gamePadStateMutexes[Game::MAX_GPAD_COUNT];

		static bool gamePadDataReady[Game::MAX_GPAD_COUNT];
		static int gamePadBindingsModifiedFlags;

		static Dvar::Var gpad_enabled;
		static Dvar::Var gpad_present;
		static Dvar::Var gpad_in_use;
		static Dvar::Var gpad_style;
		static Dvar::Var gpad_sticksConfig;
		static Dvar::Var gpad_buttonConfig;
		static Dvar::Var gpad_menu_scroll_delay_first;
		static Dvar::Var gpad_menu_scroll_delay_rest;
		static Dvar::Var gpad_rumble;
		static Dvar::Var gpad_use_hold_time;
		static Dvar::Var gpad_lockon_enabled;
		static Dvar::Var gpad_slowdown_enabled;
		static Dvar::Var input_viewSensitivity;
		static Dvar::Var input_invertPitch;
		static Dvar::Var aim_turnrate_pitch;
		static Dvar::Var aim_turnrate_pitch_ads;
		static Dvar::Var aim_turnrate_yaw;
		static Dvar::Var aim_turnrate_yaw_ads;
		static Dvar::Var aim_accel_turnrate_enabled;
		static Dvar::Var aim_accel_turnrate_lerp;
		static Dvar::Var aim_input_graph_enabled;
		static Dvar::Var aim_input_graph_index;
		static Dvar::Var aim_scale_view_axis;
		static Dvar::Var cl_bypassMouseInput;
		static Dvar::Var cg_mapLocationSelectionCursorSpeed;
		static Dvar::Var aim_aimAssistRangeScale;
		static Dvar::Var aim_slowdown_enabled;
		static Dvar::Var aim_slowdown_debug;
		static Dvar::Var aim_slowdown_pitch_scale;
		static Dvar::Var aim_slowdown_pitch_scale_ads;
		static Dvar::Var aim_slowdown_yaw_scale;
		static Dvar::Var aim_slowdown_yaw_scale_ads;
		static Dvar::Var aim_lockon_enabled;
		static Dvar::Var aim_lockon_deflection;
		static Dvar::Var aim_lockon_pitch_strength;
		static Dvar::Var aim_lockon_strength;

		static DS5W::DeviceContext dualSenseContext;

		static void MSG_WriteDeltaUsercmdKeyStub();

		static void ApplyMovement(Game::msg_t* msg, int key, Game::usercmd_s* from, Game::usercmd_s* to);

		static void MSG_ReadDeltaUsercmdKeyStub();
		static void MSG_ReadDeltaUsercmdKeyStub2();

		static float LinearTrack(float target, float current, float rate, float deltaTime);
		static bool AimAssist_DoBoundsIntersectCenterBox(const float* clipMins, const float* clipMaxs, float clipHalfWidth, float clipHalfHeight);
		static bool AimAssist_IsPlayerUsingOffhand(Game::AimAssistPlayerState* ps);
		static const Game::AimScreenTarget* AimAssist_GetBestTarget(const Game::AimAssistGlobals* aaGlob, float range, float regionWidth, float regionHeight);
		static const Game::AimScreenTarget* AimAssist_GetTargetFromEntity(const Game::AimAssistGlobals* aaGlob, int entIndex);
		static const Game::AimScreenTarget* AimAssist_GetPrevOrBestTarget(const Game::AimAssistGlobals* aaGlob, float range, float regionWidth, float regionHeight, int prevTargetEnt);
		static bool AimAssist_IsLockonActive(int localClientNum);
		static void AimAssist_ApplyLockOn(const Game::AimInput* input, Game::AimOutput* output);
		static void AimAssist_CalcAdjustedAxis(const Game::AimInput* input, float* pitchAxis, float* yawAxis);
		static bool AimAssist_IsSlowdownActive(const Game::AimAssistPlayerState* ps);
		static void AimAssist_CalcSlowdown(const Game::AimInput* input, float* pitchScale, float* yawScale);
		static void AimAssist_ApplyTurnRates(const Game::AimInput* input, Game::AimOutput* output);
		static void AimAssist_UpdateGamePadInput(const Game::AimInput* input, Game::AimOutput* output);

		static void CL_RemoteControlMove_GamePad(int localClientNum, Game::usercmd_s* cmd);
		static void CL_RemoteControlMove_Stub();
		static bool CG_HandleLocationSelectionInput_GamePad(int localClientNum, Game::usercmd_s* cmd);
		static void CG_HandleLocationSelectionInput_Stub();
		static bool CG_ShouldUpdateViewAngles(int localClientNum);
		static float CL_GamepadAxisValue(int localClientNum, Game::GamepadVirtualAxis virtualAxis);
		static char ClampChar(int value);
		static void CL_GamepadMove(int localClientNum, float frameTimeBase, Game::usercmd_s* cmd);
		static void CL_MouseMove(int localClientNum, Game::usercmd_s* cmd, float frametime_base);
		static void CL_MouseMove_Stub();

		static bool Gamepad_ShouldUse(const Game::gentity_s* playerEnt, unsigned useTime);
		static void Player_UseEntity_Stub();

		static bool Key_IsValidGamePadChar(int key);
		static void CL_GamepadResetMenuScrollTime(const int localClientNum, const int key, const bool down, const unsigned time);
		static bool Scoreboard_HandleInput(int localClientNum, int key);
		static bool CL_CheckForIgnoreDueToRepeat(const int localClientNum, const int key, const int repeatCount, const unsigned time);
		static void UI_GamepadKeyEvent(int localClientNum, int key, bool down);
		static void CL_GamepadGenerateAPad(int localClientNum, Game::GamepadPhysicalAxis physicalAxis, unsigned time);
		static void CL_GamepadEvent(int localClientNum, Game::GamepadPhysicalAxis physicalAxis, float value, unsigned time);
		static void CL_GamepadButtonEvent(int localClientNum, int key, Game::GamePadButtonEvent buttonEvent, unsigned time);
		static void CL_GamepadButtonEventForPort(const int localClientNum, const int key, const Game::GamePadButtonEvent buttonEvent, const unsigned time);

		static bool GPad_Check(int localClientNum, int portIndex);
		static void GPad_RefreshAll();
		static void GPad_UpdateAll();
		static void IN_GamePadsMove();
		static void IN_Frame_Hk();

		static void Gamepad_WriteBindings(int localClientNum, int handle);
		static void Key_WriteBindings_Hk(int localClientNum, int handle);
		static void Com_WriteConfiguration_Modified_Stub();

		static void Gamepad_BindAxis(int localClientNum, Game::GamepadPhysicalAxis realIndex, Game::GamepadVirtualAxis axisIndex, Game::GamepadMapping mapType);
		static Game::GamepadPhysicalAxis StringToPhysicalAxis(const char* str);
		static Game::GamepadVirtualAxis StringToVirtualAxis(const char* str);
		static Game::GamepadMapping StringToGamePadMapping(const char* str);
		static void Axis_Bind_f(const Command::Params* params);
		static void Axis_Unbindall_f();
		static void Bind_GP_SticksConfigs_f();
		static void Bind_GP_ButtonsConfigs_f();
		static void Scores_Toggle_f();

		static void InitDvars();
		static void CG_RegisterDvars_Hk();

		static const char* GetGamePadCommand(const char* command);
		static int Key_GetCommandAssignmentInternal(int localClientNum, const char* cmd, int (*keys)[2]);
		static void Key_GetCommandAssignmentInternal_Stub();
		static void Key_SetBinding_Hk(int localClientNum, int keyNum, const char* binding);
		static void CL_KeyEvent_Hk(const int localClientNum, const int key, const int down, const unsigned time);
		static int CL_MouseEvent_Hk(int x, int y, int dx, int dy);
		static bool UI_RefreshViewport_Hk();

		static Game::keyname_t* GetLocalizedKeyNameMap();
		static void GetLocalizedKeyName_Stub();
		static void CreateKeyNameMap();

		static void GetTriggerRoles(TriggerRole& leftTrigger, TriggerRole& rightTrigger);
		static void GetTriggerFeedbackForShooting(const Game::playerState_s* playerState, GamepadControls::GamepadAPI::TriggerFeedback& feedback);
		static void GetTriggerFeedbackForEquipment(const Game::playerState_s* playerState, bool primary, GamepadControls::GamepadAPI::TriggerFeedback& feedback);

		static void UpdateForceFeedback(GamepadControls::Controller& api);
	};
}
