#pragma once

namespace Game
{
  // Debug info dvar that x64 compiles out, registered by Components::Dvar.
  //
  extern const dvar_t* cg_debugInfoCornerOffset;

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
