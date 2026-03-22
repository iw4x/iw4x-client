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
			push edx                       	 // Arg 4
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

	Sound::
	Sound ()
	{
		Utils::Hook (0x0463A80, Sound::Init, HOOK_JUMP).install ()->quick ();
		Utils::Hook (0x04A23E6, Sound::Loop, HOOK_JUMP).install ()->quick ();
	}
}
