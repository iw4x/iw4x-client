#pragma once

#include "../Clock.hpp"

namespace Controller
{
  namespace aim
  {
    inline constexpr float pi {3.14159265358979323846f};

    struct degrees
    {
      float value {0.0f};

      friend constexpr degrees
      operator+ (degrees a, degrees b) noexcept {return {a.value + b.value};}

      friend constexpr degrees
      operator- (degrees a, degrees b) noexcept {return {a.value - b.value};}

      friend constexpr degrees
      operator- (degrees a) noexcept {return {-a.value};}

      friend constexpr degrees
      operator* (degrees a, float s) noexcept {return {a.value * s};}

      friend constexpr bool
      operator== (degrees, degrees) noexcept = default;

      friend constexpr std::partial_ordering
      operator<=> (degrees, degrees) noexcept = default;
    };

    struct radians
    {
      float value {0.0f};
    };

    inline constexpr radians
    to_radians (degrees d) noexcept {return {d.value * (pi / 180.0f)};}

    inline constexpr degrees
    to_degrees (radians r) noexcept {return {r.value * (180.0f / pi)};}

    struct deg_per_s
    {
      float value {0.0f};

      friend constexpr deg_per_s
      operator* (deg_per_s r, float s) noexcept {return {r.value * s};}

      friend constexpr deg_per_s
      operator+ (deg_per_s a, deg_per_s b) noexcept {return {a.value + b.value};}

      friend constexpr bool
      operator== (deg_per_s, deg_per_s) noexcept = default;

      friend constexpr std::partial_ordering
      operator<=> (deg_per_s, deg_per_s) noexcept = default;
    };

    inline constexpr degrees
    operator* (deg_per_s r, seconds dt) noexcept
    {
      return {r.value * dt.count ()};
    }

    struct deg_per_s2
    {
      float value {0.0f};
    };

    inline constexpr deg_per_s
    operator* (deg_per_s2 a, seconds dt) noexcept
    {
      return {a.value * dt.count ()};
    }

    struct axis_input
    {
      float value {0.0f};

      constexpr float
      sign () const noexcept {return value >= 0.0f ? 1.0f : -1.0f;}

      constexpr float
      absolute () const noexcept {return value >= 0.0f ? value : -value;}
    };

    struct magnitude
    {
      float value {0.0f};
    };

    struct screen_vector
    {
      float x {0.0f};
      float y {0.0f};
    };

    struct world_vector
    {
      float x {0.0f};
      float y {0.0f};
      float z {0.0f};
    };

    inline constexpr float
    dot (world_vector a, world_vector b) noexcept
    {
      return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    std::ostream& operator<< (std::ostream&, degrees);
    std::ostream& operator<< (std::ostream&, deg_per_s);
  }
}
