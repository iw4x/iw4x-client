#include "Runtime.hpp"

#include "Types.hpp"

#include "Attachment.hpp"
#include "Engine/State.hpp"

namespace Sentry
{
  namespace
  {
    severity
    severity_of (const sentry_level_t l) noexcept
    {
      switch (l)
      {
      case SENTRY_LEVEL_FATAL:
      case SENTRY_LEVEL_ERROR:   return severity::error;
      case SENTRY_LEVEL_WARNING: return severity::warning;
      default:                   return severity::info;
      }
    }
  }

  runtime::
  runtime (options o)
    : options_ (std::move (o)),
      ctx_ (sink_, options_.debug)
  {
    started_ = start ();
  }

  runtime::
  ~runtime ()
  {
    shutdown ();
  }

  void
  runtime::
  forward (const sentry_level_t l, const char* m, va_list a, void* d)
  {
    runtime* r (static_cast<runtime*> (d));

    if (r == nullptr || m == nullptr)
      return;

    char b[2048] {};

    if (vsnprintf_s (b, _TRUNCATE, m, a) < 0)
      return;

    r->ctx_.report (severity_of (l), facility::sdk, errc::none, b);
  }

  void
  runtime::
  recovered (const sentry_envelope_t*, void* d)
  {
    runtime* r (static_cast<runtime*> (d));

    if (r != nullptr)
      ++r->recovered_;
  }

  bool
  runtime::
  start ()
  {
    if (!options_.configured ())
    {
      ctx_.report (severity::info, facility::options, errc::not_configured,
                   "reporting is off because this build carries no DSN");
      return false;
    }

    std::error_code ec;
    std::filesystem::create_directories (options_.database, ec);

    if (ec)
    {
      ctx_.report (severity::error, facility::options, errc::database_unusable,
                   std::format ("{} is not usable: {}",
                                options_.database.string (), ec.message ()));
      return false;
    }

    if (!std::filesystem::exists (options_.daemon))
      ctx_.report (severity::warning, facility::daemon, errc::daemon_missing,
                   std::format ("{} is missing, so crashes that end the "
                                "process will go unreported",
                                options_.daemon.string ()));

    sentry_options_t* o (sentry_options_new ());

    sentry_options_set_dsn (o, options_.dsn.c_str ());
    sentry_options_set_release (o, options_.release.c_str ());
    sentry_options_set_environment (o, options_.environment.c_str ());
    sentry_options_set_dist (o, options_.dist.c_str ());

    sentry_options_set_database_pathw (o, options_.database.c_str ());
    sentry_options_set_handler_pathw (o, options_.daemon.c_str ());

    sentry_options_set_crash_reporting_mode (o, options_.reporting);
    sentry_options_set_minidump_mode (o, options_.minidump);

    sentry_options_set_max_breadcrumbs (o, options_.breadcrumbs);
    sentry_options_set_shutdown_timeout (
      o, static_cast<uint64_t> (options_.shutdown_timeout.count ()));
    sentry_options_set_auto_session_tracking (o, options_.sessions ? 1 : 0);
    sentry_options_set_require_user_consent (o, 0);

    sentry_options_set_logger (o, &runtime::forward, this);
    sentry_options_set_debug (o, options_.debug ? 1 : 0);
    sentry_options_set_on_crashed_last_run (o, &runtime::recovered, this);

    if (sentry_init (o) != 0)
    {
      ctx_.report (severity::error, facility::runtime,
                   errc::initialization_failed, "the SDK refused to start");
      return false;
    }

    attach (ctx_, engine::base_path ());

    scope_.publish ();
    record (trail::lifecycle, SENTRY_LEVEL_INFO, "the client started");

    ctx_.report (severity::info, facility::runtime, errc::none,
                 std::format ("reporting {} to the {} environment as {}",
                              options_.release, options_.environment,
                              options_.dist));

    if (recovered_ != 0)
      ctx_.report (severity::warning, facility::runtime, errc::none,
                   std::format ("{} crash report(s) were recovered from the "
                                "previous run", recovered_));

    return true;
  }

  void
  runtime::
  engine_ready ()
  {
    if (!started_)
      return;

    refresh ();
    record (trail::lifecycle, SENTRY_LEVEL_INFO, "the game finished loading");
  }

  void
  runtime::
  refresh ()
  {
    if (started_)
      scope_.publish ();
  }

  void
  runtime::
  capture (const sentry_level_t l, const std::string& n, const std::string& m)
  {
    if (!started_)
      return;

    sentry_capture_event (
      sentry_value_new_message_event (l, n.c_str (), m.c_str ()));
  }

  void
  runtime::
  leave (const trail t, const sentry_level_t l, const std::string& m)
  {
    if (started_)
      record (t, l, m);
  }

  void
  runtime::
  shutdown ()
  {
    if (!started_)
      return;

    record (trail::lifecycle, SENTRY_LEVEL_INFO, "the client is shutting down");

    started_ = false;

    if (sentry_close () != 0)
      ctx_.report (severity::warning, facility::runtime, errc::shutdown_failed,
                   "the SDK did not shut down cleanly");
  }
}
