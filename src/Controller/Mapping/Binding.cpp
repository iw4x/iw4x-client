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

    namespace
    {
      struct layout
      {
        const char* name;
        const char* keyword;

        action trigger_right;
        action trigger_left;
        action shoulder_right;
        action shoulder_left;
        action stick_right;
        action stick_left;
        action face_b;
      };

      constexpr layout layouts[]
      {
        {"buttons_default", "default",
         action::fire, action::ads,
         action::frag, action::special_grenade,
         action::melee, action::sprint,
         action::stance},

        {"buttons_tactical", "tactical",
         action::fire, action::ads,
         action::frag, action::special_grenade,
         action::stance, action::sprint,
         action::melee},

        {"buttons_lefty", "lefty",
         action::ads, action::fire,
         action::special_grenade, action::frag,
         action::sprint, action::melee,
         action::stance},

        {"buttons_nomad", "nomad",
         action::fire, action::ads_toggle,
         action::frag, action::special_grenade,
         action::stance, action::sprint,
         action::melee},
      };

      const layout&
      layout_for (const std::string& name) noexcept
      {
        for (const layout& l: layouts)
        {
          if (name == l.name)
            return l;
        }

        for (const layout& l: layouts)
        {
          if (contains (name, l.keyword))
            return l;
        }

        return layouts[0];
      }

      constexpr std::string_view alt_suffix {"_alt"};
    }

    void
    apply_button_layout (binding_table& t, std::string_view name)
    {
      std::string n (lowercase (name));

      const bool alt (n.size () > alt_suffix.size () &&
                      n.compare (n.size () - alt_suffix.size (),
                                 alt_suffix.size (),
                                 alt_suffix) == 0);

      if (alt)
        n.resize (n.size () - alt_suffix.size ());

      const layout& l (layout_for (n));

      t.clear ();

      t.bind (engine_key::button_start, action::menu);
      t.bind (engine_key::button_back,  action::scoreboard);

      t.bind (engine_key::button_a, action::jump_stand);
      t.bind (engine_key::button_b, l.face_b);
      t.bind (engine_key::button_x, action::use_reload);
      t.bind (engine_key::button_y, action::next_weapon);

      t.bind (engine_key::button_rstick, l.stick_right);
      t.bind (engine_key::button_lstick, l.stick_left);

      t.bind (alt ? engine_key::button_rshldr : engine_key::button_rtrig,
              l.trigger_right);
      t.bind (alt ? engine_key::button_lshldr : engine_key::button_ltrig,
              l.trigger_left);
      t.bind (alt ? engine_key::button_rtrig : engine_key::button_rshldr,
              l.shoulder_right);
      t.bind (alt ? engine_key::button_ltrig : engine_key::button_lshldr,
              l.shoulder_left);

      t.bind (engine_key::dpad_up,    action::action_slot_1);
      t.bind (engine_key::dpad_down,  action::action_slot_2);
      t.bind (engine_key::dpad_left,  action::action_slot_3);
      t.bind (engine_key::dpad_right, action::action_slot_4);
    }
  }
}
