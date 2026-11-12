#include "Ceg.hpp"

namespace Components
{
  Ceg::Ceg()
  {
    // Looking for stuff in the registry (CD key, from Com_Init).
    //
    Utils::Hook::Nop(0x14027712D, 5);

    // Note that x86 also disables CEG's integrity checks, its shutdown, and
    // the 'Steam Start' check for DRM IPC. The x64 executable carries no CEG
    // and calls SteamAPI_Init without that check, so nothing else is left
    // to disable.
    //
  }
}
