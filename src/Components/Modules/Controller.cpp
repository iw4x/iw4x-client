#include "Controller.hpp"

#include "Dedicated.hpp"
#include "ZoneBuilder.hpp"

#include "../../Controller/Engine/Hook.hpp"

namespace Components
{
	namespace
	{
		std::unique_ptr<::Controller::runtime> theRuntime;
	}

	Dvar::Var Controller::sv_allowAimAssist;

	float Controller::lowRumble = 0.0f;
	float Controller::highRumble = 0.0f;

	::Controller::runtime* Controller::Runtime()
	{
		return theRuntime.get();
	}

	bool Controller::IsControllerInUse()
	{
		return theRuntime != nullptr && theRuntime->keys().in_use();
	}

	void Controller::OnMouseMove(const int dx, const int dy)
	{
		::Controller::engine::note_mouse_move(dx, dy);
	}

	void Controller::SubmitRumble()
	{
		if (theRuntime == nullptr)
			return;

		if (!::Controller::engine::read(theRuntime->dvars().rumble, true))
			return;

		theRuntime->submit(::Controller::driver::rumble_request{lowRumble, highRumble});
	}

	void Controller::PlayHapticEffect(const ::Controller::haptic::effect& effect)
	{
		if (theRuntime == nullptr ||
			!::Controller::engine::read(theRuntime->dvars().rumble, true) ||
			!::Controller::engine::read(theRuntime->dvars().haptics, true))
			return;

		theRuntime->submit(effect);
	}

	void Controller::StopHapticEffect(const std::uint32_t tag)
	{
		if (theRuntime != nullptr)
			theRuntime->stop_haptic(tag);
	}

	void Controller::GPad_SetLowRumble(int, const double rumble)
	{
		const auto value = static_cast<float>(std::clamp(rumble, 0.0, 1.0));

		if (value == lowRumble)
			return;

		lowRumble = value;
		SubmitRumble();
	}

	void Controller::GPad_SetHighRumble(int, const double rumble)
	{
		const auto value = static_cast<float>(std::clamp(rumble, 0.0, 1.0));

		if (value == highRumble)
			return;

		highRumble = value;
		SubmitRumble();
	}

	void Controller::GPad_StopRumbles(int)
	{
		if (lowRumble == 0.0f && highRumble == 0.0f)
			return;

		lowRumble = 0.0f;
		highRumble = 0.0f;

		if (theRuntime != nullptr)
			theRuntime->submit(::Controller::driver::rumble_request{});
	}

	void Controller::GPad_UpdateFeedbacks()
	{
		if (theRuntime == nullptr)
			return;

		if (!::Controller::engine::read(theRuntime->dvars().rumble, true))
			GPad_StopRumbles(0);
	}

	Controller::Controller()
	{
		if (ZoneBuilder::IsEnabled())
			return;

		sv_allowAimAssist = Dvar::Register<bool>("sv_allowAimAssist", true, Game::DVAR_NONE,
			"Controls whether aim assist features on clients are enabled");

		if (Dedicated::IsEnabled())
		{
			::Controller::engine::install_protocol();
			return;
		}

		if (theRuntime != nullptr)
			return;

#ifdef _DEBUG
		constexpr bool developer = true;
#else
		constexpr bool developer = false;
#endif

		theRuntime = std::make_unique<::Controller::runtime>(developer);
		::Controller::engine::install(*theRuntime);
	}

	Controller::~Controller()
	{
	}
}
