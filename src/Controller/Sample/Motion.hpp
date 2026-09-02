#pragma once

#include "../Types.hpp"

namespace Controller
{
  struct sensor_vec3
  {
    float x {0.0f};
    float y {0.0f};
    float z {0.0f};
  };

  struct gyro_sample
  {
    sensor_vec3 angular_velocity {};
  };

  struct accel_sample
  {
    sensor_vec3 acceleration {};
  };

  struct motion_sample
  {
    gyro_sample gyro {};
    accel_sample accel {};
    std::optional<uint32_t> device_timestamp;
  };
}
