#pragma once

#include "../Types.hpp"

namespace Controller
{
  namespace mapping
  {
    enum class engine_key : int
    {
      button_a      = 0x01,
      button_b      = 0x02,
      button_x      = 0x03,
      button_y      = 0x04,
      button_lshldr = 0x05,
      button_rshldr = 0x06,

      button_start  = 0x0E,
      button_back   = 0x0F,
      button_lstick = 0x10,
      button_rstick = 0x11,
      button_ltrig  = 0x12,
      button_rtrig  = 0x13,

      dpad_up       = 0x14,
      dpad_down     = 0x15,
      dpad_left     = 0x16,
      dpad_right    = 0x17,

      apad_up       = 0x1C,
      apad_down     = 0x1D,
      apad_left     = 0x1E,
      apad_right    = 0x1F,

      rstick_up     = 0xE0,
      rstick_down   = 0xE1,
      rstick_left   = 0xE2,
      rstick_right  = 0xE3,
    };

    inline constexpr size_t engine_key_count {24};

    std::span<const engine_key>
    keys () noexcept;

    size_t
    key_index (engine_key) noexcept;

    static_assert (static_cast<int> (engine_key::rstick_right) < 256,
                   "controller keys must fit the engine's key state array");

    bool
    is_controller_key (int keynum) noexcept;

    const char*
    key_name (engine_key) noexcept;

    std::optional<engine_key>
    key_from_name (std::string_view) noexcept;

    std::optional<engine_key>
    key_from_keynum (int) noexcept;
  }
}
