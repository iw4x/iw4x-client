#include "StickLayout.hpp"

#include "../Types.hpp"

#include <cctype>
#include <algorithm>

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

        std::transform (r.begin (), r.end (), r.begin (), [] (char c)
        {
          return static_cast<char> (
            std::tolower (static_cast<unsigned char> (c)));
        });

        return r;
      }

      float
      squared (float component, stick_vector v) noexcept
      {
        return std::clamp (v.magnitude () * component, -1.0f, 1.0f);
      }
    }

    const char*
    to_string (virtual_axis a) noexcept
    {
      switch (a)
      {
        case virtual_axis::side:    return "side";
        case virtual_axis::forward: return "forward";
        case virtual_axis::yaw:     return "yaw";
        case virtual_axis::pitch:   return "pitch";
      }

      return "unknown";
    }

    const char*
    to_string (stick_layout l) noexcept
    {
      switch (l)
      {
        case stick_layout::standard:        return "thumbstick_default";
        case stick_layout::southpaw:        return "thumbstick_southpaw";
        case stick_layout::legacy:          return "thumbstick_legacy";
        case stick_layout::legacy_southpaw: return "thumbstick_legacysouthpaw";
      }

      return "thumbstick_default";
    }

    stick_layout
    stick_layout_from_name (std::string_view name) noexcept
    {
      const std::string n (lowercase (name));

      if (n == "thumbstick_legacysouthpaw")
        return stick_layout::legacy_southpaw;

      if (n == "thumbstick_legacy")
        return stick_layout::legacy;

      if (n == "thumbstick_southpaw")
        return stick_layout::southpaw;

      return stick_layout::standard;
    }

    virtual_axis
    axis_for (stick_layout l, stick which, bool horizontal) noexcept
    {
      const bool southpaw (l == stick_layout::southpaw ||
                           l == stick_layout::legacy_southpaw);
      const bool legacy (l == stick_layout::legacy ||
                         l == stick_layout::legacy_southpaw);

      const bool move_stick ((which == stick::left) != southpaw);

      if (!horizontal)
        return move_stick ? virtual_axis::forward : virtual_axis::pitch;

      if (move_stick)
        return legacy ? virtual_axis::yaw : virtual_axis::side;

      return legacy ? virtual_axis::side : virtual_axis::yaw;
    }

    resolved_axes
    resolve (stick_layout l, stick_vector left, stick_vector right) noexcept
    {
      resolved_axes r;

      const stick sticks[] {stick::left, stick::right};

      for (stick s: sticks)
      {
        const stick_vector& v (s == stick::left ? left : right);

        for (bool horizontal: {true, false})
        {
          const float component (horizontal ? v.x : v.y);

          switch (axis_for (l, s, horizontal))
          {
            case virtual_axis::side:    r.side = squared (component, v);    break;
            case virtual_axis::forward: r.forward = squared (component, v); break;
            case virtual_axis::yaw:     r.yaw = component;                  break;
            case virtual_axis::pitch:   r.pitch = component;                break;
          }
        }
      }

      return r;
    }
  }
}
