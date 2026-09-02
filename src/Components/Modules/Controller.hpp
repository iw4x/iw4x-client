#pragma once

#include "../../Controller/Runtime.hpp"
#include "../../Controller/Haptic/Effect.hpp"

namespace Components
{
	class Controller : public Component
	{
	public:
		static const int RUMBLE_CONFIGSTRINGS_COUNT = 32;

		Controller();
		~Controller();

		static ::Controller::runtime* Runtime();

		static bool IsControllerInUse();
		static void OnMouseMove(int dx, int dy);

		static void PlayHapticEffect(const ::Controller::haptic::effect& effect);
		static void StopHapticEffect(std::uint32_t tag);

		static void GPad_SetLowRumble(int gamePadIndex, double rumble);
		static void GPad_SetHighRumble(int gamePadIndex, double rumble);
		static void GPad_StopRumbles(int gamePadIndex);
		static void GPad_UpdateFeedbacks();

		static Dvar::Var sv_allowAimAssist;

	private:
		static float lowRumble;
		static float highRumble;

		static void SubmitRumble();
	};
}
