#pragma once

#include "../../Sentry/Runtime.hpp"

namespace Components
{
	class Sentry : public Component
	{
	public:
		Sentry();
		~Sentry();

		static ::Sentry::runtime* Runtime();

		static void CaptureError(Game::errorParm_t error, const std::string& message);
		static void Leave(::Sentry::trail trail, sentry_level_t level, const std::string& message);
	};
}
