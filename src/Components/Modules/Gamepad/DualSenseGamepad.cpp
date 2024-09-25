#include <STDInclude.hpp>
#include "DualSenseGamepad.hpp"

bool Components::GamepadControls::DualSenseGamePadAPI::PlugIn(uint8_t index)
{
	ZeroMemory(&context, sizeof(DS5W::DeviceContext));

	DS5W::DeviceEnumInfo infos[Game::MAX_GPAD_COUNT];
	unsigned int controllersCount = 0;
	DS5W_ReturnValue rv = DS5W::enumDevices(infos, Game::MAX_GPAD_COUNT, &controllersCount);

	this->portIndex = index;

	if (DS5W_SUCCESS(rv) && controllersCount > 0)
	{
		if (DS5W_SUCCESS(DS5W::initDeviceContext(&infos[index], &context))) {
			// Ok!
			return true;
		}
		else
		{
			Components::Logger::Error(Game::ERR_SCRIPT, "A DualSense™ controller was detected, but could not be connected to.\n");
		}
	}

	return false;
}

void Components::GamepadControls::DualSenseGamePadAPI::UpdateRumbles(float left, float right)
{
	if (EnsureConnected())
	{
		if (ToSCE(left, outState.leftRumble) && ToSCE(right, outState.rightRumble))
		{
			DS5W::setDeviceOutputState(&context, &outState);
		}
	}
}

bool Components::GamepadControls::DualSenseGamePadAPI::EnsureConnected()
{
	if (context._internal.connected)
	{
		return true;
	}
	else
	{
		ZeroMemory(&inputState, sizeof(DS5W::DS5InputState));
		if (PlugIn(this->portIndex))
		{
			return true; // We reconnected?
		}
		else
		{
			return false;
		}
	}
}


bool Components::GamepadControls::DualSenseGamePadAPI::ToSCE(const float& amount01, unsigned char &amount)
{
	amount = static_cast<unsigned char>(std::clamp(amount01, 0.F, 1.F) * std::numeric_limits<unsigned char>().max());

	return true;
}

bool Components::GamepadControls::DualSenseGamePadAPI::ToSCE(const TriggerFeedback& triggerFeedback, DS5W::TriggerEffect* sce)
{
	if (sce)
	{
		switch (triggerFeedback.resistance)
		{
		default:
		case TriggerFeedback::Resistance::NoResistance:
			sce->effectType = DS5W::TriggerEffectType::NoResitance;
			return true;

		case TriggerFeedback::Resistance::ContinuousSlightResistance:
			sce->effectType = DS5W::TriggerEffectType::ContinuousResitance;
			sce->Continuous.force = 70;
			sce->Continuous.startPosition = 20;
			return true;

		case TriggerFeedback::Resistance::ContinuousHeavyResistance:
			sce->effectType = DS5W::TriggerEffectType::ContinuousResitance;
			sce->Continuous.force = 170;
			sce->Continuous.startPosition = 20;
			return true;

		case TriggerFeedback::Resistance::SectionSlightResistance:
			sce->effectType = DS5W::TriggerEffectType::SectionResitance;
			sce->Section.startPosition = 0x00;
			sce->Section.endPosition = 0x30;
			return true;

		case TriggerFeedback::Resistance::SectionHeavyResistance:
			sce->effectType = DS5W::TriggerEffectType::SectionResitance;
			sce->Section.startPosition = 0x00;
			sce->Section.endPosition = 0x60;
			return true;
		}
	}

	return false;
}


void Components::GamepadControls::DualSenseGamePadAPI::UpdateForceFeedback(const TriggerFeedback& left, const TriggerFeedback& right)
{
	if (EnsureConnected())
	{
		if (ToSCE(left, &outState.leftTriggerEffect) && ToSCE(right, &outState.rightTriggerEffect))
		{
			DS5W::setDeviceOutputState(&context, &outState);
		}
	}
}

bool Components::GamepadControls::DualSenseGamePadAPI::Fetch()
{
	if (EnsureConnected())
	{
		if (DS5W_SUCCESS(DS5W::getDeviceInputState(&context, &inputState)))
		{
			return true;
		}
	}

	return false;
}

void Components::GamepadControls::DualSenseGamePadAPI::ReadSticks(Game::vec2_t& leftStick, Game::vec2_t& rightStick)
{
	ConvertStickToFloat(inputState.leftStick.x, inputState.leftStick.y, leftStick[0], leftStick[1]);
	ConvertStickToFloat(inputState.rightStick.x, inputState.rightStick.y, rightStick[0], rightStick[1]);
}

void Components::GamepadControls::DualSenseGamePadAPI::ReadDigitals(unsigned short& digitals)
{
	auto buttons = inputState.buttonsAndDpad;

	digitals = 0;

#define TRANSLATE(x, ds5Button)\
	if ((buttons & ds5Button) != 0) \
	{\
		digitals |= Game::GamePadButton::x;\
	}

	TRANSLATE(GPAD_UP, DS5W_ISTATE_DPAD_UP);
	TRANSLATE(GPAD_DOWN, DS5W_ISTATE_DPAD_DOWN);
	TRANSLATE(GPAD_RIGHT, DS5W_ISTATE_DPAD_RIGHT);
	TRANSLATE(GPAD_LEFT, DS5W_ISTATE_DPAD_LEFT);
	TRANSLATE(GPAD_X, DS5W_ISTATE_BTX_SQUARE);
	TRANSLATE(GPAD_Y, DS5W_ISTATE_BTX_TRIANGLE);
	TRANSLATE(GPAD_B, DS5W_ISTATE_BTX_CIRCLE);
	TRANSLATE(GPAD_A, DS5W_ISTATE_BTX_CROSS);

	buttons = inputState.buttonsA;
	TRANSLATE(GPAD_START, DS5W_ISTATE_BTN_A_MENU);
	TRANSLATE(GPAD_BACK, DS5W_ISTATE_BTN_A_SELECT);
	TRANSLATE(GPAD_L3, DS5W_ISTATE_BTN_A_LEFT_STICK);
	TRANSLATE(GPAD_R3, DS5W_ISTATE_BTN_A_RIGHT_STICK);
	
	TRANSLATE(GPAD_L_SHLDR, DS5W_ISTATE_BTN_A_LEFT_BUMPER);
	TRANSLATE(GPAD_R_SHLDR, DS5W_ISTATE_BTN_A_RIGHT_BUMPER);

#undef TRANSLATE
}

void Components::GamepadControls::DualSenseGamePadAPI::ReadAnalogs(float& leftTrigger, float& rightTrigger)
{
	ConvertStickToFloat(inputState.leftTrigger, inputState.rightTrigger, leftTrigger, rightTrigger);
}

