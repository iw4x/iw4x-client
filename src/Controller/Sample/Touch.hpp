#pragma once

#include "../Types.hpp"

namespace Controller
{
  struct touch_point
  {
    bool active {false};
    uint8_t id {0};
    uint16_t x {0};
    uint16_t y {0};
  };

  struct touchpad
  {
    static constexpr size_t max_points {2};

    std::array<touch_point, max_points> points {};
  };
}
