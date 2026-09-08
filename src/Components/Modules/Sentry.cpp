#include "Sentry.hpp"

#include "Events.hpp"
#include "Exception.hpp"
#include "Scheduler.hpp"

#include "../../Sentry/Engine/Dvar.hpp"

namespace Components
{
	namespace
	{
		std::unique_ptr<::Sentry::runtime> theRuntime;

		struct ErrorClass
		{
			Game::errorParm_t parm;
			const char* name;
			sentry_level_t level;
		};

		constexpr ErrorClass errorClasses[] =
		{
			{Game::ERR_FATAL,                "fatal",         SENTRY_LEVEL_FATAL},
			{Game::ERR_DROP,                 "drop",          SENTRY_LEVEL_ERROR},
			{Game::ERR_SERVERDISCONNECT,     "serverdrop",    SENTRY_LEVEL_ERROR},
			{Game::ERR_DISCONNECT,           "disconnect",    SENTRY_LEVEL_WARNING},
			{Game::ERR_SCRIPT,               "script",        SENTRY_LEVEL_ERROR},
			{Game::ERR_SCRIPT_DROP,          "scriptdrop",    SENTRY_LEVEL_ERROR},
			{Game::ERR_LOCALIZATION,         "localization",  SENTRY_LEVEL_WARNING},
			{Game::ERR_MAPLOADERRORSUMMARY,  "maploadsummary", SENTRY_LEVEL_ERROR},
		};

		const ErrorClass& Classify(const Game::errorParm_t parm)
		{
			for (const auto& entry : errorClasses)
			{
				if (entry.parm == parm)
				{
					return entry;
				}
			}

			return errorClasses[0];
		}
	}

	::Sentry::runtime* Sentry::Runtime()
	{
		return theRuntime.get();
	}

	void Sentry::CaptureError(const Game::errorParm_t error, const std::string& message)
	{
		if (theRuntime == nullptr)
		{
			return;
		}

		const auto& classification = Classify(error);

		theRuntime->capture(classification.level,
			std::format("com_error.{}", classification.name), message);
	}

	void Sentry::Leave(const ::Sentry::trail trail, const sentry_level_t level, const std::string& message)
	{
		if (theRuntime != nullptr)
		{
			theRuntime->leave(trail, level, message);
		}
	}

	Sentry::Sentry()
	{
		Exception::ChainFilterInstalledBy([]
		{
			theRuntime = std::make_unique<::Sentry::runtime>(::Sentry::discover());
		});

		if (theRuntime == nullptr || !theRuntime->started())
		{
			return;
		}

		Events::OnDvarInit([]
		{
			::Sentry::engine::publish_status(theRuntime->configuration());
		});

		Scheduler::OnGameInitialized([]
		{
			theRuntime->engine_ready();
		}, Scheduler::Pipeline::MAIN);

		Scheduler::Loop([]
		{
			theRuntime->refresh();
		}, Scheduler::Pipeline::MAIN, theRuntime->configuration().refresh_interval);

		Scheduler::OnGameShutdown([]
		{
			if (theRuntime != nullptr)
			{
				theRuntime->shutdown();
			}
		});
	}

	Sentry::~Sentry()
	{
		theRuntime.reset();
	}
}
