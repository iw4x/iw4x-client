#pragma once

#include "../Types.hpp"

#include "../Device/Identity.hpp"
#include "../Sample/Axis.hpp"
#include "../Sample/Trigger.hpp"
#include "../Sample/Motion.hpp"

namespace Controller
{
  namespace calibration
  {
    enum class value_source : uint8_t
    {
      built_in,
      measured,
      user,
    };

    const char*
    to_string (value_source) noexcept;

    struct stick_calibration
    {
      float center_x {0.0f};
      float center_y {0.0f};
      float range_x {1.0f};
      float range_y {1.0f};

      float drift_threshold {0.0f};
    };

    struct trigger_calibration
    {
      float min {0.0f};
      float max {1.0f};
    };

    struct motion_calibration
    {
      sensor_vec3 gyro_bias {};
      sensor_vec3 accel_bias {};
      float gyro_scale {1.0f};
      float accel_scale {1.0f};
    };

    struct profile
    {
      static constexpr uint16_t current_version {1};

      uint16_t version {current_version};
      Controller::family family {Controller::family::unknown};
      std::optional<uint64_t> device_key;
      value_source source {value_source::built_in};

      std::array<stick_calibration, stick_count> sticks {};
      std::array<trigger_calibration, trigger_count> triggers {};
      motion_calibration motion {};

      float smoothing {0.0f};
    };

    profile
    default_profile (Controller::family) noexcept;
  }
}
