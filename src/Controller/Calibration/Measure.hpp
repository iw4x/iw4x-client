#pragma once

#include "../Types.hpp"

#include "Profile.hpp"
#include "../Sample/Axis.hpp"
#include "../Sample/Motion.hpp"

namespace Controller
{
  namespace calibration
  {
    class stick_measurer
    {
    public:
      void
      observe_rest (stick_vector) noexcept;

      void
      observe_sweep (stick_vector) noexcept;

      stick_calibration
      finalize () const noexcept;

      void
      reset () noexcept;

      size_t
      rest_samples () const noexcept {return rest_n_;}

      size_t
      sweep_samples () const noexcept {return sweep_n_;}

    private:
      double rest_sum_x_ {0.0};
      double rest_sum_y_ {0.0};
      size_t rest_n_ {0};

      float max_x_ {0.0f};
      float max_y_ {0.0f};
      size_t sweep_n_ {0};
    };

    class motion_bias_measurer
    {
    public:
      void
      observe (const motion_sample&) noexcept;

      void
      finalize (motion_calibration&) const noexcept;

      void
      reset () noexcept;

      size_t
      samples () const noexcept {return n_;}

    private:
      double gx_ {0.0}, gy_ {0.0}, gz_ {0.0};
      double ax_ {0.0}, ay_ {0.0}, az_ {0.0};
      size_t n_ {0};
    };
  }
}
