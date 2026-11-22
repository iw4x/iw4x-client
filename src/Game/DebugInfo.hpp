#pragma once

namespace Game
{
  // The debug info dvar that x64 compiles out, which Components::Dvar
  // registers for us.
  //
  extern const dvar_t* cg_debugInfoCornerOffset;

  // Draw the text aligned to the point by the low two bits of align
  // (horizontally) and the next two (vertically), and return the font
  // height.
  //
  // Note that x64 compiles out the dev string draws along with what calls
  // them, so we reimplement this from x86.
  //
  int CG_DrawDevString(const ScreenPlacement* scrPlace, float x, float y, float xScale, float yScale, const char* text, const float* color, int align, Font_s* font);

  // Trace along the direction and describe the material we hit along with
  // its surface type and flags, and its contents. Return false if we hit
  // nothing or the descriptions don't fit charLimit.
  //
  // Note that x64 compiles out R_PickMaterial() along with
  // CG_DrawMaterial(), so we reimplement both from x86. Both are debugging
  // aids, which is presumably why they didn't make the cut.
  //
  bool R_PickMaterial(bool locational, int traceMask, const float* org, const float* dir, char* name, char* surfaceFlags, char* contents, unsigned int charLimit);

  // Draw what R_PickMaterial() describes at the center of the view, with
  // the trace mask that drawMaterialType (cg_drawMaterial) selects.
  //
  void CG_DrawMaterial(int localClientNum, int drawMaterialType);

  // Draw the cg_drawFPS, cg_drawSnapshot, and cg_drawViewpos lines down
  // from the upper right corner of the screen.
  //
  void CG_DrawUpperRightDebugInfo(int localClientNum);

  // Paint the UI menus and record the frame time for cg_drawFPS. Then, once
  // the client is active, draw the upper right debug info on top.
  //
  // Note that this replaces the UI menu paint call in SCR_UpdateFrame(),
  // since x64 drops the debug overlay call that x86 makes right after it.
  //
  void CL_DrawDebugOverlays(int localClientNum);
}
