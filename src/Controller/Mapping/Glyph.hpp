#pragma once

#include "../Types.hpp"

#include "Key.hpp"
#include "../Device/Identity.hpp"

namespace Controller
{
  namespace mapping
  {
    enum class glyph_family : uint8_t
    {
      xbox,
      playstation,
    };

    glyph_family
    glyph_family_for (Controller::family device,
                      std::optional<glyph_family> user_override) noexcept;

    const char*
    glyph_for (engine_key, glyph_family) noexcept;
  }
}
