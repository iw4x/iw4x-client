#include "Context.hpp"

#include "Types.hpp"

namespace Sentry
{
  void
  context::
  report (severity level,
          facility origin,
          errc code,
          std::string message) const
  {
    Sentry::report (diagnostics_, level, origin, code, std::move (message));
  }
}
