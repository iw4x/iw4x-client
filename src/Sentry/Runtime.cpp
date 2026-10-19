#include "Runtime.hpp"

#include "Types.hpp"

#include "Attachment.hpp"
#include "Redact.hpp"
#include "Report.hpp"
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

  sentry_value_t
  runtime::
  before_send (sentry_value_t e, void*, void* d)
  {
    runtime* r (static_cast<runtime*> (d));

    if (r == nullptr)
      return e;

    // Note that we may be crashing, possibly in the middle of a print, so
    // don't wait for anything.
    //
    r->write_console (false /* wait */);

    sentry_value_t cs (sentry_value_get_by_key (e, "contexts"));

    if (sentry_value_is_null (cs))
    {
      cs = sentry_value_new_object ();
      sentry_value_set_by_key (e, "contexts", cs);
    }

    sentry_value_set_by_key (cs, "process", r->process ());

    return e;
  }

  sentry_value_t
  runtime::
  process () const
  {
    using namespace std::chrono;

    sentry_value_t r (sentry_value_new_object ());

    auto set = [r] (const char* k, const int64_t v)
    {
      sentry_value_set_by_key (
        r, k, sentry_value_new_int32 (static_cast<int32_t> (v)));
    };

    const steady_clock::time_point now (steady_clock::now ());

    set ("uptime_seconds",
         duration_cast<seconds> (now - started_at_).count ());

    if (const steady_clock::rep m = map_at_.load (); m != 0)
    {
      const steady_clock::duration d (now.time_since_epoch () -
                                      steady_clock::duration (m));

      set ("map_seconds", duration_cast<seconds> (d).count ());
    }

    // We are a 32-bit process, so running out of address space (rather than
    // memory) is a common way to crash with a large mod or map.
    //
    MEMORYSTATUSEX s {};
    s.dwLength = sizeof (s);

    if (GlobalMemoryStatusEx (&s) != FALSE)
    {
      constexpr uint64_t mib (1024 * 1024);

      set ("address_space_used_mib",
           (s.ullTotalVirtual - s.ullAvailVirtual) / mib);
      set ("address_space_total_mib", s.ullTotalVirtual / mib);
    }

    return r;
  }

  void
  runtime::
  write_console (const bool wait)
  {
    std::unique_lock l (console_mutex_, std::defer_lock);

    if (wait)
      l.lock ();
    else if (!l.try_lock ())
      return;

    std::optional<std::pair<std::string, uint64_t>> s;

    if (wait)
      s = console_.snapshot ();
    else
      s = console_.try_snapshot ();

    if (!s || s->second == console_written_)
      return;

    // The console shows whatever the player types, which includes rcon
    // passwords.
    //
    if (write_generated (options_.database,
                         "console.log",
                         redact_lines (std::move (s->first))))
      console_written_ = s->second;
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

    // Note that we hand the reporter over even if it is missing: the daemon
    // then fails to launch it and drops the report, which is what we want
    // since crashes are only sent once the player agrees to it.
    //
    if (!std::filesystem::exists (options_.reporter))
      ctx_.report (severity::warning, facility::daemon, errc::reporter_missing,
                   std::format ("{} is missing, so crashes that end the "
                                "process will go unreported",
                                options_.reporter.string ()));

    sentry_options_t* o (sentry_options_new ());

    sentry_options_set_dsn (o, options_.dsn.c_str ());
    sentry_options_set_release (o, options_.release.c_str ());
    sentry_options_set_environment (o, options_.environment.c_str ());
    sentry_options_set_dist (o, options_.dist.c_str ());

    sentry_options_set_database_pathw (o, options_.database.c_str ());
    sentry_options_set_handler_pathw (o, options_.daemon.c_str ());
    sentry_options_set_external_crash_reporter_pathw (
      o, options_.reporter.c_str ());

    // Everything else that we would send goes through the reporter as well,
    // so that nothing leaves without the player agreeing to it.
    //
    sentry_options_set_transport (o, make_transport (options_));

    sentry_options_set_crash_reporting_mode (o, options_.reporting);
    sentry_options_set_minidump_mode (o, options_.minidump);

    if (options_.minidump_flags)
      sentry_options_set_minidump_flags (o, *options_.minidump_flags);

    sentry_options_set_max_breadcrumbs (o, options_.breadcrumbs);
    sentry_options_set_shutdown_timeout (
      o, static_cast<uint64_t> (options_.shutdown_timeout.count ()));
    sentry_options_set_auto_session_tracking (o, options_.sessions ? 1 : 0);

    sentry_options_set_enable_app_hang_tracking (o, 1);
    sentry_options_set_app_hang_timeout (
      o, static_cast<uint64_t> (options_.hang_timeout.count ()));
    sentry_options_set_require_user_consent (o, 0);

    sentry_options_set_logger (o, &runtime::forward, this);
    sentry_options_set_debug (o, options_.debug ? 1 : 0);
    sentry_options_set_on_crashed_last_run (o, &runtime::recovered, this);
    sentry_options_set_before_send (o, &runtime::before_send, this);

    if (sentry_init (o) != 0)
    {
      ctx_.report (severity::error, facility::runtime,
                   errc::initialization_failed, "the SDK refused to start");
      return false;
    }

    attach (ctx_, engine::base_path (), options_.database);

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
    if (!started_)
      return;

    scope_.publish ();
    refresh_snapshots (ctx_, options_.database);
    write_console (true /* wait */);

    if (std::string m (engine::map ()); m != map_)
    {
      map_at_.store (
        m.empty ()
        ? 0
        : std::chrono::steady_clock::now ().time_since_epoch ().count ());

      map_ = std::move (m);
    }
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
  heartbeat ()
  {
    if (!started_)
      return;

    // A debugger may stop the thread for as long as it likes, which is not
    // a freeze. Note that the next heartbeat resumes detection.
    //
    if (IsDebuggerPresent () != FALSE)
      sentry_app_hang_pause ();
    else
      sentry_app_hang_heartbeat ();
  }

  void
  runtime::
  print (const std::string_view s)
  {
    console_.append (s);
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
