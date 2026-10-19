#pragma once

#include "Types.hpp"

namespace Sentry
{
  struct options
  {
    std::string dsn;
    std::string release;
    std::string environment;
    std::string dist;

    std::filesystem::path database;
    std::filesystem::path daemon;
    std::filesystem::path reporter;

    sentry_minidump_mode_t minidump {SENTRY_MINIDUMP_MODE_SMART};

    // Raw MINIDUMP_TYPE flags which, if present, take precedence over the
    // minidump mode.
    //
    std::optional<sentry_minidump_flags_t> minidump_flags;
    sentry_crash_reporting_mode_t reporting {
      SENTRY_CRASH_REPORTING_MODE_NATIVE_WITH_MINIDUMP};

    size_t breadcrumbs {128};
    std::chrono::milliseconds shutdown_timeout {2000};
    std::chrono::milliseconds refresh_interval {5000};

    bool debug {false};
    // Session tracking reports every run, crash or not, which is not
    // something we send without asking.
    //
    bool sessions {false};

    bool
    configured () const noexcept {return !dsn.empty ();}
  };

  options
  discover ();

  std::filesystem::path
  module_directory ();
}
