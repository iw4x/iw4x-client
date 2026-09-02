#include "Integrator.hpp"

#include "../Types.hpp"

#include <algorithm>

namespace Controller
{
  namespace aim
  {
    degrees
    turn_integrator::
    advance (deg_per_s target, const limits& lim, seconds dt) noexcept
    {
      const float dtc (dt.count ());

      if (dtc <= 0.0f)
        return degrees {0.0f};

      const float prev (current_.value);
      const float tgt (target.value);
      float cur (prev);

      if (cur < tgt)
      {
        const float step (lim.accel.value <= 0.0f ? (tgt - cur)
                                                  : lim.accel.value * dtc);
        cur = std::min (cur + step, tgt);
      }
      else if (cur > tgt)
      {
        const float step (lim.decel.value <= 0.0f ? (cur - tgt)
                                                  : lim.decel.value * dtc);
        cur = std::max (cur - step, tgt);
      }

      current_ = deg_per_s {cur};

      return degrees {0.5f * (prev + cur) * dtc};
    }
  }
}
