#pragma once

#include "../Types.hpp"

#include "Axis.hpp"
#include "Button.hpp"
#include "Trigger.hpp"
#include "Touch.hpp"
#include "Motion.hpp"

#include "../Device/Capability.hpp"

namespace Controller
{
  struct battery_state
  {
    enum class status : uint8_t
    {
      unknown,
      discharging,
      charging,
      full,
    };

    status state {status::unknown};
    std::optional<uint8_t> percent;
  };

  const char*
  to_string (battery_state::status) noexcept;

  struct raw_sample
  {
    std::array<stick_raw, stick_count> sticks {};
    std::array<uint16_t, trigger_count> triggers {};
    uint32_t buttons {0};
    std::optional<motion_sample> motion;
    std::optional<touchpad> touch;
    std::optional<battery_state> battery;
  };

  struct canonical_sample
  {
    button_set buttons {};
    std::array<stick_sample, stick_count> sticks {};
    std::array<trigger_sample, trigger_count> triggers {};
    std::optional<touchpad> touch;
    std::optional<motion_sample> motion;
    std::optional<battery_state> battery;
    capabilities caps {};
  };
}
