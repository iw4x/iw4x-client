#include "Physical.hpp"

#include "../Types.hpp"

#include <variant>

namespace Controller
{
  namespace mapping
  {
    std::optional<engine_key>
    to_engine_key (button b) noexcept
    {
      switch (b)
      {
        case button::face_south: return engine_key::button_a;
        case button::face_east:  return engine_key::button_b;
        case button::face_west:  return engine_key::button_x;
        case button::face_north: return engine_key::button_y;
        case button::l1:         return engine_key::button_lshldr;
        case button::r1:         return engine_key::button_rshldr;
        case button::l2:         return engine_key::button_ltrig;
        case button::r2:         return engine_key::button_rtrig;
        case button::l3:         return engine_key::button_lstick;
        case button::r3:         return engine_key::button_rstick;
        case button::start:      return engine_key::button_start;
        case button::back:       return engine_key::button_back;
        case button::dpad_up:    return engine_key::dpad_up;
        case button::dpad_down:  return engine_key::dpad_down;
        case button::dpad_left:  return engine_key::dpad_left;
        case button::dpad_right: return engine_key::dpad_right;

        case button::guide:
        case button::touchpad:
        case button::mute:
        case button::edge_paddle_left:
        case button::edge_paddle_right:
        case button::edge_fn_left:
        case button::edge_fn_right:
        case button::count:
          return std::nullopt;
      }

      return std::nullopt;
    }

    engine_key
    to_engine_key (const apad_input& a) noexcept
    {
      if (a.which == stick::right)
      {
        switch (a.direction)
        {
          case stick_direction::up:    return engine_key::rstick_up;
          case stick_direction::down:  return engine_key::rstick_down;
          case stick_direction::left:  return engine_key::rstick_left;
          case stick_direction::right: return engine_key::rstick_right;
        }

        return engine_key::rstick_up;
      }

      switch (a.direction)
      {
        case stick_direction::up:    return engine_key::apad_up;
        case stick_direction::down:  return engine_key::apad_down;
        case stick_direction::left:  return engine_key::apad_left;
        case stick_direction::right: return engine_key::apad_right;
      }

      return engine_key::apad_up;
    }

    std::optional<engine_key>
    to_engine_key (const physical_input& p) noexcept
    {
      if (const auto* b = std::get_if<button> (&p))
        return to_engine_key (*b);

      return to_engine_key (std::get<apad_input> (p));
    }

    bool
    axis_deflected (float value,
                    bool positive,
                    bool was_down,
                    float pressed,
                    float hysteresis) noexcept
    {
      const float threshold (pressed + (was_down ? -hysteresis : hysteresis));

      return positive ? value > threshold : value < -threshold;
    }
  }
}
