#pragma once

namespace Components
{
	class XInput : public Component
	{
	public:
		XInput();

		struct ActionMapping {
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
		static XINPUT_STATE xiStates[XUSER_MAX_COUNT];
		static int xiPlayerNum;
		static XINPUT_STATE lastXiState;

		static bool isHoldingMaxLookX;
		static std::chrono::milliseconds timeAtFirstHeldMaxLookX;
		static std::chrono::milliseconds msBeforeUnlockingSensitivity;
		static float lockedSensitivityMultiplier;
		static float generalXSensitivityMultiplier;
		static float generalYSensitivityMultiplier;

		static void Vibrate(int leftVal = 0, int rightVal = 0);

		static void CL_FrameStub();
		static void PollXInputDevices();

		static void CL_CreateCmdStub();
		static void CL_GamepadMove(int, Game::usercmd_s*);

		static void MSG_WriteDeltaUsercmdKeyStub();

		static void ApplyMovement(Game::msg_t* msg, int key, Game::usercmd_s* from, Game::usercmd_s* to);

		static void MSG_ReadDeltaUsercmdKeyStub();
		static void MSG_ReadDeltaUsercmdKeyStub2();
	};
}
