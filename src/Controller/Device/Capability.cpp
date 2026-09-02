#include "Capability.hpp"

#include "../Types.hpp"

namespace Controller
{
  std::ostream&
  operator<< (std::ostream& os, capabilities caps)
  {
    struct entry
    {
      capability bit;
      const char* name;
    };

    static constexpr entry entries[]
    {
      {capability::gyroscope,         "gyroscope"},
      {capability::accelerometer,     "accelerometer"},
      {capability::touchpad,          "touchpad"},
      {capability::battery,           "battery"},
      {capability::microphone_button, "microphone-button"},
      {capability::back_buttons,      "back-buttons"},
      {capability::rumble,            "rumble"},
      {capability::haptics,           "haptics"},
      {capability::adaptive_triggers, "adaptive-triggers"},
      {capability::light_bar,         "light-bar"},
      {capability::player_leds,       "player-leds"},
    };

    os << '{';

    bool first (true);
    for (const entry& e: entries)
    {
      if (!caps.has (e.bit))
        continue;

      if (!first)
        os << ", ";

      os << e.name;
      first = false;
    }

    return os << '}';
  }
}
