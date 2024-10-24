#pragma once
#include "GamepadAPI.hpp"
#include "DualSenseAPI.hpp"

namespace Components::GamepadControls
{
	class DualSenseGamePadAPI final : public GamepadAPI
	{
	public:
		bool PlugIn(uint8_t portIndex) override;
		void UpdateRumbles(float left, float right) override;
		void UpdateForceFeedback(const TriggerFeedback& left, const TriggerFeedback& right) override;
		void UpdateLights(uint32_t color) override;

		bool Fetch() override;
		void ReadSticks(Game::vec2_t& leftStick, Game::vec2_t& rightStick) override;
		void ReadDigitals(unsigned short& digitals) override;
		void ReadAnalogs(float& leftTrigger, float& rightTrigger) override;

		void Send() override;

		bool SupportsForceFeedback()  const override {return true;};

	private:
		DS5W::DeviceContext context;
		DS5W::DS5InputState inputState;
		DS5W::DS5OutputState outState;

		uint8_t portIndex;

		bool dirty;

		bool EnsureConnected();
		bool ToSCE(const TriggerFeedback& triggerFeedback, DS5W::TriggerEffect* sce);
		inline bool AreEqual(const DS5W::TriggerEffect& a, const DS5W::TriggerEffect& b);
		inline bool AreEqual(const DS5W::Color& a, const DS5W::Color& b);
		bool ToSCE(const float & amount01, unsigned char & amount);
	};
}
