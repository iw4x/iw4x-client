#pragma once

#include "../Types.hpp"

#include "../Sample/Axis.hpp"

namespace Controller
{
  namespace mapping
  {
    enum class virtual_axis : uint8_t
    {
      side,
      forward,
      yaw,
      pitch,
    };

    const char*
    to_string (virtual_axis) noexcept;

    enum class stick_layout : uint8_t
    {
      standard,
      southpaw,
      legacy,
      legacy_southpaw,
    };

    const char*
    to_string (stick_layout) noexcept;

    stick_layout
    stick_layout_from_name (std::string_view) noexcept;

    virtual_axis
    axis_for (stick_layout, stick which, bool horizontal) noexcept;

    struct resolved_axes
    {
      float side {0.0f};
      float forward {0.0f};
      float yaw {0.0f};
      float pitch {0.0f};
    };

    resolved_axes
    resolve (stick_layout, stick_vector left, stick_vector right) noexcept;
  }
}
