#include "Assist.hpp"

#include "../Types.hpp"

#include <cmath>
#include <algorithm>

namespace Controller
{
  namespace aim
  {
    namespace
    {
      deg_per_s
      lerp_rate (deg_per_s a, deg_per_s b, float t) noexcept
      {
        return {std::lerp (a.value, b.value, t)};
      }

      deadzone_params
      lerp_deadzone (const deadzone_params& a,
                     const deadzone_params& b,
                     float t) noexcept
      {
        return {magnitude {std::lerp (a.inner.value, b.inner.value, t)},
                magnitude {std::lerp (a.outer.value, b.outer.value, t)},
                magnitude {std::lerp (a.anti.value, b.anti.value, t)}};
      }
    }

    aim_processor::
    aim_processor (config c)
      : cfg_ (std::move (c))
    {
    }

    void
    aim_processor::
    reset () noexcept
    {
      yaw_.reset ();
      pitch_.reset ();
    }

    aim_frame_output
    aim_processor::
    process (const aim_frame_input& in) noexcept
    {
      const float t (std::clamp (in.ads_lerp, 0.0f, 1.0f));

      const deadzone_params dz (
        lerp_deadzone (cfg_.hip.deadzone, cfg_.ads.deadzone, t));
      const stick_vector v (apply (dz, in.look));

      const float deflection (
        std::clamp (std::sqrt (v.x * v.x + v.y * v.y), 0.0f, 1.0f));

      const float response (cfg_.graph != nullptr
                            ? cfg_.graph->evaluate (deflection)
                            : 1.0f);

      const float eff_yaw (v.x * response);
      const float eff_pitch (v.y * response);

      const float gain_yaw (in.fov_scale * in.sensitivity * in.slowdown_yaw);
      const float gain_pitch (in.fov_scale * in.sensitivity * in.slowdown_pitch);

      deg_per_s yaw_rate (
        lerp_rate (cfg_.hip.yaw_rate, cfg_.ads.yaw_rate, t) * gain_yaw);
      deg_per_s pitch_rate (
        lerp_rate (cfg_.hip.pitch_rate, cfg_.ads.pitch_rate, t) * gain_pitch);

      if (in.yaw_max && in.yaw_max->value < yaw_rate.value)
        yaw_rate = *in.yaw_max;
      if (in.pitch_max && in.pitch_max->value < pitch_rate.value)
        pitch_rate = *in.pitch_max;

      const float yaw_sign (eff_yaw >= 0.0f ? 1.0f : -1.0f);
      const float pitch_sign (eff_pitch >= 0.0f ? 1.0f : -1.0f);

      const deg_per_s yaw_target {std::fabs (eff_yaw) * yaw_rate.value};
      const deg_per_s pitch_target {std::fabs (eff_pitch) * pitch_rate.value};

      degrees yaw_delta (yaw_.advance (yaw_target, cfg_.accel, in.dt) * yaw_sign);
      degrees pitch_delta (
        pitch_.advance (pitch_target, cfg_.accel, in.dt) * pitch_sign);

      if (in.invert_pitch)
        pitch_delta = -pitch_delta;

      return {yaw_delta, pitch_delta};
    }

    float
    slowdown_scale (bool target_present,
                    float hip_scale,
                    float ads_scale,
                    float ads_lerp) noexcept
    {
      if (!target_present)
        return 1.0f;

      return std::lerp (hip_scale, ads_scale, std::clamp (ads_lerp, 0.0f, 1.0f));
    }

    stick_vector
    scale_dominant_axis (stick_vector look) noexcept
    {
      const float ax (std::fabs (look.x));
      const float ay (std::fabs (look.y));

      if (ay <= ax)
        look.y *= 1.0f - (ax - ay);
      else
        look.x *= 1.0f - (ay - ax);

      return look;
    }

    aim_frame_output
    lock_on (const lock_on_target& target,
             const lock_on_params& params,
             seconds dt) noexcept
    {
      if (target.distance <= 0.0f)
        return {};

      const float arc (target.distance * pi);

      const float pitch_rate (
        (dot (target.target_velocity, target.view_pitch_axis) -
         dot (target.player_velocity, target.view_pitch_axis)) /
        arc * 180.0f * params.pitch_strength);

      const float yaw_rate (
        (dot (target.target_velocity, target.view_yaw_axis) -
         dot (target.player_velocity, target.view_yaw_axis)) /
        arc * 180.0f * params.yaw_strength);

      return {degrees {yaw_rate * dt.count ()},
              degrees {pitch_rate * dt.count ()}};
    }
  }
}
