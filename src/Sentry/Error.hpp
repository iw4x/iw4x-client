#pragma once

#include "Types.hpp"

namespace Sentry
{
  enum class errc : uint8_t
  {
    none,

    not_configured,
    already_started,

    daemon_missing,
    database_unusable,

    initialization_failed,
    shutdown_failed,

    scope_rejected,
    attachment_missing,

    dvar_registration,
    filter_unchained,
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
