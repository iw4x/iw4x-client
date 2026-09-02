#include "Measure.hpp"

#include "../Types.hpp"

#include <cmath>
#include <algorithm>

namespace Controller
{
  namespace calibration
  {
    void
    stick_measurer::
    observe_rest (stick_vector v) noexcept
    {
      rest_sum_x_ += v.x;
      rest_sum_y_ += v.y;
      ++rest_n_;
    }

    void
    stick_measurer::
    observe_sweep (stick_vector v) noexcept
    {
      max_x_ = std::max (max_x_, std::fabs (v.x));
      max_y_ = std::max (max_y_, std::fabs (v.y));
      ++sweep_n_;
    }

    stick_calibration
    stick_measurer::
    finalize () const noexcept
    {
      constexpr float min_range {0.1f};

      stick_calibration c;
      c.center_x = rest_n_ != 0
        ? static_cast<float> (rest_sum_x_ / static_cast<double> (rest_n_))
        : 0.0f;
      c.center_y = rest_n_ != 0
        ? static_cast<float> (rest_sum_y_ / static_cast<double> (rest_n_))
        : 0.0f;

      c.range_x = sweep_n_ != 0 ? std::max (max_x_, min_range) : 1.0f;
      c.range_y = sweep_n_ != 0 ? std::max (max_y_, min_range) : 1.0f;
      c.drift_threshold = 0.0f;
      return c;
    }

    void
    stick_measurer::
    reset () noexcept
    {
      *this = stick_measurer {};
    }

    void
    motion_bias_measurer::
    observe (const motion_sample& m) noexcept
    {
      gx_ += m.gyro.angular_velocity.x;
      gy_ += m.gyro.angular_velocity.y;
      gz_ += m.gyro.angular_velocity.z;
      ax_ += m.accel.acceleration.x;
      ay_ += m.accel.acceleration.y;
      az_ += m.accel.acceleration.z;
      ++n_;
    }

    void
    motion_bias_measurer::
    finalize (motion_calibration& mc) const noexcept
    {
      if (n_ == 0)
        return;

      const double inv (1.0 / static_cast<double> (n_));
      mc.gyro_bias = {static_cast<float> (gx_ * inv),
                      static_cast<float> (gy_ * inv),
                      static_cast<float> (gz_ * inv)};
      mc.accel_bias = {static_cast<float> (ax_ * inv),
                       static_cast<float> (ay_ * inv),
                       static_cast<float> (az_ * inv)};
    }

    void
    motion_bias_measurer::
    reset () noexcept
    {
      *this = motion_bias_measurer {};
    }
  }
}
