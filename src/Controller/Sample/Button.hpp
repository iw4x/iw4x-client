#pragma once

#include "../Types.hpp"

namespace Controller
{
  enum class button : uint8_t
  {
    face_south,
    face_east,
    face_west,
    face_north,

    dpad_up,
    dpad_down,
    dpad_left,
    dpad_right,

    l1,
    r1,
    l2,
    r2,
    l3,
    r3,

    start,
    back,
    guide,

    touchpad,
    mute,

    edge_paddle_left,
    edge_paddle_right,
    edge_fn_left,
    edge_fn_right,

    count,
  };

  static_assert (static_cast<size_t> (button::count) <= 32,
                 "button_set stores buttons in a 32-bit mask");

  const char*
  to_string (button) noexcept;

  class button_set
  {
  public:
    constexpr button_set () = default;

    constexpr bool
    down (button b) const noexcept
    {
      return (bits_ & mask (b)) != 0;
    }

    constexpr void
    set (button b, bool on) noexcept
    {
      if (on)
        bits_ |= mask (b);
      else
        bits_ &= ~mask (b);
    }

    constexpr bool
    any () const noexcept {return bits_ != 0;}

    constexpr uint32_t
    value () const noexcept {return bits_;}

    constexpr button_set
    pressed_since (button_set prev) const noexcept
    {
      return from_bits (bits_ & ~prev.bits_);
    }

    constexpr button_set
    released_since (button_set prev) const noexcept
    {
      return from_bits (prev.bits_ & ~bits_);
    }

    friend constexpr bool
    operator== (button_set, button_set) noexcept = default;

  private:
    static constexpr uint32_t
    mask (button b) noexcept
    {
      return uint32_t {1} << static_cast<uint32_t> (b);
    }

    static constexpr button_set
    from_bits (uint32_t b) noexcept
    {
      button_set s;
      s.bits_ = b;
      return s;
    }

    uint32_t bits_ {0};
  };
}
