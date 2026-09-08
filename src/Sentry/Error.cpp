#include "Error.hpp"

namespace Sentry
{
  const char*
  to_string (const errc code) noexcept
  {
    switch (code)
    {
    case errc::none:                  return "none";
    case errc::not_configured:        return "not-configured";
    case errc::already_started:       return "already-started";
    case errc::daemon_missing:        return "daemon-missing";
    case errc::database_unusable:     return "database-unusable";
    case errc::initialization_failed: return "initialization-failed";
    case errc::shutdown_failed:       return "shutdown-failed";
    case errc::scope_rejected:        return "scope-rejected";
    case errc::attachment_missing:    return "attachment-missing";
    case errc::dvar_registration:     return "dvar-registration";
    case errc::filter_unchained:      return "filter-unchained";
    }

    return "unknown";
  }

  std::ostream&
  operator<< (std::ostream& os, const errc code)
  {
    return os << to_string (code);
  }

  error::
  error (const errc code, const std::string& what)
    : std::runtime_error (what), code_ (code) {}

  error::
  error (const errc code, const char* what)
    : std::runtime_error (what), code_ (code) {}
}
