#pragma once

#include "../Types.hpp"

namespace Controller
{
  enum class trigger_side : uint8_t
  {
    left,
    right,
  };

  inline constexpr size_t trigger_count {2};

  const char*
  to_string (trigger_side) noexcept;

  struct trigger_sample
  {
    uint16_t raw {0};
    float normalized {0.0f};
  };
}
