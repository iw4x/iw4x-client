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

    sentry_minidump_mode_t minidump {SENTRY_MINIDUMP_MODE_SMART};
    sentry_crash_reporting_mode_t reporting {
      SENTRY_CRASH_REPORTING_MODE_NATIVE_WITH_MINIDUMP};

    size_t breadcrumbs {128};
    std::chrono::milliseconds shutdown_timeout {2000};
    std::chrono::milliseconds refresh_interval {5000};

    bool debug {false};
    bool sessions {true};

    bool
    configured () const noexcept {return !dsn.empty ();}
  };

  options
  discover ();

  std::filesystem::path
  module_directory ();
}
