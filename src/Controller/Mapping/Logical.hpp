#pragma once

#include "../Types.hpp"

namespace Controller
{
  namespace mapping
  {
    enum class action : uint8_t
    {
      none,

      fire,
      ads,
      jump_stand,
      stance,
      melee,
      use_reload,
      sprint,
      next_weapon,
      frag,
      special_grenade,
      menu,

      scoreboard,
      action_slot_1,
      action_slot_2,
      action_slot_3,
      action_slot_4,
    };

    const char*
    to_string (action) noexcept;

    const char*
    command (action) noexcept;
  }
}
