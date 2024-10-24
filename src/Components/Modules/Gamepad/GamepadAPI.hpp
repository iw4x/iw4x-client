#pragma once

namespace Components::GamepadControls
{
	class GamepadAPI
	{
	public:

		struct TriggerFeedback
		{
			enum Resistance
			{
				NoResistance,
				ContinuousSlightResistance,
				ContinuousHeavyResistance,
				SectionSlightResistance,
				SectionHeavyResistance
			};

			Resistance resistance;
		};

		virtual bool Fetch() { return false; };
		virtual void ReadSticks([[maybe_unused]] Game::vec2_t& leftStick, [[maybe_unused]] Game::vec2_t& rightStick) {};
		virtual void ReadDigitals([[maybe_unused]] unsigned short& digitals) {};
		virtual void ReadAnalogs([[maybe_unused]] float& leftTrigger, [[maybe_unused]] float& rightTrigger) {};

		virtual bool PlugIn([[maybe_unused]] uint8_t portIndex) { return false; };
		virtual bool SupportsForceFeedback() const { return false; };

		virtual void UpdateRumbles([[maybe_unused]] float left, [[maybe_unused]] float right) {};
		virtual void UpdateLights([[maybe_unused]] uint32_t color) {};
		virtual void StopRumbles() {};
		virtual void Send() {};

		virtual void UpdateForceFeedback([[maybe_unused]] const TriggerFeedback& left, [[maybe_unused]] const TriggerFeedback& right) {};

	protected:
		template <typename T>
		void ConvertStickToFloat(const T x, const T y, float& outX, float& outY)
		{
			if (x == 0 && y == 0)
			{
				outX = 0.0f;
				outY = 0.0f;
				return;
			}

			Game::vec2_t stickVec;
			stickVec[0] = static_cast<float>(x) / static_cast<float>(std::numeric_limits<T>::max());
			stickVec[1] = static_cast<float>(y) / static_cast<float>(std::numeric_limits<T>::max());

			outX = stickVec[0];
			outY = stickVec[1];
		}

	};
}
