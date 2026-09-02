#pragma once

#include "../Types.hpp"

#include "../Clock.hpp"
#include "Types.hpp"

namespace Controller
{
  namespace aim
  {
    class turn_integrator
    {
    public:
      struct limits
      {
        deg_per_s2 accel {0.0f};
        deg_per_s2 decel {0.0f};
      };

      degrees
      advance (deg_per_s target, const limits&, seconds dt) noexcept;

      deg_per_s
      current () const noexcept {return current_;}

      void
      reset () noexcept {current_ = deg_per_s {0.0f};}

    private:
      deg_per_s current_ {0.0f};
    };
  }
}
