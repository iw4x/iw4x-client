#include "Logical.hpp"

#include "../Types.hpp"

namespace Controller
{
  namespace mapping
  {
    const char*
    to_string (action a) noexcept
    {
      switch (a)
      {
        case action::none:            return "none";
        case action::fire:            return "fire";
        case action::ads:             return "ads";
        case action::jump_stand:      return "jump-stand";
        case action::stance:          return "stance";
        case action::melee:           return "melee";
        case action::use_reload:      return "use-reload";
        case action::sprint:          return "sprint";
        case action::next_weapon:     return "next-weapon";
        case action::frag:            return "frag";
        case action::special_grenade: return "special-grenade";
        case action::menu:            return "menu";
        case action::scoreboard:      return "scoreboard";
        case action::action_slot_1:   return "action-slot-1";
        case action::action_slot_2:   return "action-slot-2";
        case action::action_slot_3:   return "action-slot-3";
        case action::action_slot_4:   return "action-slot-4";
      }

      return "none";
    }

    const char*
    command (action a) noexcept
    {
      switch (a)
      {
        case action::none:            return "";
        case action::fire:            return "+attack";
        case action::ads:             return "+speed_throw";
        case action::jump_stand:      return "+gostand";
        case action::stance:          return "+stance";
        case action::melee:           return "+melee";
        case action::use_reload:      return "+usereload";
        case action::sprint:          return "+breath_sprint";
        case action::next_weapon:     return "weapnext";
        case action::frag:            return "+frag";
        case action::special_grenade: return "+smoke";
        case action::menu:            return "togglemenu";
        case action::scoreboard:      return "+scores";
        case action::action_slot_1:   return "+actionslot 1";
        case action::action_slot_2:   return "+actionslot 2";
        case action::action_slot_3:   return "+actionslot 3";
        case action::action_slot_4:   return "+actionslot 4";
      }

      return "";
    }
  }
}
