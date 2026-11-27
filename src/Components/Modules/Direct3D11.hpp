#pragma once

namespace Components
{
  // Direct3D 9 on top of Direct3D 11.
  //
  // IW4 renders with Direct3D 9, which by now is a legacy API: on Windows
  // it goes through an aging driver path and under Wine it needs a
  // translation layer of its own. So instead we hand IW4 an IDirect3D9 of
  // our own (see Direct3D11/) that implements the part of Direct3D 9 that
  // IW4 uses on top of Direct3D 11.
  //
  // The main difficulty is the shaders. Direct3D 11 can't run IW4's Shader
  // Model 3 bytecode, so every IW4 shader is translated to Shader Model 5
  // ahead of time (with dexilifier and fxc) into a shader pack embedded into
  // iw4x.dll. When IW4 creates a shader, we look up its translation in the
  // pack by the hash of its bytecode. A shader that isn't in the pack (one
  // from a custom zone that was never added to it, say) is logged and its
  // draws are skipped.
  //
  // Set the IW4X_D3D11_DEBUG environment variable to enable the Direct3D 11
  // debug layer, if installed.
  //
  class direct3d11: public Component
  {
  public:
    direct3d11 ();
  };
}
