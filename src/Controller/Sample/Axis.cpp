#include "Axis.hpp"

#include "../Types.hpp"

#include <cmath>

namespace Controller
{
  const char*
  to_string (stick s) noexcept
  {
    switch (s)
    {
      case stick::left:  return "left";
      case stick::right: return "right";
    }

    return "left";
  }

  float
  stick_vector::
  magnitude () const noexcept
  {
    return std::sqrt (x * x + y * y);
  }
}
