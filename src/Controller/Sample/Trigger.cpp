#include "Trigger.hpp"

#include "../Types.hpp"

namespace Controller
{
  const char*
  to_string (trigger_side s) noexcept
  {
    switch (s)
    {
      case trigger_side::left:  return "left";
      case trigger_side::right: return "right";
    }

    return "left";
  }
}
