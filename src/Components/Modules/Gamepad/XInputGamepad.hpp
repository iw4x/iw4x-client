#pragma once
#include "GamepadAPI.hpp"

namespace Components::GamepadControls
{
	class XInputGamePadAPI final : public GamepadAPI
	{
	public:
		bool PlugIn(uint8_t plugIndex) override;

		void UpdateRumbles(float left, float right) override;
		void StopRumbles() override;

		bool Fetch() override;
		void ReadSticks(Game::vec2_t& leftStick, Game::vec2_t& rightStick) override;
		void ReadDigitals(unsigned short& digitals) override;
		void ReadAnalogs(float& leftTrigger, float& rightTrigger) override;

		void Send() override;

	private:
		DWORD gamePadIndex;
		XINPUT_VIBRATION rumble;
		XINPUT_CAPABILITIES caps;
		XINPUT_STATE state;
	};

}
