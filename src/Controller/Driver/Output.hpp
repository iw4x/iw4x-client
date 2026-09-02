#pragma once

#include "../Types.hpp"

#include "../Haptic/Effect.hpp"
#include "../Sample/Trigger.hpp"

namespace Controller
{
  namespace driver
  {
    enum class haptic_mode : uint8_t
    {
      waveform,
      emulated,
    };

    struct rumble_request
    {
      float low_frequency {0.0f};
      float high_frequency {0.0f};
    };

    struct output_policy
    {
      bool rumble {true};
      haptic_mode haptics {haptic_mode::waveform};
    };

    struct light_bar_request
    {
      uint8_t red {0};
      uint8_t green {0};
      uint8_t blue {0};
    };

    struct player_led_request
    {
      uint8_t mask {0};
    };

    enum class trigger_effect : uint8_t
    {
      off,
      feedback,
      weapon,
    };

    struct adaptive_trigger_request
    {
      trigger_side side {trigger_side::left};
      trigger_effect effect {trigger_effect::off};

      uint8_t start_position {0};
      uint8_t end_position {0};
      uint8_t strength {0};
    };

    using output_request = std::variant<rumble_request,
                                       haptic::effect,
                                       light_bar_request,
                                       player_led_request,
                                       adaptive_trigger_request>;
  }
}
