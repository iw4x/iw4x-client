#pragma once

#include "../Types.hpp"

namespace Controller
{
  namespace engine
  {
    struct move_delta
    {
      int8_t forward {0};
      int8_t right {0};

      friend constexpr bool
      operator== (move_delta, move_delta) noexcept = default;
    };

    bool
    move_changed (move_delta from, move_delta to) noexcept;

    uint16_t
    pack_move (move_delta to, int key) noexcept;

    move_delta
    unpack_move (uint16_t packed, int key) noexcept;
  }
}
