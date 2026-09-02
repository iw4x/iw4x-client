#pragma once

#include "../Types.hpp"

namespace Controller
{
  enum class stick : uint8_t
  {
    left,
    right,
  };

  inline constexpr size_t stick_count {2};

  const char*
  to_string (stick) noexcept;

  struct stick_vector
  {
    float x {0.0f};
    float y {0.0f};

    float
    magnitude () const noexcept;
  };

  struct stick_raw
  {
    int32_t x {0};
    int32_t y {0};
  };

  struct stick_sample
  {
    stick_raw raw {};
    stick_vector normalized {};
    stick_vector calibrated {};
    stick_vector filtered {};
  };
}
