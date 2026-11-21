#include "Sound.hpp"

namespace Components
{
  Sound::
  Sound ()
  {
    // Note that on x86 we work around a few bugs in the sound and voice
    // initialization, none of which x64 has:
    //
    // The x86 game initializes the sound driver (Miles over DirectSound)
    // before it creates its window, which leaves DirectSound bound to the
    // wrong window, or to none, so we initialize it after the renderer.
    // x64 plays sound through XAudio2, which isn't bound to a window, and
    // binds the DirectSound of voice chat to the desktop window.
    //
    // We also patch a double release of the DirectSound interface in Miles,
    // which x64 doesn't use.
    //
    // Finally we fix the creation of the voice playback buffers, which x86
    // starts without checking that DirectSound is initialized, with a
    // clobbered argument, and releases uninitialized on failure, and whose
    // failure poisons the buffer array with null pointers. x64 checks that
    // DirectSound is initialized, creates the buffer inline, releases it only
    // if it was created, and skips the null buffers wherever it uses them.
  }
}
