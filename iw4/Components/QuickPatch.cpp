#include "..\STDInclude.hpp"

namespace Components
{
	QuickPatch::QuickPatch()
	{
		// remove system pre-init stuff (improper quit, disk full)
		Utils::Hook::Set<BYTE>(0x411350, 0xC3);

		// remove STEAMSTART checking for DRM IPC
		Utils::Hook::Nop(0x451145, 5);
		Utils::Hook::Set<BYTE>(0x45114C, 0xEB);

		// disable playlist download function
		Utils::Hook::Set<BYTE>(0x4D4790, 0xC3);

		// disable playlist.ff loading function
		Utils::Hook::Set<BYTE>(0x4D6E60, 0xC3);

		// playlist dvar 'validity check'
		Utils::Hook::Set<BYTE>(0x4B1170, 0xC3);

		//Got playlists is true
		Utils::Hook::Set<bool>(0x1AD3680, true);

		// LSP disabled
		Utils::Hook::Set<BYTE>(0x435950, 0xC3); // LSP HELLO
		Utils::Hook::Set<BYTE>(0x49C220, 0xC3); // We wanted to send a logging packet, but we haven't connected to LSP!
		Utils::Hook::Set<BYTE>(0x4BD900, 0xC3); // main LSP response func
		Utils::Hook::Set<BYTE>(0x682170, 0xC3); // Telling LSP that we're playing a private match

		// Don't delete config files if corrupted
		Utils::Hook::Set<BYTE>(0x47DCB3, 0xEB);

		// hopefully allow alt-tab during game, used at least in alt-enter handling
		Utils::Hook::Set<DWORD>(0x45ACE0, 0xC301B0);

		// fs_basegame
		Utils::Hook::Set<char*>(0x6431D1, "data");

		// Why?
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_WEAPON, 2400);
	}
}
