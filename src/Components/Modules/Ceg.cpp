#include <STDInclude.hpp>
#include "Ceg.hpp"

namespace Components
{
	Ceg::Ceg()
	{
		Utils::Hook::Signature signature(0x401000, 0x340000);

		// Generic killer caller.
		signature.add({
			"\x56\x8B\x00\x24\x0c\x85\xF6\x7F\x0E", "xx?xxxxxx", [](char* address)
			{
				Utils::Hook::Set<std::uint8_t>(address, 0xC3);
			}
		});

		signature.process();

		// Some more generic obfuscation (mov al, 1; retn)
		Utils::Hook::Set<std::uint32_t>(0x471B20, 0xC301B0);
		Utils::Hook::Set<std::uint32_t>(0x43A070, 0xC301B0);
		Utils::Hook::Set<std::uint32_t>(0x4C8B30, 0xC301B0);
		Utils::Hook::Set<std::uint32_t>(0x469340, 0xC301B0);

		// Other checks
		Utils::Hook::Set<std::uint32_t>(0x401000, 0xC301B0);
		Utils::Hook::Set<std::uint32_t>(0x45F8B0, 0xC301B0);
		Utils::Hook::Set<std::uint32_t>(0x46FAE0, 0xC301B0);

		// Removed in 159 SP binaries
		Utils::Hook::Nop(0x46B173, 9);
		Utils::Hook::Nop(0x43CA16, 9);
		Utils::Hook::Nop(0x505426, 9);

		// Removed on IW5 MP (unprotected) but present on IW5 SP (protected) - CEG uninitialization / Steam Shutdown
		Utils::Hook::Set<std::uint8_t>(0x4F6370, 0xC3);

		// Remove 'Steam Start' checking for DRM IPC
		Utils::Hook::Nop(0x451145, 5);
		Utils::Hook::Set<BYTE>(0x45114C, 0xEB);

		// Disable some checks on certain game events
		Utils::Hook::Nop(0x43EC96, 9);
		Utils::Hook::Nop(0x4675C6, 9);
		Utils::Hook::Nop(0x405A36, 9);
		Utils::Hook::Nop(0x4CE656, 9);
		Utils::Hook::Nop(0x461E66, 9);
		Utils::Hook::Nop(0x4EB3F6, 9);

		// Random checks scattered throughout the binary
		Utils::Hook::Set<std::uint8_t>(0x499F90, 0xC3);
		Utils::Hook::Set<std::uint8_t>(0x4FC700, 0xC3);
		Utils::Hook::Set<std::uint8_t>(0x4C4170, 0xC3);
		Utils::Hook::Set<std::uint8_t>(0x49E8C0, 0xC3);
		Utils::Hook::Set<std::uint8_t>(0x42DB00, 0xC3);
		Utils::Hook::Set<std::uint8_t>(0x4F4CF0, 0xC3);
		Utils::Hook::Set<std::uint8_t>(0x432180, 0xC3);
		Utils::Hook::Set<std::uint8_t>(0x461930, 0xC3);
		Utils::Hook::Set<std::uint8_t>(0x430410, 0xC3);

		// Looking for stuff in the registry
		Utils::Hook::Nop(0x4826F8, 5);
	}
}
