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
// 		if (!Dedicated::IsDedicated())
// 		{
// 			FreeConsole();
// 			return;
// 		}
// 
// 		initscr();
// 		raw();
// 		noecho();
// 
// 		// patch functions
// #define WIDTH 80
// #define HEIGHT 25
// #define OUTPUT_HEIGHT 250
// #define OUTPUT_MAX_TOP (OUTPUT_HEIGHT - (HEIGHT - 2))
// 
// 		WINDOW* outputWindow = newpad(OUTPUT_HEIGHT, WIDTH);
// 		WINDOW* inputWindow = newwin(1, WIDTH, HEIGHT - 1, 0);
// 		WINDOW* infoWindow = newwin(1, WIDTH, 0, 0);
// 
// 		scrollok(outputWindow, true);
// 		scrollok(inputWindow, true);
// 		nodelay(inputWindow, true);
// 		keypad(inputWindow, true);
// 
// 		if (has_colors())
// 		{
// 			start_color();
// 			init_pair(1, COLOR_BLACK, COLOR_WHITE);
// 			init_pair(2, COLOR_WHITE, COLOR_BLACK);
// 			init_pair(3, COLOR_RED, COLOR_BLACK);
// 			init_pair(4, COLOR_GREEN, COLOR_BLACK);
// 			init_pair(5, COLOR_YELLOW, COLOR_BLACK);
// 			init_pair(6, COLOR_BLUE, COLOR_BLACK);
// 			init_pair(7, COLOR_CYAN, COLOR_BLACK);
// 			init_pair(8, COLOR_RED, COLOR_BLACK);
// 			init_pair(9, COLOR_WHITE, COLOR_BLACK);
// 			init_pair(10, COLOR_WHITE, COLOR_BLACK);
// 			//init_pair(2, COLOR_WHITE, COLOR_BLACK);
// 		}
// 
// 		wbkgd(infoWindow, COLOR_PAIR(1));
// 
// 		wrefresh(infoWindow);
// 		//wrefresh(outputWindow);
// 		wrefresh(inputWindow);
// 
// 		//prefresh(outputWindow, (currentOutputTop >= 1) ? currentOutputTop - 1 : 0, 0, 1, 0, HEIGHT - 2, WIDTH);
// 
// 		waddch(outputWindow, 'g');
// 		waddch(outputWindow, 'i');
// 
// 		prefresh(outputWindow, 0, 0, 1, 0, HEIGHT - 2, WIDTH);

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
