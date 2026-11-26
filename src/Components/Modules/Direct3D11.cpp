#include "Direct3D11.hpp"

#include "Direct3D11/Direct3D.hpp"

using namespace std;

namespace Components
{
  namespace
  {
    IDirect3D9* CALLBACK
    create_direct3d (UINT sdk)
    {
      // Direct3DCreate9() returns null for an SDK version it doesn't know,
      // which for IW4 means a build we don't expect.
      //
      if (sdk != D3D_SDK_VERSION)
        return nullptr;

      d3d11::direct3d* r (new d3d11::direct3d ());

      if (FAILED (r->create ()))
      {
        r->Release ();
        return nullptr;
      }

      return r;
    }

    // Fail with a proper error if we couldn't create the interface.
    //
    // The renderer ignores this failure and carries on to create the
    // window and the device with the interface it doesn't have, crashing
    // on the null pointer.
    //
    // Note that x64 inlines R_InitDirect3D() into its caller (presumably
    // R_InitGraphicsApi(), the one that goes on to create the window). So
    // there is no R_InitDirect3D() call to hook as on x86 and we hook the
    // Direct3DCreate9() call itself, which is why we take the SDK version
    // and return the interface.
    //
    IDirect3D9*
    r_init_direct3d (UINT sdk)
    {
      if (IDirect3D9* r = create_direct3d (sdk))
        return r;

      Game::Sys_Error (Utils::IsWineEnvironment ()
                       ? "Couldn't initialize Direct3D 11. Make sure your graphics drivers support Vulkan, which Wine DXVK needs to run the game."
                       : "Couldn't initialize Direct3D 11. Make sure your graphics card supports Direct3D 11 and its drivers are up to date.");
      return nullptr;
    }
  }

  direct3d11::
  direct3d11 ()
  {
    if (Dedicated::IsEnabled ())
      return;

    // Replace the imported Direct3DCreate9(), which other callers go
    // through, and the renderer's own call.
    //
    Utils::Hook::Set (0x1403619C8, create_direct3d);
    Utils::Hook (0x140032646, r_init_direct3d, HOOK_CALL).install ()->quick (); // R_InitGraphicsApi
  }
}
