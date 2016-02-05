#include "STDInclude.hpp"

namespace Components
{
	WINDOW* Console::OutputWindow;
	WINDOW* Console::InputWindow;
	WINDOW* Console::InfoWindow;

	int Console::OutputTop = 0;
	int Console::OutBuffer = 0;
	int Console::LastRefresh = 0;

	int Console::Height = 25;
	int Console::Width = 80;

	char Console::LineBuffer[1024] = { 0 };
	char Console::LineBuffer2[1024] = { 0 };
	int Console::LineBufferIndex = 0;

	bool Console::HasConsole = false;

	char** Console::GetAutoCompleteFileList(const char *path, const char *extension, Game::FsListBehavior_e behavior, int *numfiles, int allocTrackType)
	{
		if (path == reinterpret_cast<char*>(0xBAADF00D) || path == reinterpret_cast<char*>(0xCDCDCDCD) || IsBadReadPtr(path, 1)) return nullptr;
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

	void Console::RefreshStatus()
	{
		std::string mapname = Dvar::Var("mapname").Get<const char*>();
		std::string hostname = Colors::Strip(Dvar::Var("sv_hostname").Get<const char*>());
		SetConsoleTitle(hostname.data());

		int clientCount = 0;
		int maxclientCount = *Game::svs_numclients;

		if (maxclientCount)
		{
			for (int i = 0; i < maxclientCount; ++i)
			{
				if (Game::svs_clients[i].state >= 3)
				{
					clientCount++;
				}
			}
		}
		else
		{
			//maxclientCount = Dvar::Var("sv_maxclients").Get<int>();
			maxclientCount = Game::Party_GetMaxPlayers(*Game::partyIngame);
			clientCount = Game::PartyHost_CountMembers((Game::PartyData_s*)0x1081C00);
		}

		wclear(Console::InfoWindow);
		wprintw(Console::InfoWindow, "%s : %d/%d players : map %s", hostname.data(), clientCount, maxclientCount, (mapname.size() ? mapname.data() : "none"));
		wnoutrefresh(Console::InfoWindow);
	}

	void Console::ShowPrompt()
	{
		wattron(Console::InputWindow, COLOR_PAIR(10) | A_BOLD);
		wprintw(Console::InputWindow, "%s> ", VERSION_STR);
	}

	void Console::RefreshOutput()
	{
		prefresh(Console::OutputWindow, ((Console::OutputTop > 0) ? (Console::OutputTop - 1) : 0), 0, 1, 0, Console::Height - 2, Console::Width);
	}

	void Console::ScrollOutput(int amount)
	{
		Console::OutputTop += amount;

		if (Console::OutputTop > OUTPUT_MAX_TOP)
		{
			Console::OutputTop = OUTPUT_MAX_TOP;
		}
		else if (Console::OutputTop < 0)
		{
			Console::OutputTop = 0;
		}

		// make it only scroll the top if there's more than HEIGHT lines
		if (Console::OutBuffer >= 0)
		{
			Console::OutBuffer += amount;

			if (Console::OutBuffer >= Console::Height)
			{
				Console::OutBuffer = -1;
			}

			if (Console::OutputTop < Console::Height)
			{
				Console::OutputTop = 0;
			}
		}
	}

	const char* Console::Input()
	{
		if (!Console::HasConsole)
		{
			Console::ShowPrompt();
			wrefresh(Console::InputWindow);
			Console::HasConsole = true;
		}

		int currentTime = static_cast<int>(GetTickCount64()); // Make our compiler happy
		if ((currentTime - Console::LastRefresh) > 250)
		{
			Console::RefreshOutput();
			Console::RefreshStatus();
			Console::LastRefresh = currentTime;
		}

		int c = wgetch(Console::InputWindow);

		if (c == ERR)
		{
			return NULL;
		}

		switch (c)
		{
		case '\r':
		case 459: // keypad enter
		{
			wattron(Console::OutputWindow, COLOR_PAIR(10) | A_BOLD);
			wprintw(Console::OutputWindow, "%s", "]");

			if (Console::LineBufferIndex)
			{
				wprintw(Console::OutputWindow, "%s", Console::LineBuffer);
			}

			wprintw(Console::OutputWindow, "%s", "\n");
			wattroff(Console::OutputWindow, A_BOLD);
			wclear(Console::InputWindow);

			Console::ShowPrompt();

			wrefresh(Console::InputWindow);

			Console::ScrollOutput(1);
			Console::RefreshOutput();

			if (Console::LineBufferIndex)
			{
				strcpy(Console::LineBuffer2, Console::LineBuffer);
				strcat(Console::LineBuffer, "\n");
				Console::LineBufferIndex = 0;
				return Console::LineBuffer;
			}

			break;
		}
		case 'c' - 'a' + 1: // ctrl-c
		case 27:
		{
			Console::LineBuffer[0] = '\0';
			Console::LineBufferIndex = 0;

			wclear(Console::InputWindow);

			Console::ShowPrompt();

			wrefresh(Console::InputWindow);
			break;
		}
		case 8: // backspace
		{
			if (Console::LineBufferIndex > 0)
			{
				Console::LineBufferIndex--;
				Console::LineBuffer[Console::LineBufferIndex] = '\0';

				wprintw(Console::InputWindow, "%c %c", static_cast<char>(c), static_cast<char>(c));
				wrefresh(Console::InputWindow);
			}
			break;
		}
		case KEY_PPAGE:
		{
			Console::ScrollOutput(-1);
			Console::RefreshOutput();
			break;
		}
		case KEY_NPAGE:
		{
			Console::ScrollOutput(1);
			Console::RefreshOutput();
			break;
		}
		case KEY_UP:
		{
			wclear(Console::InputWindow);
			Console::ShowPrompt();
			wprintw(Console::InputWindow, "%s", Console::LineBuffer2);
			wrefresh(Console::InputWindow);

			strcpy(Console::LineBuffer, Console::LineBuffer2);
			Console::LineBufferIndex = strlen(Console::LineBuffer);
			break;
		}
		default:
			if (c <= 127 && Console::LineBufferIndex < 1022)
			{
				// temporary workaround , find out what overwrites our index later on
				//consoleLineBufferIndex = strlen(consoleLineBuffer);

				Console::LineBuffer[Console::LineBufferIndex++] = static_cast<char>(c);
				Console::LineBuffer[Console::LineBufferIndex] = '\0';
				wprintw(Console::InputWindow, "%c", static_cast<char>(c));
				wrefresh(Console::InputWindow);
			}
			break;
		}

		return NULL;
	}

	void Console::Destroy()
	{
		delwin(Console::OutputWindow);
		delwin(Console::InputWindow);
		delwin(Console::InfoWindow);
		endwin();
	}

	void Console::Create()
	{
		Console::OutputTop = 0;
		Console::OutBuffer = 0;
		Console::LastRefresh = 0;
		Console::LineBufferIndex = 0;
		Console::HasConsole = false;

		CONSOLE_SCREEN_BUFFER_INFO info;
		if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info))
		{
			Console::Width = info.dwSize.X;
			Console::Height = info.srWindow.Bottom - info.srWindow.Top + 1;
		}
		else
		{
			Console::Height = 25;
			Console::Width = 80;
		}

		initscr();
		raw();
		noecho();

		Console::OutputWindow = newpad(OUTPUT_HEIGHT, Console::Width);
		Console::InputWindow = newwin(1, Console::Width, Console::Height - 1, 0);
		Console::InfoWindow = newwin(1, Console::Width, 0, 0);

		scrollok(Console::OutputWindow, true);
		scrollok(Console::InputWindow, true);
		nodelay(Console::InputWindow, true);
		keypad(Console::InputWindow, true);

		if (has_colors())
		{
			start_color();
			init_pair(1, COLOR_BLACK, COLOR_WHITE);
			init_pair(2, COLOR_WHITE, COLOR_BLACK);
			init_pair(3, COLOR_RED, COLOR_BLACK);
			init_pair(4, COLOR_GREEN, COLOR_BLACK);
			init_pair(5, COLOR_YELLOW, COLOR_BLACK);
			init_pair(6, COLOR_BLUE, COLOR_BLACK);
			init_pair(7, COLOR_CYAN, COLOR_BLACK);
			init_pair(8, COLOR_RED, COLOR_BLACK);
			init_pair(9, COLOR_WHITE, COLOR_BLACK);
			init_pair(10, COLOR_WHITE, COLOR_BLACK);
		}

		wbkgd(Console::InfoWindow, COLOR_PAIR(1));

		wrefresh(Console::InfoWindow);
		wrefresh(Console::InputWindow);

		Console::RefreshOutput();
	}

	void Console::Error(const char* format, ...)
	{
		static char buffer[32768];

		va_list va;
		va_start(va, format);
		_vsnprintf(buffer, sizeof(buffer), format, va);
		va_end(va);

		Game::Com_Printf(0, "ERROR:\n");
		Game::Com_Printf(0, buffer);

		Console::RefreshOutput();

		if (IsDebuggerPresent())
		{
			while (true)
			{
				Sleep(5000);
			}
		}

		TerminateProcess(GetCurrentProcess(), 0xDEADDEAD);
	}

	void Console::Print(const char* message)
	{
		const char* p = message;
		while (*p != '\0')
		{
			if (*p == '\n')
			{
				Console::ScrollOutput(1);
			}

			if (*p == '^')
			{
				char color;
				p++;

				color = (*p - '0');

				if (color < 9 && color > 0)
				{
					wattron(Console::OutputWindow, COLOR_PAIR(color + 2));
					p++;
					continue;
				}
			}

			waddch(Console::OutputWindow, *p);

			p++;
		}

		wattron(Console::OutputWindow, COLOR_PAIR(9));

// 		int currentTime = static_cast<int>(GetTickCount64()); // Make our compiler happy
// 
// 		if (!Console::HasConsole)
// 		{
// 			Console::RefreshOutput();
// 		}
// 		else if ((currentTime - Console::LastRefresh) > 100)
// 		{
// 			Console::RefreshOutput();
// 			Console::LastRefresh = currentTime;
// 		}

		Console::RefreshOutput();
	}

	Console::Console()
	{
		// Console '%s: %s> ' string
		Utils::Hook::Set<char*>(0x5A44B4, "IW4x: r" REVISION_STR "> ");

		// Internal console
		Utils::Hook(0x4F690C, Console::ToggleConsole, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x4F65A5, Console::ToggleConsole, HOOK_JUMP).Install()->Quick();

		// Check for bad food ;)
		Utils::Hook(0x4CB9F4, Console::GetAutoCompleteFileList, HOOK_CALL).Install()->Quick();

		// Code below is not necessary, when performing unit tests!
		if (Loader::PerformingUnitTests()) return;

		// External console
		if (Flags::HasFlag("console") || ZoneBuilder::IsEnabled()) // ZoneBuilder uses the game's console, until the native one is adapted.
		{
			FreeConsole();
			Utils::Hook::Nop(0x60BB58, 11);
		}
		else if (Dedicated::IsDedicated()/* || ZoneBuilder::IsEnabled()*/)
		{
			Utils::Hook::Nop(0x60BB58, 11);

			Utils::Hook(0x4305E0, Console::Create, HOOK_JUMP).Install()->Quick();
			Utils::Hook(0x4528A0, Console::Destroy, HOOK_JUMP).Install()->Quick();
			Utils::Hook(0x4B2080, Console::Print, HOOK_JUMP).Install()->Quick();
			Utils::Hook(0x43D570, Console::Error, HOOK_JUMP).Install()->Quick();
			Utils::Hook(0x4859A5, Console::Input, HOOK_CALL).Install()->Quick();
		}
		else
		{
			FreeConsole();
		}
	}
}
