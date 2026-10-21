#pragma once

#include "../Types.hpp"

#include "Glyph.hpp"

namespace Controller
{
  namespace mapping
  {
    // Return the controller version of a PLATFORM_* localized string, with
    // its buttons drawn in the given glyph family, or nullptr if the key has
    // no controller version.
    //
    // The PC strings name keyboard keys ("BACK - ESC") or no key at all
    // ("Reload"), where the console strings show the controller button. The
    // returned string stays valid for the lifetime of the process.
    //
    const char*
    platform_string (const char* key, glyph_family) noexcept;
  }
}
