#include "Key.hpp"

#include "../Types.hpp"

#include <cctype>

namespace Controller
{
  namespace mapping
  {
    namespace
    {
      struct named_key
      {
        engine_key key;
        const char* name;
      };

      constexpr named_key key_names[]
      {
        {engine_key::button_a,      "BUTTON_A"},
        {engine_key::button_b,      "BUTTON_B"},
        {engine_key::button_x,      "BUTTON_X"},
        {engine_key::button_y,      "BUTTON_Y"},
        {engine_key::button_lshldr, "BUTTON_LSHLDR"},
        {engine_key::button_rshldr, "BUTTON_RSHLDR"},
        {engine_key::button_start,  "BUTTON_START"},
        {engine_key::button_back,   "BUTTON_BACK"},
        {engine_key::button_lstick, "BUTTON_LSTICK"},
        {engine_key::button_rstick, "BUTTON_RSTICK"},
        {engine_key::button_ltrig,  "BUTTON_LTRIG"},
        {engine_key::button_rtrig,  "BUTTON_RTRIG"},
        {engine_key::dpad_up,       "DPAD_UP"},
        {engine_key::dpad_down,     "DPAD_DOWN"},
        {engine_key::dpad_left,     "DPAD_LEFT"},
        {engine_key::dpad_right,    "DPAD_RIGHT"},
        {engine_key::apad_up,       "APAD_UP"},
        {engine_key::apad_down,     "APAD_DOWN"},
        {engine_key::apad_left,     "APAD_LEFT"},
        {engine_key::apad_right,    "APAD_RIGHT"},
        {engine_key::rstick_up,     "RSTICK_UP"},
        {engine_key::rstick_down,   "RSTICK_DOWN"},
        {engine_key::rstick_left,   "RSTICK_LEFT"},
        {engine_key::rstick_right,  "RSTICK_RIGHT"},
      };

      static_assert (sizeof (key_names) / sizeof (key_names[0]) == engine_key_count,
                     "the key name table must cover every controller key");

      constexpr std::array<engine_key, engine_key_count>
      make_key_list () noexcept
      {
        std::array<engine_key, engine_key_count> r {};

        for (size_t i (0); i != engine_key_count; ++i)
          r[i] = key_names[i].key;

        return r;
      }

      constexpr std::array<engine_key, engine_key_count> key_list {make_key_list ()};

      bool
      iequals (std::string_view a, std::string_view b) noexcept
      {
        if (a.size () != b.size ())
          return false;

        for (size_t i (0); i < a.size (); ++i)
        {
          if (std::tolower (static_cast<unsigned char> (a[i])) !=
              std::tolower (static_cast<unsigned char> (b[i])))
            return false;
        }

        return true;
      }
    }

    std::span<const engine_key>
    keys () noexcept
    {
      return {key_list.data (), key_list.size ()};
    }

    size_t
    key_index (engine_key k) noexcept
    {
      for (size_t i (0); i != engine_key_count; ++i)
      {
        if (key_list[i] == k)
          return i;
      }

      assert (false);
      return 0;
    }

    bool
    is_controller_key (int keynum) noexcept
    {
      return (keynum >= 0x01 && keynum <= 0x06) ||
             (keynum >= 0x0E && keynum <= 0x19) ||
             (keynum >= 0x1C && keynum <= 0x1F) ||
             (keynum >= static_cast<int> (engine_key::rstick_up) &&
              keynum <= static_cast<int> (engine_key::rstick_right));
    }

    const char*
    key_name (engine_key k) noexcept
    {
      return key_names[key_index (k)].name;
    }

    std::optional<engine_key>
    key_from_name (std::string_view name) noexcept
    {
      for (const named_key& e: key_names)
      {
        if (iequals (name, e.name))
          return e.key;
      }

      return std::nullopt;
    }

    std::optional<engine_key>
    key_from_keynum (int keynum) noexcept
    {
      for (const engine_key k: key_list)
      {
        if (static_cast<int> (k) == keynum)
          return k;
      }

      if (!is_controller_key (keynum))
        return std::nullopt;

      return static_cast<engine_key> (keynum);
    }
  }
}
