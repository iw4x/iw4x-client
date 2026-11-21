#pragma once

namespace Game
{
  // Debug info dvar that x64 compiles out, registered by Components::Dvar.
  //
  extern const dvar_t* cg_debugInfoCornerOffset;

  // Draw the text aligned to the point in the low (horizontal) and next
  // (vertical) two bits of align, and return the font height.
  //
  // Note that x64 compiles out the dev string draws along with what calls
  // them, so we reimplement it from x86.
  //
  int CG_DrawDevString(const ScreenPlacement* scrPlace, float x, float y, float xScale, float yScale, const char* text, const float* color, int align, Font_s* font);

  // Draw the cg_drawFPS, cg_drawSnapshot and cg_drawViewpos lines down from
  // the upper right corner of the screen.
  //
  void CG_DrawUpperRightDebugInfo(int localClientNum);

  // Paint the UI menus, record the frame time for cg_drawFPS and, once the
  // client is active, draw the upper right debug info on top.
  //
  // Note that this replaces the UI menu paint call in SCR_UpdateFrame: x64
  // drops the debug overlay call that x86 makes right after it.
  //
  void CL_DrawDebugOverlays(int localClientNum);
}
