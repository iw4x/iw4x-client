#include "Context.hpp"

#include "Types.hpp"

namespace Controller
{
  void
  context::
  report (severity level,
          facility origin,
          errc code,
          device_id device,
          std::string message) const
  {
    Controller::report (diagnostics_, level, origin, code, device,
                        std::move (message));
  }

  void
  context::
  report (severity level,
          facility origin,
          errc code,
          std::string message) const
  {
    Controller::report (diagnostics_, level, origin, code, std::move (message));
  }
}
