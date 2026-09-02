#pragma once

#include "../Types.hpp"

namespace Controller
{
  enum class capability : uint32_t
  {
    none               = 0,

    gyroscope          = 1u << 0,
    accelerometer      = 1u << 1,
    touchpad           = 1u << 2,
    battery            = 1u << 3,
    microphone_button  = 1u << 4,
    back_buttons       = 1u << 5,

    rumble             = 1u << 8,
    haptics            = 1u << 9,
    adaptive_triggers  = 1u << 10,
    light_bar          = 1u << 11,
    player_leds        = 1u << 12,
  };

  class capabilities
  {
  public:
    using rep = std::underlying_type_t<capability>;

    constexpr capabilities () = default;

    constexpr capabilities (capability c) noexcept
      : bits_ (static_cast<rep> (c)) {}

    constexpr bool
    has (capability c) const noexcept
    {
      return (bits_ & static_cast<rep> (c)) == static_cast<rep> (c) &&
             c != capability::none;
    }

    constexpr capabilities&
    add (capability c) noexcept
    {
      bits_ |= static_cast<rep> (c);
      return *this;
    }

    constexpr capabilities&
    remove (capability c) noexcept
    {
      bits_ &= ~static_cast<rep> (c);
      return *this;
    }

    constexpr bool
    empty () const noexcept {return bits_ == 0;}

    constexpr rep
    value () const noexcept {return bits_;}

    friend constexpr bool
    operator== (capabilities, capabilities) noexcept = default;

    friend constexpr capabilities
    operator| (capabilities l, capabilities r) noexcept
    {
      capabilities c;
      c.bits_ = l.bits_ | r.bits_;
      return c;
    }

  private:
    rep bits_ {0};
  };

  constexpr capabilities
  operator| (capability l, capability r) noexcept
  {
    return capabilities (l) | capabilities (r);
  }

  std::ostream&
  operator<< (std::ostream&, capabilities);
}
