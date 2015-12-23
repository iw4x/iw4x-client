#include "..\STDInclude.hpp"

namespace Components
{
	void Console::ToggleConsole()
	{
		// possibly cls.keyCatchers?
		DWORD catcher = 0xB2C538;
		Utils::Hook::Set<DWORD>(catcher, Utils::Hook::Get<DWORD>(catcher) ^ 1);

		// g_consoleField
		Game::Field_Clear((void*)0xA1B6B0);

		// show console output?
		Utils::Hook::Set<BYTE>(0xA15F38, 0);
	}

	Console::Console()
	{
		// External console
		Utils::Hook::Nop(0x60BB58, 11);

		// Console '%s: %s> ' string
		Utils::Hook::Set<char*>(0x5A44B4, "IW4x > ");

		// Internal console
		Utils::Hook(0x4F690C, Console::ToggleConsole, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x4F65A5, Console::ToggleConsole, HOOK_JUMP).Install()->Quick();
	}
}
