#include "Error.hpp"

#include "Types.hpp"

namespace Controller
{
  const char*
  to_string (errc c) noexcept
  {
    switch (c)
    {
      case errc::none:                return "none";
      case errc::device_unavailable:  return "device-unavailable";
      case errc::ambiguous_identity:  return "ambiguous-identity";
      case errc::unsupported_device:  return "unsupported-device";
      case errc::transport_failure:   return "transport-failure";
      case errc::report_malformed:    return "report-malformed";
      case errc::report_truncated:    return "report-truncated";
      case errc::checksum_mismatch:   return "checksum-mismatch";
      case errc::output_rejected:     return "output-rejected";
      case errc::calibration_invalid: return "calibration-invalid";
      case errc::calibration_version: return "calibration-version";
      case errc::graph_invalid:       return "graph-invalid";
      case errc::binding_invalid:     return "binding-invalid";
      case errc::steam_unavailable:   return "steam-unavailable";
      case errc::steam_unsuitable:    return "steam-unsuitable";
      case errc::hook_failed:         return "hook-failed";
      case errc::dvar_registration:   return "dvar-registration";
    }

    return "none";
  }

  std::ostream&
  operator<< (std::ostream& os, errc c)
  {
    return os << to_string (c);
  }

  error::
  error (errc c, const std::string& what)
    : std::runtime_error (what), code_ (c)
  {
  }

  error::
  error (errc c, const char* what)
    : std::runtime_error (what), code_ (c)
  {
  }
}
