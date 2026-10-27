#include "Platform.hpp"

#include "../Types.hpp"

#include <cstring>
#include <mutex>
#include <string>

namespace Controller
{
  namespace mapping
  {
    namespace
    {
      // The controller versions of the PLATFORM_* strings IW4x shows, as in
      // the console game: a control byte stands for the button whose
      // engine_key has that value (0x01 A, 0x02 B, 0x03 X, 0x04 Y, ...), the
      // same codes the console font uses for its button glyphs.
      //
      // PLATFORM_RELOAD is left alone: the weapon HUD draws the reload
      // button next to it already.
      //
      // The hints that name a binding keep &&1, which the engine fills in
      // with the bound button, and take the console wording around it.
      //
      struct platform_entry
      {
        const char* key;
        const char* text;
      };

      constexpr platform_entry platform_strings[]
      {
        {"PLATFORM_BACK_CAPS",              "BACK    \x02"},
        {"PLATFORM_GAME_SUMMARY_CAPS",      "GAME SUMMARY    \x04"},
        {"PLATFORM_UI_SELECTBUTTON",        "\x01"},
        {"PLATFORM_REFRESH",                "\x03 Refresh"},
        {"PLATFORM_VIEW_CHALLENGES",        "\x03 View Challenges"},
        {"PLATFORM_VIEW_CHALLENGE_DETAILS", "\x03 Challenge Details"},
        {"PLATFORM_UI_CLEAR_KILLSTREAKS",   "\x04 Clear Kill Streaks"},
        {"PLATFORM_FOLLOWNEXTPLAYER",       "\x02 Next"},
        {"PLATFORM_FOLLOWPREVIOUSPLAYER",   "\x01 Previous"},
        {"PLATFORM_CALL_NUKE",              "Press and hold ^3&&1^7 for Nuclear Strike."},
        {"PLATFORM_DETONATE",               "Press ^3&&1^7 to detonate"},
        {"PLATFORM_GET_AUTO_SHOT",          "Press and hold ^3&&1^7 to pick up an Auto Shotgun."},
        {"PLATFORM_GET_KILLSTREAK",         "Press and hold ^3&&1^7 for Killstreak"},
        {"PLATFORM_GET_KIT",                "Press and hold ^3&&1^7 for new kit."},
        {"PLATFORM_GET_RANDOM",             "Press and hold ^3&&1^7 to pick up a random item."},
        {"PLATFORM_GET_SENTRY",             "Press and hold ^3&&1^7 to pick up a turret."},
        {"PLATFORM_GET_THUMPER",            "Press and hold ^3&&1^7 to pick up a Thumper."},
        {"PLATFORM_HOLD_BREATH",            "Press and hold &&1 to steady"},
        {"PLATFORM_HOLD_TO_DEFUSE_EXPLOSIVES", "Press and hold &&1 to defuse explosives  "},
        {"PLATFORM_HOLD_TO_DROP",           "Press &&1 to drop"},
        {"PLATFORM_HOLD_TO_PLANT_EXPLOSIVES", "Press and hold &&1 to plant explosives  "},
        {"PLATFORM_HOLD_TO_USE",            "Hold &&1 to use"},
        {"PLATFORM_MANTLE",                 "Press &&1 to  "},
        {"PLATFORM_PICKUPAMMO",             "Press &&1 to pickup the &&2 ammo"},
        {"PLATFORM_PICKUPNEWWEAPON",        "Hold &&1 to pick up"},
        {"PLATFORM_PRESS_TO_USE_AIRSTRIKE", "Hold &&1 to call in air strike"},
        {"PLATFORM_PRESS_TO_USE_ARTILLERY", "Hold &&1 to call in artillery"},
        {"PLATFORM_PRESS_TO_USE_COPTER",    "Hold &&1 to call in helicopter"},
        {"PLATFORM_PRESS_TO_USE_RADAR",     "Hold &&1 to call in UAV Recon"},
        {"PLATFORM_RESUPPLY",               "Press and hold ^3&&1^7 to resupply"},
        {"PLATFORM_REVIVE",                 "Press and hold ^3&&1^7 to revive player."},
        {"PLATFORM_STANCEHINT_CROUCH",      "Press &&1 to crouch"},
        {"PLATFORM_STANCEHINT_JUMP",        "Press &&1 to jump"},
        {"PLATFORM_STANCEHINT_PRONE",       "Press &&1 to go prone"},
        {"PLATFORM_STANCEHINT_STAND",       "Press &&1 to stand"},
        {"PLATFORM_SWAPWEAPONS",            "Press and hold &&1 for"},
        {"PLATFORM_THROWBACKGRENADE",       "&&1 throw back"},
        {"PLATFORM_USEAION30CAL",           "Hold &&1 to commandeer the .30 cal"},
      };

      constexpr size_t platform_count {std::size (platform_strings)};

      std::string
      expand (const char* text, glyph_family f)
      {
        std::string r;

        for (const char* p (text); *p != '\0'; ++p)
        {
          const unsigned char c (static_cast<unsigned char> (*p));

          if (c < 0x20)
          {
            if (const char* g = glyph_for (static_cast<engine_key> (c), f))
              r += g;
          }
          else
            r += *p;
        }

        return r;
      }

      // Both families are expanded once, the first time a string is asked
      // for, since the glyph table is fixed and lookups come from several
      // threads.
      //
      std::string expanded[2][platform_count];
      std::once_flag expanded_once;
    }

    const char*
    platform_string (const char* key, glyph_family f) noexcept
    {
      if (key == nullptr)
        return nullptr;

      for (size_t i (0); i != platform_count; ++i)
      {
        if (std::strcmp (platform_strings[i].key, key) != 0)
          continue;

        std::call_once (expanded_once, []
        {
          for (size_t j (0); j != platform_count; ++j)
          {
            expanded[0][j] = expand (platform_strings[j].text, glyph_family::xbox);
            expanded[1][j] = expand (platform_strings[j].text, glyph_family::playstation);
          }
        });

        return expanded[f == glyph_family::playstation ? 1 : 0][i].c_str ();
      }

      return nullptr;
    }
  }
}
