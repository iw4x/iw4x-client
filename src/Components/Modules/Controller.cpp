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

	namespace
	{
		float submittedLowRumble = 0.0f;
		float submittedHighRumble = 0.0f;
		bool rumbleSubmitted = false;
	}

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

		const auto& dvars = theRuntime->dvars();

		const auto enabled = ::Controller::engine::read(dvars.rumble, true);

		const auto scale = [](const float rumble, Game::dvar_t* dvar)
		{
			return std::clamp(rumble * std::clamp(::Controller::engine::read(dvar, 1.0f), 0.0f, 1.0f), 0.0f, 1.0f);
		};

		const auto low = enabled ? scale(lowRumble, dvars.rumble_scale_low) : 0.0f;
		const auto high = enabled ? scale(highRumble, dvars.rumble_scale_high) : 0.0f;

		if (rumbleSubmitted && low == submittedLowRumble && high == submittedHighRumble)
			return;

		if (!theRuntime->submit(::Controller::driver::rumble_request{low, high}))
			return;

		submittedLowRumble = low;
		submittedHighRumble = high;
		rumbleSubmitted = true;
	}

	void Controller::PlayHapticEffect(const ::Controller::haptic::effect& effect)
	{
		if (theRuntime == nullptr ||
			!::Controller::engine::read(theRuntime->dvars().rumble, true) ||
			!::Controller::engine::read(theRuntime->dvars().haptics, true))
			return;

		if (!theRuntime->supports_haptics())
			return;

		const auto intensity = std::clamp(
			::Controller::engine::read(theRuntime->dvars().haptic_intensity, 1.0f), 0.0f, 1.0f);

		if (intensity <= 0.0f)
			return;

		auto scaled = effect;
		scaled.intensity *= intensity;

		theRuntime->submit(scaled);
	}

	void Controller::StopHapticEffect(const std::uint32_t tag)
	{
		if (theRuntime != nullptr)
			theRuntime->stop_haptic(tag);
	}

	void Controller::GPad_SetLowRumble(int, const double rumble)
	{
		lowRumble = static_cast<float>(std::clamp(rumble, 0.0, 1.0));
		SubmitRumble();
	}

	void Controller::GPad_SetHighRumble(int, const double rumble)
	{
		highRumble = static_cast<float>(std::clamp(rumble, 0.0, 1.0));
		SubmitRumble();
	}

	void Controller::GPad_StopRumbles(int)
	{
		lowRumble = 0.0f;
		highRumble = 0.0f;

		SubmitRumble();
	}

	void Controller::GPad_UpdateFeedbacks()
	{
		SubmitRumble();
	}

	Controller::Controller()
	{
		if (ZoneBuilder::IsEnabled())
			return;

		sv_allowAimAssist = Dvar::Register<bool>("sv_allowAimAssist", true, Game::DVAR_SYSTEMINFO,
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
