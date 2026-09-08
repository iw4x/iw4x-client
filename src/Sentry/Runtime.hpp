#pragma once

#include "Types.hpp"

#include "Breadcrumb.hpp"
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

    void
    shutdown ();

  private:
    bool
    start ();

    static void
    forward (sentry_level_t, const char*, va_list, void*);

    static void
    recovered (const sentry_envelope_t*, void*);

    logging_sink sink_;
    options options_;
    context ctx_;
    scope scope_;

    bool started_ {false};
    unsigned recovered_ {0};
  };
}
