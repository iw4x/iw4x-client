#include "Filter.hpp"

#include "../Types.hpp"

#include <cmath>
#include <algorithm>

namespace Controller
{
  namespace aim
  {
    low_pass::
    low_pass (seconds time_constant) noexcept
      : tau_ (time_constant)
    {
    }

    float
    low_pass::
    apply (float target, seconds dt) noexcept
    {
      if (tau_.count () <= 0.0f)
        return target;

      if (!primed_)
      {
        state_ = target;
        primed_ = true;
        return state_;
      }

      const float alpha (
        std::clamp (1.0f - std::exp (-dt.count () / tau_.count ()), 0.0f, 1.0f));

      state_ += alpha * (target - state_);
      return state_;
    }

    void
    low_pass::
    reset (float value) noexcept
    {
      state_ = value;
      primed_ = true;
    }

    void
    low_pass::
    reset () noexcept
    {
      primed_ = false;
    }
  }
}
