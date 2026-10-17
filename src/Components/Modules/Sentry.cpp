#include "Sentry.hpp"

#include "Events.hpp"
#include "Exception.hpp"
#include "Scheduler.hpp"
#include "TextRenderer.hpp"

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

    // Sys_Error shows the error in a dialog and then keeps the game around
    // until it is closed, which is neither a freeze nor something that goes
    // through Com_Error. So capture it ourselves before showing the dialog.
    //
    int ShowErrorDialog(const char* message)
    {
      sentry_app_hang_pause();

      theRuntime->capture(SENTRY_LEVEL_FATAL, "sys_error", message);

      return Utils::Hook::Call<int(const char*)>(0x433940)(message);
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

    // Only a fatal error ends the game, which is worth asking the player to
    // report. The others (being kicked, a script error, etc.) are common
    // enough that asking every time would only get in the way, so we keep
    // them as breadcrumbs for when something does go wrong.
    //
    if (error != Game::ERR_FATAL)
    {
      theRuntime->leave(::Sentry::trail::error, classification.level,
        std::format("com_error.{}: {}", classification.name, message));
      return;
    }

    // The game now waits for the player to dismiss the error, which is not
    // a freeze.
    //
    sentry_app_hang_pause();

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

  void Sentry::Print(const char* message)
  {
    if (theRuntime != nullptr && message != nullptr)
    {
      theRuntime->print(TextRenderer::StripColors(message));
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

    // Note that the first heartbeat decides which thread is watched, so it
    // has to come from the main thread.
    //
    Scheduler::Loop([]
    {
      if (Game::Sys_IsMainThread())
      {
        theRuntime->heartbeat();
      }
    }, Scheduler::Pipeline::MAIN, 1s);

    Utils::Hook(0x43D5DF, ShowErrorDialog, HOOK_CALL).install()->quick(); // Sys_Error

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
