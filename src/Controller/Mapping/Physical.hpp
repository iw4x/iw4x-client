#pragma once

#include "../Types.hpp"

#include "../Sample/Axis.hpp"
#include "../Sample/Button.hpp"
#include "Key.hpp"

#include <variant>

namespace Controller
{
  namespace mapping
  {
    enum class stick_direction : uint8_t
    {
      up,
      down,
      left,
      right,
    };

    struct apad_input
    {
      stick which {stick::left};
      stick_direction direction {stick_direction::up};
    };

    using physical_input = std::variant<button, apad_input>;

    std::optional<engine_key>
    to_engine_key (button) noexcept;

    engine_key
    to_engine_key (const apad_input&) noexcept;

    std::optional<engine_key>
    to_engine_key (const physical_input&) noexcept;

    bool
    axis_deflected (float value,
                    bool positive,
                    bool was_down,
                    float pressed,
                    float hysteresis) noexcept;
  }
}
