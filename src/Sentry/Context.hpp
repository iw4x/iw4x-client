#pragma once

#include "Types.hpp"

#include "Diagnostic.hpp"
#include "Error.hpp"

namespace Sentry
{
  class context
  {
  public:
    explicit
    context (diagnostic_sink& sink, bool developer) noexcept
      : diagnostics_ (sink), developer_ (developer) {}

    diagnostic_sink&
    diagnostics () const noexcept {return diagnostics_;}

    bool
    developer () const noexcept {return developer_;}

    void
    report (severity, facility, errc, std::string message) const;

  private:
    diagnostic_sink& diagnostics_;
    bool developer_;
  };
}
