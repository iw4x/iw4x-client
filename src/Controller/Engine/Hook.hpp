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

    // Release the images we loaded for the glyphs and forget them, so that
    // they are loaded again, from the new device, once there is one. Must be
    // called before the renderer destroys the device, with the render thread
    // idle.
    //
    // Note that the engine only releases and restores its own images, so a
    // texture of ours kept across the restart would be one of the destroyed
    // device, which the new one fails to draw with.
    //
    void
    release_glyph_images () noexcept;

    // Return the material hint text draws the named button glyph with
    // instead of base, the HUD one it found for the name, or nullptr to draw
    // base. Must be called from the render thread, and the result is only
    // good until the next call.
    //
    Game::Material*
    text_glyph_material (const char* name, const Game::Material* base) noexcept;
  }
}
