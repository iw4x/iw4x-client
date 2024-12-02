#include "STDInclude.hpp"
#include "Controller.hpp"

namespace Components::GamepadControls
{
	Dvar::Var Controller::gpad_debug;

	Dvar::Var Controller::gpad_allow_force_feedback;
	Dvar::Var Controller::gpad_force_xinput_only;

	Dvar::Var Controller::gpad_stick_pressed_hysteresis;
	Dvar::Var Controller::gpad_stick_pressed;
	Dvar::Var Controller::gpad_stick_deadzone_max;
	Dvar::Var Controller::gpad_stick_deadzone_min;
	Dvar::Var Controller::gpad_button_deadzone;
	Dvar::Var Controller::gpad_button_rstick_deflect_max;
	Dvar::Var Controller::gpad_button_lstick_deflect_max;

	bool Controller::PlugIn(uint8_t index)
	{
		enabled = false;

		if (gpad_force_xinput_only.get<bool>() == false)
		{
			// Dualsense first
			if (!enabled)
			{
				api.reset(new GamepadControls::DualSenseGamePadAPI());

				if (api->PlugIn(index))
				{
					portIndex = index;
					enabled = true;
				}
			}
		}

		// Xinput
		if (!enabled)
		{
			api.reset(new GamepadControls::XInputGamePadAPI());

			if (api->PlugIn(index))
			{
				portIndex = index;
				enabled = true;
			}
		}

		if (!enabled)
		{
			// Clear memory
			api.reset(nullptr);
		}

		return enabled;
	}

	void Controller::SetLowRumble(double rumble)
	{
		lowRumble = static_cast<float>(rumble);
	}

	void Controller::SetHighRumble(double rumble)
	{
		highRumble = static_cast<float>(rumble);
	}

	void Controller::SetForceFeedback(const GamepadAPI::TriggerFeedback& left, const GamepadAPI::TriggerFeedback& right)
	{
		if (gpad_allow_force_feedback.get<bool>())
		{
			leftForceFeedback = left;
			rightForceFeedback = right;
		}
		else
		{
			leftForceFeedback.resistance = GamepadAPI::TriggerFeedback::NoResistance;
			rightForceFeedback.resistance = GamepadAPI::TriggerFeedback::NoResistance;
		}
	}

	void Controller::StopRumbles()
	{
		lowRumble = 0.f;
		highRumble = 0.f;
	}

	void Controller::UpdateState(bool additive)
	{
		// We read
		if (api)
		{
			if (api->Fetch())
			{
				UpdateSticks(additive);
				UpdateAnalogs();
				UpdateDigitals(additive);
			}
			else
			{
				enabled = false;
			}
		}
		else
		{
			enabled = false;
		}
	}

	void Controller::PushUpdates()
	{
		// We write
		if (api)
		{
			api->UpdateLights(RGB(50, 237, 40));
			api->UpdateForceFeedback(leftForceFeedback, rightForceFeedback);
			api->UpdateRumbles(lowRumble, highRumble);
			api->Send();
		}
	}


	void Controller::UpdateDigitals(bool additive)
	{
		lastDigitals = digitals;
		
		if (api)
		{
			unsigned short newDigitals{};
			api->ReadDigitals(newDigitals);
			
			if (additive)
			{
				digitals |= newDigitals;
			}
			else
			{
				digitals = newDigitals;
			}
		}

		const auto leftDeflect = gpad_button_lstick_deflect_max.get<float>();
		if (std::fabs(sticks[0]) > leftDeflect || std::fabs(sticks[1]) > leftDeflect)
		{
			digitals &= ~static_cast<short>(XINPUT_GAMEPAD_LEFT_THUMB);
		}

		const auto rightDeflect = gpad_button_rstick_deflect_max.get<float>();
		if (std::fabs(sticks[2]) > leftDeflect || std::fabs(sticks[3]) > rightDeflect)
		{
			digitals &= ~static_cast<short>(XINPUT_GAMEPAD_RIGHT_THUMB);
		}

		if (gpad_debug.get<bool>())
		{
			Logger::Debug("Buttons: {:#x}", digitals);
		}
	}

	void Controller::UpdateAnalogs()
	{
		const auto buttonDeadZone = gpad_button_deadzone.get<float>();

		lastAnalogs[0] = analogs[0];
		lastAnalogs[1] = analogs[1];

		if (api)
		{
			api->ReadAnalogs(analogs[0], analogs[1]);
		}

		if (analogs[0] < buttonDeadZone)
		{
			analogs[0] = 0.0f;
		}

		if (analogs[1] < buttonDeadZone)
		{
			analogs[1] = 0.0f;
		}

		if (gpad_debug.get<bool>())
		{
			Logger::Debug("Triggers: {:f} {:f}", analogs[0], analogs[1]);
		}
	}

	void Controller::UpdateSticks(bool additive)
	{

		lastSticks[0] = sticks[0];
		lastSticks[1] = sticks[1];
		lastSticks[2] = sticks[2];
		lastSticks[3] = sticks[3];

		Game::vec2_t lVec, rVec;

		if (api)
		{
			api->ReadSticks(lVec, rVec);
		}

		ApplyDeadzone(lVec);
		ApplyDeadzone(rVec);

		sticks[0] = lVec[0];
		sticks[1] = lVec[1];
		sticks[2] = rVec[0];
		sticks[3] = rVec[1];

		UpdateSticksDown(additive);

		if (gpad_debug.get<bool>())
		{
			Logger::Debug("Left: X: {:f} Y: {:f}", lVec[0], lVec[1]);
			Logger::Debug("Right: X: {:f} Y: {:f}", rVec[0], rVec[1]);
			Logger::Debug("Down: {}:{} {}:{} {}:{} {}:{}", stickDown[0][Game::GPAD_STICK_POS], stickDown[0][Game::GPAD_STICK_NEG],
				stickDown[1][Game::GPAD_STICK_POS], stickDown[1][Game::GPAD_STICK_NEG],
				stickDown[2][Game::GPAD_STICK_POS], stickDown[2][Game::GPAD_STICK_NEG],
				stickDown[3][Game::GPAD_STICK_POS], stickDown[3][Game::GPAD_STICK_NEG]);
		}
	}

	void Controller::ApplyDeadzone(Game::vec2_t& stick)
	{
		const auto deadZoneTotal = gpad_stick_deadzone_min.get<float>() + gpad_stick_deadzone_max.get<float>();
		auto len = Game::Vec2Normalize(stick);

		if (gpad_stick_deadzone_min.get<float>() <= len)
		{
			if (1.0f - gpad_stick_deadzone_max.get<float>() >= len)
			{
				len = (len - gpad_stick_deadzone_min.get<float>()) / (1.0f - deadZoneTotal);
			}
			else
			{
				len = 1.0f;
			}
		}
		else
		{
			len = 0.0f;
		}

		stick[0] *= len;
		stick[1] *= len;
	}

	float Controller::GetStick(const Game::GamePadStick stick)
	{
		assert(stick & Game::GPAD_STICK_MASK);
		return sticks[stick];
	}

	float Controller::GetButton(Game::GamePadButton button)
	{
		float value = 0.0f;

		if (button & Game::GPAD_DIGITAL_MASK)
		{
			const auto buttonValue = button & Game::GPAD_VALUE_MASK;
			value = buttonValue & digitals ? 1.0f : 0.0f;
		}
		else if (button & Game::GPAD_ANALOG_MASK)
		{
			const auto analogIndex = button & Game::GPAD_VALUE_MASK;
			if (analogIndex < std::extent_v<decltype(analogs)>)
			{
				value = analogs[analogIndex];
			}
		}

		return value;
	}


	bool Controller::ButtonRequiresUpdates(Game::GamePadButton button)
	{
		return (button & Game::GPAD_ANALOG_MASK || button & Game::GPAD_DPAD_MASK) && GetButton(button) > 0.0f;
	}

	bool Controller::IsButtonReleased(Game::GamePadButton button)
	{
		bool down = false;
		bool lastDown = false;

		if (button & Game::GPAD_DIGITAL_MASK)
		{
			const auto buttonValue = button & Game::GPAD_VALUE_MASK;

			down = (digitals & buttonValue) != 0;
			lastDown = (lastDigitals & buttonValue) != 0;
		}
		else if (button & Game::GPAD_ANALOG_MASK)
		{
			const auto analogIndex = button & Game::GPAD_VALUE_MASK;
			assert(analogIndex < std::extent_v<decltype(analogs)>);

			if (analogIndex < std::extent_v<decltype(analogs)>)
			{
				down = analogs[analogIndex] > 0.0f;
				lastDown = lastAnalogs[analogIndex] > 0.0f;
			}
		}

		return !down && lastDown;
	}


	bool Controller::IsButtonPressed(Game::GamePadButton button)
	{
		bool down = false;
		bool lastDown = false;

		if (button & Game::GPAD_DIGITAL_MASK)
		{
			const auto buttonValue = button & Game::GPAD_VALUE_MASK;
			down = (buttonValue & digitals) != 0;
			lastDown = (buttonValue & lastDigitals) != 0;
		}
		else if (button & Game::GPAD_ANALOG_MASK)
		{
			const auto analogIndex = button & Game::GPAD_VALUE_MASK;
			assert(analogIndex < std::extent_v<decltype(analogs)>);

			if (analogIndex < std::extent_v<decltype(analogs)>)
			{
				down = analogs[analogIndex] > 0.0f;
				lastDown = lastAnalogs[analogIndex] > 0.0f;
			}
		}

		return down && !lastDown;
	}

	void Controller::UpdateSticksDown(bool additive)
	{
		for (auto stickIndex = 0u; stickIndex < std::extent_v<decltype(Controller::sticks)>; stickIndex++)
		{
			for (auto dir = 0; dir < Game::GPAD_STICK_DIR_COUNT; dir++)
			{
				if (!additive)
				{
					stickDown[stickIndex][dir] = false;
				}

				stickDownLast[stickIndex][dir] = stickDown[stickIndex][dir];

				auto threshold = gpad_stick_pressed.get<float>();

				if (stickDownLast[stickIndex][dir])
				{
					threshold -= gpad_stick_pressed_hysteresis.get<float>();
				}
				else
				{
					threshold += gpad_stick_pressed_hysteresis.get<float>();
				}

				if (dir == Game::GPAD_STICK_POS)
				{
					stickDown[stickIndex][dir] |= sticks[stickIndex] > threshold;
				}
				else
				{
					assert(dir == Game::GPAD_STICK_NEG);
					stickDown[stickIndex][dir] |= sticks[stickIndex] < -threshold;
				}
			}
		}
	}

	void Controller::InitializeDvars()
	{
		gpad_debug = Dvar::Register<bool>("gpad_debug", false, Game::DVAR_NONE, "Game pad debugging");

		gpad_allow_force_feedback = Dvar::Register<bool>("gpad_allow_force_feedback", true, Game::DVAR_NONE, "Allow force feedback if the game pad supports it");

		gpad_force_xinput_only = Dvar::Register<bool>("gpad_force_xinput_only", false, Game::DVAR_ARCHIVE, "Only listen for XInput controllers and ignore the rest");

		gpad_stick_pressed_hysteresis = Dvar::Register<float>("gpad_stick_pressed_hysteresis", 0.1f, 0.0f, 1.0f, Game::DVAR_ARCHIVE,
			"Game pad stick pressed no-change-zone around gpad_stick_pressed to prevent bouncing");
		gpad_stick_pressed = Dvar::Register<float>("gpad_stick_pressed", 0.4f, 0.0, 1.0, Game::DVAR_ARCHIVE, "Game pad stick pressed threshhold");
		gpad_stick_deadzone_max = Dvar::Register<float>("gpad_stick_deadzone_max", 0.01f, 0.0f, 1.0f, Game::DVAR_ARCHIVE, "Game pad maximum stick deadzone");
		gpad_stick_deadzone_min = Dvar::Register<float>("gpad_stick_deadzone_min", 0.2f, 0.0f, 1.0f, Game::DVAR_ARCHIVE, "Game pad minimum stick deadzone");
		gpad_button_deadzone = Dvar::Register<float>("gpad_button_deadzone", 0.13f, 0.0f, 1.0f, Game::DVAR_ARCHIVE, "Game pad button deadzone threshhold");
		gpad_button_lstick_deflect_max = Dvar::Register<float>("gpad_button_lstick_deflect_max", 1.0f, 0.0f, 1.0f, Game::DVAR_ARCHIVE, "Game pad maximum pad stick pressed value");
		gpad_button_rstick_deflect_max = Dvar::Register<float>("gpad_button_rstick_deflect_max", 1.0f, 0.0f, 1.0f, Game::DVAR_ARCHIVE, "Game pad maximum pad stick pressed value");
	}
}
