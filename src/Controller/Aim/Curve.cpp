#include "Curve.hpp"

#include "../Types.hpp"

#include <cmath>
#include <algorithm>

namespace Controller
{
  namespace aim
  {
    const char*
    to_string (curve_kind k) noexcept
    {
      switch (k)
      {
        case curve_kind::linear: return "linear";
        case curve_kind::power:  return "power";
      }

      return "linear";
    }

    bool
    validate (const response_curve& c, std::string& why) noexcept
    {
      if (c.kind == curve_kind::power)
      {
        if (!std::isfinite (c.exponent) || c.exponent <= 0.0f)
        {
          why = "power curve exponent must be finite and greater than zero";
          return false;
        }
      }

      return true;
    }

    float
    evaluate (const response_curve& c, float t) noexcept
    {
      t = std::clamp (t, 0.0f, 1.0f);

      switch (c.kind)
      {
        case curve_kind::linear:
          return t;
        case curve_kind::power:
          return std::pow (t, c.exponent);
      }

      return t;
    }
  }
}
