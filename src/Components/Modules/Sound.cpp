#include "Sound.hpp"

namespace Components
{
  constexpr auto g_en  (0x1AA4908); // global enable flag
  constexpr auto ppDS8 (0x1AA490C); // global DirectSound8 interface pointer
  constexpr auto g_cf  (0x79B174);  // config value
  constexpr auto f_new (0x454E40);  // allocator
  constexpr auto f_ini (0x6015D0);  // ds init
  constexpr auto f_log (0x402500);  // logger

  constexpr auto v_loop (0x4A23EB); // voice loop.

  constexpr auto max_rate (0x66CF248); // loaded sound sample rate limit

  const char err[] ("Error: Failed to create DirectSound play buffer\n");

  // Patch a few bugs in the engine's DirectSound buffer initialization.
  //
  // First, we prevent a crash inside the init function by making sure
  // the global DirectSound8 interface is actually initialized before
  // we attempt to use it.
  //
  // Second, we fix a register overwrite bug. The original code clobbers
  // the first argument in eax with the global config value right before
  // the usercall.
  //
  // Finally, we fix a cleanup crash in the failure path. If the buffer
  // creation fails, the engine unconditionally calls Release() on an
  // uninitialized pointer. This results in an access violation when
  // trying to read the vtable.
  //
  __declspec (naked) int Sound::Init ()
  {
    __asm
    {
      cmp  byte ptr ds:[g_en], 0
      jnz  proceed
      xor  eax, eax
      ret

    proceed:
      push esi
      mov  eax, f_new
      call eax
      mov  esi, eax

      test esi, esi
      jnz  ok
      pop  esi
      ret

    ok:
      push edi

      // Check if DirectSound8 device was actually initialized.
      //
      // Note that since ppDS8 is a pointer-to-pointer, we need to dereference
      // it twice. First to check the outer pointer, and then to verify the
      // actual interface pointer isn't null.
      //
      mov  eax, dword ptr ds:[ppDS8]
      test eax, eax
      jz   fail_e

      mov  eax, dword ptr ds:[eax]
      test eax, eax
      jz   fail_e

      // Push stack arguments first, then set up eax/ecx for the usercall.
      //
      mov  edx, dword ptr ds:[g_cf]
      push edx                         // Arg 4
      mov  dword ptr ds:[esi+8], edx
      lea  edi, [esi+4]
      push edi                         // Arg 3: LPDIRECTSOUNDBUFFER *.
      mov  ecx, dword ptr ds:[esi+38h] // Arg 2
      mov  eax, dword ptr ds:[esi+2Ch] // Arg 1

      mov  edx, f_ini
      call edx
      add  esp, 8

      test eax, eax
      jge  success

      push offset err
      push 9
      mov  eax, f_log
      call eax
      add  esp, 8

      // Handle clean-up crash. The original code blindly dereferences the
      // buffer pointer to find the Release() vfunc. But if the init failed, the
      // pointer is likely invalid.
      //
      mov  eax, dword ptr ds:[edi]
      test eax, eax
      jz   fail_c

      // It's valid, so we release the interface. Release() is usually at
      // offset 8 in IUnknown.
      //
      mov  ecx, dword ptr ds:[eax]
      mov  edx, dword ptr ds:[ecx+8]
      push eax
      call edx

    fail_c:
      // Otherwise, we zero out the slot and bail out.
      //
      mov  dword ptr ds:[edi], 0
    fail_e:
      pop  edi
      xor  eax, eax
      pop  esi
      ret

    success:
      pop  edi
      mov  eax, esi
      pop  esi
      ret
    }
  }

  // The engine's voice initialization loop blindly stores the result in an
  // array without checking if the allocation actually succeeded. If we returned
  // 0 above, it poisons the array with null pointers, which later causes a
  // divide-by-zero exception in the Miles MP3 decoder. *Sigh*
  //
  __declspec (naked) void Sound::Loop ()
  {
    __asm
    {
      test eax, eax
      jz   fail_init

      mov  dword ptr ds:[esi], eax
      add  esi, 4

      push v_loop
      ret

    fail_init:
      // If we hit a null pointer, the audio buffer creation failed so we must
      // abort the loop and return false to the caller.
      //
      xor  al, al
      pop  esi
      add  esp, 8
      ret
    }
  }

  // Initialize the sound driver once the game window exists.
  //
  // The game initializes the sound driver before it creates its window,
  // leaving Miles to bind DirectSound to whatever window it can find by then
  // and rebinding it to the game window once that one is created. This means
  // DirectSound is bound to the splash screen if there is one and fails to
  // initialize if there is no window at all (which we've seen happen). So we
  // initialize it right after the renderer creates the window instead.
  //
  // Note that by then the game window has missed its chance to rebind it, so
  // we do it ourselves.
  //
  void Sound::CL_InitRenderer ()
  {
    Game::CL_InitRenderer ();
    Game::SND_InitDriver ();
    Game::SND_SetHWND (Game::g_wv->hWnd);
  }

  // Release the DirectSound interface in the slot and clear the slot.
  //
  void __stdcall Sound::ReleaseDirectSound (IUnknown** slot)
  {
    if (*slot != nullptr)
    {
      (*slot)->Release ();
      *slot = nullptr;
    }
  }

  // Fix a double release in Miles. When it fails to set up DirectSound
  // (SetCooperativeLevel(), or CreateSoundBuffer() or SetFormat() on the
  // primary buffer), it releases the DirectSound interface but leaves it in
  // the driver. Its cleanup then releases it again, by which time the object
  // is gone, and so we end up calling into whatever took its place. So we
  // make each of these failure paths clear the interface after releasing it,
  // which turns the crash into the failure it should have been (the game then
  // carries on without sound).
  //
  // Note that this is how a missing window used to crash the game (see
  // CL_InitRenderer() above) but DirectSound can fail here for other reasons
  // as well (say, the device is in use exclusively).
  //
  // Note also that we patch the code in mss32.dll directly and only if it is
  // the one the game ships with.
  //
  void Sound::PatchMiles ()
  {
    auto* m (reinterpret_cast<std::uint8_t*> (GetModuleHandleA ("mss32.dll")));
    if (m == nullptr)
      return;

    // Each failure path loads the address of the interface slot (edi or a
    // local) and then calls Release() through the vtable. We replace this with
    // pushing the slot address and calling ReleaseDirectSound().
    //
    struct site
    {
      std::uint32_t             rva;
      std::vector<std::uint8_t> code;
      std::size_t               push; // Size of the slot push.
    };

    // The first two sites (SetCooperativeLevel() and CreateSoundBuffer()
    // failures) have the slot address in edi:
    //
    //   8B 07          mov  eax, [edi]       ; IDirectSound*
    //   8B 08          mov  ecx, [eax]       ; vtable
    //   8B 51 08       mov  edx, [ecx+8]     ; Release()
    //   50             push eax
    //   FF D2          call edx
    //
    // The last one (SetFormat() failure) has it in a local:
    //
    //   8B 44 24 14    mov  eax, [esp+14h]   ; IDirectSound**
    //   8B 00          mov  eax, [eax]       ; IDirectSound*
    //   8B 08          mov  ecx, [eax]       ; vtable
    //   8B 51 08       mov  edx, [ecx+8]     ; Release()
    //   50             push eax
    //   FF D2          call edx
    //
    // Which we turn into, respectively:
    //
    //   57             push edi
    //   E8 xx xx xx xx call ReleaseDirectSound
    //   90 90 90 90    nop
    //
    //   8B 44 24 14    mov  eax, [esp+14h]
    //   50             push eax
    //   E8 xx xx xx xx call ReleaseDirectSound
    //   90 90 90 90    nop
    //
    const site sites[] =
    {
      {0x2E8F7, {0x8B, 0x07, 0x8B, 0x08, 0x8B, 0x51, 0x08, 0x50, 0xFF, 0xD2},                         1},
      {0x2E91C, {0x8B, 0x07, 0x8B, 0x08, 0x8B, 0x51, 0x08, 0x50, 0xFF, 0xD2},                         1},
      {0x2E941, {0x8B, 0x44, 0x24, 0x14, 0x8B, 0x00, 0x8B, 0x08, 0x8B, 0x51, 0x08, 0x50, 0xFF, 0xD2}, 5},
    };

    // Make sure all the sites match before we touch any of them.
    //
    for (const auto& s: sites)
    {
      if (!std::ranges::equal (std::span (m + s.rva, s.code.size ()), s.code))
      {
        Logger::Warning (Game::CON_CHANNEL_SOUND,
                         "Unknown mss32.dll, not patching DirectSound release\n");
        return;
      }
    }

    for (const auto& s: sites)
    {
      auto* p (m + s.rva);

      // Push the slot address: push edi for the first two sites and, for the
      // last one, push eax after its mov eax, [esp+14h], which we keep.
      //
      if (s.push == 1)
        Utils::Hook::Set<std::uint8_t> (p, 0x57);
      else
        Utils::Hook::Set<std::uint8_t> (p + 4, 0x50);

      Utils::Hook (p + s.push, ReleaseDirectSound, HOOK_CALL).install ()->quick ();
      Utils::Hook::Nop (p + s.push + 5, s.code.size () - s.push - 5);
    }
  }

  Sound::
  Sound ()
  {
    Utils::Hook (0x0463A80, Sound::Init, HOOK_JUMP).install ()->quick ();
    Utils::Hook (0x04A23E6, Sound::Loop, HOOK_JUMP).install ()->quick ();

    PatchMiles ();

    if (!Dedicated::IsEnabled () && !ZoneBuilder::IsEnabled ())
    {
      Utils::Hook::Nop (0x60BE8E, 5); // Com_Init: SND_InitDriver().
      Utils::Hook (0x60BE98, CL_InitRenderer, HOOK_CALL).install ()->quick (); // Com_Init: CL_InitRenderer().

      // Loaded sounds with a sample rate above the driver's limit are
      // downsampled as they are loaded. But this limit is only set when the
      // driver is initialized and is 0 until then. Since we now initialize
      // the driver after the renderer, the sounds in the fastfiles it loads
      // (menus, turrets, etc) would be downsampled to nothing and play
      // silent. So we lift the limit until the driver sets it.
      //
      Utils::Hook::Set<std::uint32_t> (max_rate, 0x7FFFFFFF);
    }
  }
}
