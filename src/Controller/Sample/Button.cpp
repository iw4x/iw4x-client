#include "Button.hpp"

#include "../Types.hpp"

namespace Controller
{
  const char*
  to_string (button b) noexcept
  {
    switch (b)
    {
      case button::face_south: return "face-south";
      case button::face_east:  return "face-east";
      case button::face_west:  return "face-west";
      case button::face_north: return "face-north";
      case button::dpad_up:    return "dpad-up";
      case button::dpad_down:  return "dpad-down";
      case button::dpad_left:  return "dpad-left";
      case button::dpad_right: return "dpad-right";
      case button::l1:         return "l1";
      case button::r1:         return "r1";
      case button::l2:         return "l2";
      case button::r2:         return "r2";
      case button::l3:         return "l3";
      case button::r3:         return "r3";
      case button::start:      return "start";
      case button::back:       return "back";
      case button::guide:      return "guide";
      case button::touchpad:          return "touchpad";
      case button::mute:              return "mute";
      case button::edge_paddle_left:  return "edge-paddle-left";
      case button::edge_paddle_right: return "edge-paddle-right";
      case button::edge_fn_left:      return "edge-fn-left";
      case button::edge_fn_right:     return "edge-fn-right";
      case button::count:             return "count";
    }

    return "count";
  }
}
