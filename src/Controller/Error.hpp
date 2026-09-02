#pragma once

#include "Types.hpp"

namespace Controller
{
  enum class errc : uint8_t
  {
    none,

    device_unavailable,
    ambiguous_identity,
    unsupported_device,

    transport_failure,

    report_malformed,
    report_truncated,
    checksum_mismatch,

    output_rejected,

    calibration_invalid,
    calibration_version,

    graph_invalid,

    binding_invalid,

    steam_unavailable,
    steam_unsuitable,

    hook_failed,
    dvar_registration,
  };

  const char*
  to_string (errc) noexcept;

  std::ostream&
  operator<< (std::ostream&, errc);

  class error: public std::runtime_error
  {
  public:
    error (errc, const std::string& what);
    error (errc, const char* what);

    errc
    code () const noexcept {return code_;}

  private:
    errc code_;
  };
}
