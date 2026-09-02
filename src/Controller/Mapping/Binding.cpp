#include "Binding.hpp"

#include "../Types.hpp"

#include <cctype>
#include <string>

namespace Controller
{
  namespace mapping
  {
    namespace
    {
      std::string
      lowercase (std::string_view s)
      {
        std::string r (s);
        for (char& c: r)
          c = static_cast<char> (std::tolower (static_cast<unsigned char> (c)));
        return r;
      }

      bool
      contains (const std::string& s, std::string_view sub) noexcept
      {
        return s.find (sub) != std::string::npos;
      }
    }

    void
    binding_table::
    bind (engine_key k, std::string command)
    {
      commands_[key_index (k)] = std::move (command);
    }

    void
    binding_table::
    bind (engine_key k, action a)
    {
      commands_[key_index (k)] = command (a);
    }

    void
    binding_table::
    unbind (engine_key k) noexcept
    {
      commands_[key_index (k)].clear ();
    }

    void
    binding_table::
    clear () noexcept
    {
      for (std::string& c: commands_)
        c.clear ();
    }

    const std::string*
    binding_table::
    command_for (engine_key k) const noexcept
    {
      const std::string& c (commands_[key_index (k)]);
      return c.empty () ? nullptr : &c;
    }

    void
    binding_table::
    for_each (function_ref<void (engine_key, const std::string&)> fn) const
    {
      const std::span<const engine_key> all (keys ());

      for (size_t i (0); i < count; ++i)
      {
        if (!commands_[i].empty ())
          fn (all[i], commands_[i]);
      }
    }

    size_t
    binding_table::
    size () const noexcept
    {
      size_t n (0);
      for (const std::string& c: commands_)
      {
        if (!c.empty ())
          ++n;
      }
      return n;
    }

    void
    apply_button_layout (binding_table& t, std::string_view name)
    {
      const std::string n (lowercase (name));

      const bool tactical (contains (n, "tactical"));
      const bool lefty (contains (n, "lefty"));
      const bool nomad (contains (n, "nomad"));
      const bool alt (contains (n, "_alt"));

      t.clear ();

      t.bind (engine_key::button_start, action::menu);
      t.bind (engine_key::button_back,  action::scoreboard);
      t.bind (engine_key::button_x,     action::use_reload);
      t.bind (engine_key::button_y,     action::next_weapon);
      t.bind (engine_key::dpad_up,      action::action_slot_1);
      t.bind (engine_key::dpad_down,    action::action_slot_2);
      t.bind (engine_key::dpad_left,    action::action_slot_3);
      t.bind (engine_key::dpad_right,   action::action_slot_4);
      t.bind (engine_key::button_a,     action::jump_stand);

      t.bind (engine_key::button_b,      tactical ? action::melee : action::stance);
      t.bind (engine_key::button_rstick, tactical ? action::stance : action::melee);

      if (lefty)
      {
        t.bind (engine_key::button_ltrig,  action::fire);
        t.bind (engine_key::button_rtrig,  action::ads);
        t.bind (engine_key::button_lshldr, nomad ? action::sprint : action::frag);
        t.bind (engine_key::button_rshldr, action::special_grenade);
        t.bind (engine_key::button_rstick, action::sprint);
      }
      else
      {
        t.bind (engine_key::button_rtrig,  action::fire);
        t.bind (engine_key::button_ltrig,  action::ads);
        t.bind (engine_key::button_rshldr,
                alt ? action::special_grenade : action::frag);
        t.bind (engine_key::button_lshldr,
                alt ? action::frag : action::special_grenade);
        t.bind (engine_key::button_lstick,
                nomad ? action::jump_stand : action::sprint);
      }
    }
  }
}
