#include "DualSenseGamepad.hpp"
#include "XInputGamepad.hpp"

#define PUBLIC_GET_PRIVATE_SET(type, name) \
		private:\
			type name;\
		public:\
			type get_##name () const { return name; }

// type (*name(void))dimensions {\  won't work :(
#define PUBLIC_GET_PRIVATE_SET_ARRAY(type, name, dimensions) \
		private:\
			type name dimensions;\
		public:\
			const type* get_##name() {\
		return name; \
}

namespace Components::GamepadControls
{
	class Controller
	{

	public:
		static void InitializeDvars();

		bool PlugIn(uint8_t);

		void SetLowRumble(double rumble);

		void SetHighRumble(double rumble);

		void SetForceFeedback(const GamepadAPI::TriggerFeedback& left, const GamepadAPI::TriggerFeedback& right);

		void StopRumbles();

		void UpdateState(bool additive=false);
		void PushUpdates();

		float GetStick(const Game::GamePadStick stick);
		float GetButton(Game::GamePadButton button);
		bool ButtonRequiresUpdates(Game::GamePadButton button);
		bool IsButtonReleased(Game::GamePadButton button);
		bool IsButtonPressed(Game::GamePadButton button);

		static Dvar::Var gpad_debug;

		bool inUse;

	private:

		static Dvar::Var gpad_stick_pressed_hysteresis;
		static Dvar::Var gpad_stick_pressed;
		static Dvar::Var gpad_stick_deadzone_max;
		static Dvar::Var gpad_stick_deadzone_min;
		static Dvar::Var gpad_button_deadzone;
		static Dvar::Var gpad_button_rstick_deflect_max;
		static Dvar::Var gpad_button_lstick_deflect_max;
		static Dvar::Var gpad_allow_force_feedback;
		static Dvar::Var gpad_force_xinput_only;

		void UpdateDigitals(bool additive);
		void UpdateAnalogs();
		void UpdateSticks(bool additive);
		void ApplyDeadzone(Game::vec2_t& stick);
		void UpdateSticksDown(bool additive);

		bool stickDown[4][Game::GPAD_STICK_DIR_COUNT];
		bool stickDownLast[4][Game::GPAD_STICK_DIR_COUNT];

	public:
		bool get_stickDown(int stickIndex, Game::GamePadStickDir dir) { return stickDown[stickIndex][dir];}
		bool get_stickDownLast(int stickIndex, Game::GamePadStickDir dir) { return stickDownLast[stickIndex][dir];}

		PUBLIC_GET_PRIVATE_SET(GamepadAPI::TriggerFeedback, leftForceFeedback);
		PUBLIC_GET_PRIVATE_SET(GamepadAPI::TriggerFeedback, rightForceFeedback);

		PUBLIC_GET_PRIVATE_SET(bool, enabled);
		PUBLIC_GET_PRIVATE_SET(int, portIndex);
		PUBLIC_GET_PRIVATE_SET(unsigned short, digitals);
		PUBLIC_GET_PRIVATE_SET(unsigned short, lastDigitals);
		PUBLIC_GET_PRIVATE_SET(float, lowRumble);
		PUBLIC_GET_PRIVATE_SET(float, highRumble);

		PUBLIC_GET_PRIVATE_SET_ARRAY(float, analogs, [2]);
		PUBLIC_GET_PRIVATE_SET_ARRAY(float, lastAnalogs, [2]);
		PUBLIC_GET_PRIVATE_SET_ARRAY(float, sticks, [4]);
		PUBLIC_GET_PRIVATE_SET_ARRAY(float, lastSticks, [4]);


	private:
		std::unique_ptr<GamepadControls::GamepadAPI> api;
	};
}


#undef PUBLIC_GET_PRIVATE_SET
#undef PUBLIC_GET_PRIVATE_SET_ARRAY
