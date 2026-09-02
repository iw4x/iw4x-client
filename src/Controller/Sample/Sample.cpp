#include "Sample.hpp"

#include "../Types.hpp"

namespace Controller
{
  const char*
  to_string (battery_state::status s) noexcept
  {
    switch (s)
    {
      case battery_state::status::unknown:     return "unknown";
      case battery_state::status::discharging: return "discharging";
      case battery_state::status::charging:    return "charging";
      case battery_state::status::full:        return "full";
    }

    return "unknown";
  }
}
