#include "Ceg.hpp"

namespace Components
{
  Ceg::Ceg()
  {
    // Looking for stuff in the registry
    //
    // On x64 that's the CD key, which the file system startup reads
    // (presumably for the same reason as on x86).
    //
    // Note that this is all that is left of the x86 component. There we also
    // disable CEG's integrity checks, its shutdown, and the 'Steam Start'
    // check for DRM IPC. As far as we can tell, the x64 executable carries no
    // CEG at all and calls SteamAPI_Init() without that check, so there is
    // nothing else to disable.
    //
    Utils::Hook::Nop(0x14027712D, 5);
  }
}
