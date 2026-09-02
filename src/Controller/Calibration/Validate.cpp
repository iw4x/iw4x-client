#include "Validate.hpp"

#include "../Types.hpp"

#include <cmath>

namespace Controller
{
  namespace calibration
  {
    namespace
    {
      bool
      finite (float f) noexcept {return std::isfinite (f);}

      bool
      finite (const sensor_vec3& v) noexcept
      {
        return finite (v.x) && finite (v.y) && finite (v.z);
      }
    }

    bool
    validate (const profile& p, std::string& why) noexcept
    {
      if (p.version == 0 || p.version > profile::current_version)
      {
        why = "unsupported calibration profile version";
        return false;
      }

      for (const stick_calibration& s: p.sticks)
      {
        if (!finite (s.center_x) || !finite (s.center_y) ||
            !finite (s.range_x) || !finite (s.range_y) ||
            !finite (s.drift_threshold))
        {
          why = "stick calibration has a non-finite value";
          return false;
        }

        if (s.range_x <= 0.0f || s.range_y <= 0.0f)
        {
          why = "stick calibration range must be strictly positive";
          return false;
        }

        if (s.drift_threshold < 0.0f || s.drift_threshold >= 1.0f)
        {
          why = "stick drift threshold must be in [0, 1)";
          return false;
        }
      }

      for (const trigger_calibration& t: p.triggers)
      {
        if (!finite (t.min) || !finite (t.max))
        {
          why = "trigger calibration has a non-finite value";
          return false;
        }

        if (t.max <= t.min)
        {
          why = "trigger calibration max must exceed min";
          return false;
        }
      }

      if (!finite (p.motion.gyro_bias) || !finite (p.motion.accel_bias) ||
          !finite (p.motion.gyro_scale) || !finite (p.motion.accel_scale))
      {
        why = "motion calibration has a non-finite value";
        return false;
      }

      if (!finite (p.smoothing) || p.smoothing < 0.0f)
      {
        why = "smoothing time constant must be finite and non-negative";
        return false;
      }

      return true;
    }
  }
}
