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
      // with the bound button, but lose the PC colour codes around it, as on
      // console: a glyph drawn in ^3 comes out tinted and washed out.
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
        {"PLATFORM_HOLD_BREATH",            "Press and hold &&1 to steady"},
        {"PLATFORM_MANTLE",                 "Press &&1 to  "},
        {"PLATFORM_THROWBACKGRENADE",       "&&1 throw back"},
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
