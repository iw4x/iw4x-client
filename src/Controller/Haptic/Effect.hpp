#pragma once

#include "../Types.hpp"

#include "../Clock.hpp"
#include "../Support/InplaceVector.hpp"

namespace Controller
{
  namespace haptic
  {
    struct frame
    {
      float left {0.0f};
      float right {0.0f};
    };

    enum class actuator : uint8_t
    {
      left,
      right,
      both,
    };

    const char*
    to_string (actuator) noexcept;

    class envelope
    {
    public:
      static constexpr size_t max_knots {16};

      struct knot
      {
        float at {0.0f};
        float amplitude {0.0f};
      };

      constexpr envelope () = default;

      static envelope
      level (float amplitude) noexcept;

      static envelope
      from (std::span<const knot>) noexcept;

      float
      evaluate (float t) const noexcept;

      bool
      empty () const noexcept {return knots_.empty ();}

      std::span<const knot>
      knots () const noexcept {return {knots_.data (), knots_.size ()};}

    private:
      inplace_vector<knot, max_knots> knots_;
    };

    inline constexpr float min_hertz {40.0f};
    inline constexpr float max_hertz {320.0f};

    float
    hertz_for (float sharpness) noexcept;

    struct effect
    {
      envelope deep;
      envelope crisp;

      float deep_sharpness {0.0f};
      float crisp_sharpness {1.0f};

      float intensity {1.0f};

      seconds duration {0.25f};

      bool loop {false};

      actuator where {actuator::both};

      uint32_t tag {0};
    };

    effect
    transient (float intensity, float sharpness) noexcept;

    effect
    continuous (float intensity, float sharpness, seconds duration) noexcept;
  }
}
