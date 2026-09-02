#include "Glyph.hpp"

#include "../Types.hpp"

namespace Controller
{
  namespace mapping
  {
    namespace
    {
      struct glyph_entry
      {
        engine_key key;
        const char* xbox;
        const char* playstation;
      };

      constexpr glyph_entry glyphs[]
      {
        {engine_key::button_a,      "^\x01\x32\x32\x08" "button_a",      "^\x01\x32\x32\x10" "button_ps3_cross"},
        {engine_key::button_b,      "^\x01\x32\x32\x08" "button_b",      "^\x01\x32\x32\x11" "button_ps3_circle"},
        {engine_key::button_x,      "^\x01\x32\x32\x08" "button_x",      "^\x01\x32\x32\x11" "button_ps3_square"},
        {engine_key::button_y,      "^\x01\x32\x32\x08" "button_y",      "^\x01\x32\x32\x13" "button_ps3_triangle"},
        {engine_key::button_lshldr, "^\x01\x32\x32\x0D" "button_lshldr", "^\x01\x32\x32\x0D" "button_ps3_l1"},
        {engine_key::button_rshldr, "^\x01\x32\x32\x0D" "button_rshldr", "^\x01\x32\x32\x0D" "button_ps3_r1"},
        {engine_key::button_start,  "^\x01\x32\x32\x0C" "button_start",  "^\x01\x32\x32\x10" "button_ps3_start"},
        {engine_key::button_back,   "^\x01\x32\x32\x0B" "button_back",   "^\x01\x32\x32\x0F" "button_ps3_back"},
        {engine_key::button_lstick, "^\x01\x48\x32\x0D" "button_lstick", "^\x01\x48\x32\x0D" "button_ps3_l3"},
        {engine_key::button_rstick, "^\x01\x48\x32\x0D" "button_rstick", "^\x01\x48\x32\x0D" "button_ps3_r3"},
        {engine_key::button_ltrig,  "^\x01\x32\x32\x0C" "button_ltrig",  "^\x01\x32\x32\x0D" "button_ps3_l2"},
        {engine_key::button_rtrig,  "^\x01\x32\x32\x0C" "button_rtrig",  "^\x01\x32\x32\x0D" "button_ps3_r2"},
        {engine_key::dpad_up,       "^\x01\x32\x32\x07" "dpad_up",       "^\x01\x32\x32\x0B" "dpad_ps3_up"},
        {engine_key::dpad_down,     "^\x01\x32\x32\x09" "dpad_down",     "^\x01\x32\x32\x0D" "dpad_ps3_down"},
        {engine_key::dpad_left,     "^\x01\x32\x32\x09" "dpad_left",     "^\x01\x32\x32\x0D" "dpad_ps3_left"},
        {engine_key::dpad_right,    "^\x01\x32\x32\x0A" "dpad_right",    "^\x01\x32\x32\x0E" "dpad_ps3_right"},
      };
    }

    glyph_family
    glyph_family_for (Controller::family device,
                      std::optional<glyph_family> user_override) noexcept
    {
      if (user_override)
        return *user_override;

      switch (device)
      {
        case Controller::family::dualshock4:
        case Controller::family::dualsense:
        case Controller::family::dualsense_edge:
          return glyph_family::playstation;

        case Controller::family::xbox:
        case Controller::family::unknown:
          return glyph_family::xbox;
      }

      return glyph_family::xbox;
    }

    const char*
    glyph_for (engine_key k, glyph_family f) noexcept
    {
      for (const glyph_entry& e: glyphs)
      {
        if (e.key == k)
          return f == glyph_family::playstation ? e.playstation : e.xbox;
      }

      return nullptr;
    }
  }
}
