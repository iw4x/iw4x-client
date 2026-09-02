#include "Deadzone.hpp"

#include "../Types.hpp"

#include <cmath>
#include <cassert>
#include <algorithm>

namespace Controller
{
  namespace aim
  {
    bool
    validate (const deadzone_params& p, std::string& why) noexcept
    {
      auto in01 = [] (float f) noexcept {return f >= 0.0f && f < 1.0f;};

      if (!in01 (p.inner.value))
      {
        why = "inner deadzone must be in [0, 1)";
        return false;
      }

      if (!in01 (p.outer.value))
      {
        why = "outer deadzone must be in [0, 1)";
        return false;
      }

      if (!in01 (p.anti.value))
      {
        why = "anti-deadzone must be in [0, 1)";
        return false;
      }

      if (p.inner.value >= 1.0f - p.outer.value)
      {
        why = "inner deadzone must be below (1 - outer deadzone)";
        return false;
      }

      return true;
    }

    stick_vector
    apply (const deadzone_params& p, stick_vector v) noexcept
    {
      const float inner (p.inner.value);
      const float outer (p.outer.value);
      const float anti (p.anti.value);

      assert (inner >= 0.0f && outer >= 0.0f && anti >= 0.0f &&
              inner < 1.0f - outer);

      const float m (std::sqrt (v.x * v.x + v.y * v.y));

      if (m <= inner)
        return {0.0f, 0.0f};

      const float denom ((1.0f - outer) - inner);
      float t ((m - inner) / denom);
      t = std::clamp (t, 0.0f, 1.0f);

      if (anti > 0.0f)
        t = anti + t * (1.0f - anti);

      const float scale (t / m);
      return {v.x * scale, v.y * scale};
    }
  }
}
