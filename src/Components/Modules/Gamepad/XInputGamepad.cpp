#include "XInputGamepad.hpp"

namespace Components::GamepadControls
{
	bool XInputGamePadAPI::PlugIn(uint8_t portIndex)
	{
		if (XInputGetCapabilities(portIndex, XINPUT_FLAG_GAMEPAD, &caps) == ERROR_SUCCESS)
		{
			return true;
		}

		return false;
	}

	void XInputGamePadAPI::UpdateRumbles(float left, float right)
	{
		rumble.wRightMotorSpeed = static_cast<WORD>(right * 65535.0);
		rumble.wLeftMotorSpeed = static_cast<WORD>(left * 65535.0);
	}

	void XInputGamePadAPI::StopRumbles()
	{
		rumble.wLeftMotorSpeed = 0;
		rumble.wRightMotorSpeed = 0;
		XInputSetState(gamePadIndex, &rumble);
	}

	bool XInputGamePadAPI::Fetch()
	{
		if (XInputGetState(gamePadIndex, &state) == ERROR_SUCCESS)
		{
			// OK
			return true;
		}

		return false;
	}

	void XInputGamePadAPI::ReadSticks(Game::vec2_t& leftStick, Game::vec2_t& rightStick)
	{
		ConvertStickToFloat(state.Gamepad.sThumbLX, state.Gamepad.sThumbLY, leftStick[0], leftStick[1]);
		ConvertStickToFloat(state.Gamepad.sThumbRX, state.Gamepad.sThumbRY, rightStick[0], rightStick[1]);
	}

	void XInputGamePadAPI::ReadDigitals(unsigned short& digitals)
	{
		digitals = state.Gamepad.wButtons;
	}

	void XInputGamePadAPI::ReadAnalogs(float& leftTrigger, float& rightTrigger)
	{
		leftTrigger = static_cast<float>(state.Gamepad.bLeftTrigger) / static_cast<float>(std::numeric_limits<unsigned char>::max());
		rightTrigger = static_cast<float>(state.Gamepad.bRightTrigger) / static_cast<float>(std::numeric_limits<unsigned char>::max());
	}
	void XInputGamePadAPI::Send()
	{
		// We have nothing else to set here
		XInputSetState(gamePadIndex, &rumble);
	}
}
