#pragma once

#include "Types.hpp"

#include "Breadcrumb.hpp"
#include "Console.hpp"
#include "Context.hpp"
#include "Diagnostic.hpp"
#include "Options.hpp"
#include "Scope.hpp"

namespace Sentry
{
  class runtime
  {
  public:
    explicit
    runtime (options);

    ~runtime ();

    runtime (const runtime&) = delete;
    runtime& operator= (const runtime&) = delete;
    runtime (runtime&&) = delete;
    runtime& operator= (runtime&&) = delete;

    bool
    started () const noexcept {return started_;}

    const options&
    configuration () const noexcept {return options_;}

    diagnostic_sink&
    diagnostics () noexcept {return sink_;}

    void
    engine_ready ();

    void
    refresh ();

    void
    capture (sentry_level_t, const std::string& logger,
             const std::string& message);

    void
    leave (trail, sentry_level_t, const std::string& message);

    // Add console output to what we attach to reports. This may be called
    // from any thread.
    //
    void
    print (std::string_view);

    void
    shutdown ();

  private:
    bool
    start ();

    static void
    forward (sentry_level_t, const char*, va_list, void*);

    static void
    recovered (const sentry_envelope_t*, void*);

    static sentry_value_t
    before_send (sentry_value_t event, void* hint, void* data);

    // Write the console output we attach if it changed since. If wait is
    // false, then give up rather than wait for another thread.
    //
    void
    write_console (bool wait);

    // Return the process context: uptime, time on the current map, and
    // address space usage. This may be called from any thread.
    //
    sentry_value_t
    process () const;

    logging_sink sink_;
    options options_;
    context ctx_;
    scope scope_;

    bool started_ {false};
    unsigned recovered_ {0};

    // When we started and when the current map was loaded (or its zero
    // value if there is none), for the process context. Note that we only
    // notice a map change on refresh.
    //
    const std::chrono::steady_clock::time_point started_at_ {
      std::chrono::steady_clock::now ()};

    std::string map_;
    std::atomic<std::chrono::steady_clock::rep> map_at_ {0};

    console console_ {256 * 1024};
    std::mutex console_mutex_;
    uint64_t console_written_ {0}; // Under console_mutex_.
  };
}
