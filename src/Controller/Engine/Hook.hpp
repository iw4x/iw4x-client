#pragma once

#include "../Types.hpp"

#include "../Mapping/Glyph.hpp"

namespace Controller
{
  class runtime;
}

namespace Controller
{
  namespace engine
  {
    void
    install (runtime&);

    void
    install_protocol ();

    void
    note_mouse_move (int dx, int dy) noexcept;

    // The glyph family button prompts are drawn in: the one gpad_style
    // forces, or else the one of the active controller.
    //
    mapping::glyph_family
    active_glyph_family () noexcept;

    // The glyph family of the active controller, whatever gpad_style says.
    //
    mapping::glyph_family
    controller_glyph_family () noexcept;

    // Flick the thumbstick glyphs between their up and pressed images, as
    // the console prompts do, and load the small glyphs hint text uses.
    //
    void
    update_glyph_images () noexcept;

    // The material hint text draws the named button glyph with instead of
    // the HUD one, or nullptr to draw that.
    //
    Game::Material*
    text_glyph_material (const char* name) noexcept;
  }
}
