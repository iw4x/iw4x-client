#include "STDInclude.hpp"

namespace Components
{
	char** Console::GetAutoCompleteFileList(const char *path, const char *extension, Game::FsListBehavior_e behavior, int *numfiles, int allocTrackType)
	{
		if (path == (char*)0xBAADF00D || path == (char*)0xCDCDCDCD || IsBadReadPtr(path, 1)) return nullptr;
		return Game::FS_ListFiles(path, extension, behavior, numfiles, allocTrackType);
	}

	void Console::ToggleConsole()
	{
		// possibly cls.keyCatchers?
		Utils::Hook::Xor<DWORD>(0xB2C538, 1);

		// g_consoleField
		Game::Field_Clear((void*)0xA1B6B0);

		// show console output?
		Utils::Hook::Set<BYTE>(0xA15F38, 0);
	}

	Console::Console()
	{
		// External console
		if (Flags::HasFlag("console") || Dedicated::IsDedicated() || ZoneBuilder::IsEnabled())
		{
			Utils::Hook::Nop(0x60BB58, 11);
		}

		// Console '%s: %s> ' string
		Utils::Hook::Set<char*>(0x5A44B4, "IW4x: r" REVISION_STR "> ");

		// Internal console
		Utils::Hook(0x4F690C, Console::ToggleConsole, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x4F65A5, Console::ToggleConsole, HOOK_JUMP).Install()->Quick();

		// Check for bad food ;)
		Utils::Hook(0x4CB9F4, Console::GetAutoCompleteFileList, HOOK_CALL).Install()->Quick();
	}
}
