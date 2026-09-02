#include "NetMove.hpp"

#include "../Types.hpp"

namespace Controller
{
  namespace engine
  {
    bool
    move_changed (move_delta from, move_delta to) noexcept
    {
      return to.forward != from.forward || to.right != from.right;
    }

    uint16_t
    pack_move (move_delta to, int key) noexcept
    {
      const int plain (static_cast<uint8_t> (to.forward) |
                       (static_cast<uint8_t> (to.right) << 8));

      return static_cast<uint16_t> ((key ^ plain) & 0xFFFF);
    }

    move_delta
    unpack_move (uint16_t packed, int key) noexcept
    {
      const int bits ((key ^ packed) & 0xFFFF);

      return {static_cast<int8_t> (bits & 0xFF),
              static_cast<int8_t> ((bits >> 8) & 0xFF)};
    }
  }
}
