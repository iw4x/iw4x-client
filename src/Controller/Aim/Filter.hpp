#pragma once

#include "../Types.hpp"

#include "../Clock.hpp"

namespace Controller
{
  namespace aim
  {
    class low_pass
    {
    public:
      explicit
      low_pass (seconds time_constant) noexcept;

      float
      apply (float target, seconds dt) noexcept;

      void
      reset (float value) noexcept;

      void
      reset () noexcept;

      float
      value () const noexcept {return state_;}

    private:
      seconds tau_;
      float state_ {0.0f};
      bool primed_ {false};
    };
  }
}
