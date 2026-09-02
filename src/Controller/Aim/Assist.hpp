#pragma once

#include "../Types.hpp"

#include "../Clock.hpp"
#include "Types.hpp"
#include "Deadzone.hpp"
#include "Curve.hpp"
#include "Graph.hpp"
#include "Integrator.hpp"
#include "../Sample/Axis.hpp"

namespace Controller
{
  namespace aim
  {
    struct aim_profile
    {
      deg_per_s yaw_rate {0.0f};
      deg_per_s pitch_rate {0.0f};
      deadzone_params deadzone;
      response_curve curve;
    };

    struct aim_frame_input
    {
      stick_vector look;
      float ads_lerp {0.0f};
      float fov_scale {1.0f};
      float sensitivity {1.0f};
      float slowdown_yaw {1.0f};
      float slowdown_pitch {1.0f};
      bool invert_pitch {false};
      std::optional<deg_per_s> yaw_max;
      std::optional<deg_per_s> pitch_max;
      seconds dt {0.0f};
    };

    struct aim_frame_output
    {
      degrees yaw_delta {0.0f};
      degrees pitch_delta {0.0f};
    };

    class aim_processor
    {
    public:
      struct config
      {
        aim_profile hip;
        aim_profile ads;
        turn_integrator::limits accel;
        const aim_graph* graph {nullptr};
      };

      explicit
      aim_processor (config);

      aim_frame_output
      process (const aim_frame_input&) noexcept;

      void
      reset () noexcept;

    private:
      config cfg_;
      turn_integrator yaw_;
      turn_integrator pitch_;
    };

    float
    slowdown_scale (bool target_present,
                    float hip_scale,
                    float ads_scale,
                    float ads_lerp) noexcept;

    stick_vector
    scale_dominant_axis (stick_vector look) noexcept;

    struct lock_on_target
    {
      world_vector target_velocity;
      world_vector player_velocity;
      world_vector view_pitch_axis;
      world_vector view_yaw_axis;
      float distance {0.0f};
    };

    struct lock_on_params
    {
      float yaw_strength {0.0f};
      float pitch_strength {0.0f};
    };

    aim_frame_output
    lock_on (const lock_on_target&, const lock_on_params&, seconds dt) noexcept;
  }
}
