#include <STDInclude.hpp>

#define REMOVE_HEADERBAR 1

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

	char Console::LineBuffer[1024] = {0};
	char Console::LineBuffer2[1024] = {0};
	int Console::LineBufferIndex = 0;

	bool Console::HasConsole = false;
	bool Console::SkipShutdown = false;

	COLORREF Console::TextColor = 
#if DEBUG
		RGB(255, 200, 117);
#else
		RGB(120, 237, 122);
#endif

	COLORREF Console::BackgroundColor =
#if DEBUG
		RGB(35, 21, 0);
#else
		RGB(25, 32, 25);
#endif
	HBRUSH Console::ForegroundBrush = CreateSolidBrush(TextColor);
	HBRUSH Console::BackgroundBrush = CreateSolidBrush(BackgroundColor);

	HANDLE Console::CustomConsoleFont;

	std::thread Console::ConsoleThread;

	Game::SafeArea Console::OriginalSafeArea;

	const char** Console::GetAutoCompleteFileList(const char* path, const char* extension, Game::FsListBehavior_e behavior, int* numfiles, int allocTrackType)
	{
		if (path == reinterpret_cast<char*>(0xBAADF00D) || path == reinterpret_cast<char*>(0xCDCDCDCD) || ::Utils::Memory::IsBadReadPtr(path)) return nullptr;
		return Game::FS_ListFiles(path, extension, behavior, numfiles, allocTrackType);
	}

	void Console::RefreshStatus()
	{
		const std::string mapname = (*Game::sv_mapname)->current.string;
		const auto hostname = TextRenderer::StripColors((*Game::sv_hostname)->current.string);

		if (Console::HasConsole)
		{
			SetConsoleTitleA(hostname.data());

			auto clientCount = 0;
			auto maxclientCount = *Game::svs_clientCount;

			if (maxclientCount)
			{
				for (int i = 0; i < maxclientCount; ++i)
				{
					if (Game::svs_clients[i].header.state >= Game::CS_CONNECTED)
					{
						++clientCount;
					}
				}
			}
			else
			{
				maxclientCount = Dvar::Var("party_maxplayers").get<int>();
				//maxclientCount = Game::Party_GetMaxPlayers(*Game::partyIngame);
				clientCount = Game::PartyHost_CountMembers(reinterpret_cast<Game::PartyData*>(0x1081C00));
			}

			wclear(Console::InfoWindow);
			wprintw(Console::InfoWindow, "%s : %d/%d players : map %s", hostname.data(), clientCount, maxclientCount, (!mapname.empty()) ? mapname.data() : "none");
			wnoutrefresh(Console::InfoWindow);
		}
		else if (IsWindow(Console::GetWindow()) != FALSE)
		{
			SetWindowTextA(Console::GetWindow(), Utils::String::VA("IW4x(" VERSION ") : %s", hostname.data()));
		}
	}

	void Console::ShowPrompt()
	{
		wattron(Console::InputWindow, COLOR_PAIR(10) | A_BOLD);
		wprintw(Console::InputWindow, "%s> ", VERSION);
	}

	void Console::RefreshOutput()
	{
		prefresh(Console::OutputWindow, ((Console::OutputTop > 0) ? (Console::OutputTop - 1) : 0), 0, 1, 0, Console::Height - 2, Console::Width - 1);
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

	float Console::GetDpiScale(const HWND hWnd)
	{
		const auto user32 = Utils::Library("user32.dll");
		const auto getDpiForWindow = user32.getProc<UINT(WINAPI*)(HWND)>("GetDpiForWindow");
		const auto getDpiForMonitor = user32.getProc<HRESULT(WINAPI*)(HMONITOR, int, UINT*, UINT*)>("GetDpiForMonitor");
		
		int dpi;

		if (getDpiForWindow)
		{
			dpi = getDpiForWindow(hWnd);
		}
		else if (getDpiForMonitor)
		{
			HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
			UINT xdpi, ydpi;
			LRESULT success = getDpiForMonitor(hMonitor, 0, &xdpi, &ydpi);
			if (success == S_OK)
			{
				dpi = static_cast<int>(ydpi);
			}
			
			dpi = 96;
		}
		else
		{
			HDC hDC = GetDC(hWnd);
			INT ydpi = GetDeviceCaps(hDC, LOGPIXELSY);
			ReleaseDC(NULL, hDC);
			
			dpi = ydpi;
		}

		constexpr auto unawareDpi = 96.0f;
		return dpi / unawareDpi;
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
			Console::LastRefresh = currentTime;
		}

		int c = wgetch(Console::InputWindow);

		if (c == ERR)
		{
			return nullptr;
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
				strcpy_s(Console::LineBuffer2, Console::LineBuffer);
				strcat_s(Console::LineBuffer, "\n");
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

			strcpy_s(Console::LineBuffer, Console::LineBuffer2);
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

		return nullptr;
	}

	void Console::Destroy()
	{
		__try
		{
			delwin(Console::OutputWindow);
			delwin(Console::InputWindow);
			delwin(Console::InfoWindow);
			endwin();
			delscreen(SP);
		}
		__finally {}

		Console::OutputWindow = nullptr;
		Console::InputWindow = nullptr;
		Console::InfoWindow = nullptr;
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

		Console::OutputWindow = newpad(Console::Height - 1, Console::Width);
		Console::InputWindow = newwin(1, Console::Width, Console::Height - 1, 0);
		Console::InfoWindow = newwin(1, Console::Width, 0, 0);

		scrollok(Console::OutputWindow, true);
		idlok(Console::OutputWindow, true);
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

	void Console::Error(const char* fmt, ...)
	{
		char buf[4096] = {0};

		va_list va;
		va_start(va, fmt);
		vsnprintf_s(buf, _TRUNCATE, fmt, va);
		va_end(va);

		Logger::PrintError(Game::CON_CHANNEL_ERROR, "{}\n", buf);

		Console::RefreshOutput();

		if (IsDebuggerPresent())
		{
			while (true)
			{
				std::this_thread::sleep_for(5s);
			}
		}

		TerminateProcess(GetCurrentProcess(), 0xDEADDEAD);
	}

	void Console::Print(const char* message)
	{
		if (!Console::OutputWindow) return;

		const char* p = message;
		while (*p != '\0')
		{
			if (*p == '^')
			{
				char color;
				++p;

				color = (*p - '0');

				if (color < 9 && color > 0)
				{
					wattron(Console::OutputWindow, COLOR_PAIR(color + 2));
					++p;
					continue;
				}
			}

			waddch(Console::OutputWindow, *p);

			++p;
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

	HFONT __stdcall Console::ReplaceFont(
		[[maybe_unused]] int    cHeight,
		int    cWidth,
		int    cEscapement,
		int    cOrientation,
		[[maybe_unused]] int    cWeight,
		DWORD  bItalic,
		DWORD  bUnderline,
		DWORD  bStrikeOut,
		DWORD  iCharSet,
		[[maybe_unused]] DWORD  iOutPrecision,
		DWORD  iClipPrecision,
		[[maybe_unused]] DWORD  iQuality,
		[[maybe_unused]] DWORD  iPitchAndFamily,
		[[maybe_unused]] LPCSTR pszFaceName)
	{
		auto font = CreateFontA(
			12, 
			cWidth, 
			cEscapement, 
			cOrientation, 
			700, 
			bItalic, 
			bUnderline,
			bStrikeOut, 
			iCharSet, 
			OUT_RASTER_PRECIS,
			iClipPrecision, 
			NONANTIALIASED_QUALITY,
			0x31, 
			"Terminus (TTF)"); // Terminus (TTF)

		return font;
	}

	void Console::GetWindowPos(HWND hWnd, int* x, int* y)
	{
		HWND hWndParent = GetParent(hWnd);
		POINT p = { 0 };

		MapWindowPoints(hWnd, hWndParent, &p, 1);

		(*x) = p.x;
		(*y) = p.y;
	}

	BOOL CALLBACK Console::ResizeChildWindow(HWND hwndChild, LPARAM lParam)
	{
		auto id = GetWindowLong(hwndChild, GWL_ID);
		bool isInputBox = id == INPUT_BOX;
		bool isOutputBox = id == OUTPUT_BOX;

		if (isInputBox || isOutputBox) 
		{
			RECT newParentRect = *reinterpret_cast<LPRECT>(lParam);

			RECT childRect;

			if (GetWindowRect(hwndChild, &childRect)) 
			{

				int childX, childY;

				GetWindowPos(hwndChild, &childX, &childY);

				HWND parent = Utils::Hook::Get<HWND>(0x64A3288);

				float scale = GetDpiScale(parent);

				if (isInputBox) 
				{

					int newX = childX; // No change!
					int newY = static_cast<int>((newParentRect.bottom - newParentRect.top) - 65 * scale);
					int newWidth = static_cast<int>((newParentRect.right - newParentRect.left) - 29 * scale);
					int newHeight = static_cast<int>((childRect.bottom - childRect.top) * scale); // No change!

					MoveWindow(hwndChild, newX, newY, newWidth, newHeight, TRUE);
				}
				
				if (isOutputBox)
				{
					int newX = childX; // No change!
					int newY = childY; // No change!
					int newWidth = static_cast<int>((newParentRect.right - newParentRect.left) - 29);

					int margin = 70;

#ifdef REMOVE_HEADERBAR
					margin = 10;
#endif
					int newHeight = static_cast<int>((newParentRect.bottom - newParentRect.top) - 74 * scale - margin);

					MoveWindow(hwndChild, newX, newY, newWidth, newHeight, TRUE);
				}
			}
		}

		return TRUE;
	}

	// Instead of clearing fully the console text whenever the 0x400's character is written, we
	//	clear it progressively when we run out of room by truncating the top line by line.
	// A bit of trickery with SETREDRAW is required to avoid having the outputbox jump
	//	around whenever clearing occurs.
	void Console::MakeRoomForText([[maybe_unused]] int addedCharacters)
	{
		constexpr unsigned int maxChars = 0x4000;
		constexpr unsigned int maxAffectedChars = 0x100;
		HWND outputBox = Utils::Hook::Get<HWND>(0x64A328C);

		unsigned int totalChars;
		unsigned int totalClearLength = 0;

		char str[maxAffectedChars];
		unsigned int fetchedCharacters = static_cast<unsigned int>(GetWindowText(outputBox, str, maxAffectedChars));

		totalChars = GetWindowTextLengthA(outputBox);

		while (totalChars - totalClearLength > maxChars)
		{
			unsigned int clearLength = maxAffectedChars; // Default to full clear

			for (size_t i = 0; i < fetchedCharacters; i++)
			{
				if (str[i] == '\n')
				{
					// Shorter clear if I meet a linebreak
					clearLength = i + 1;
					break;
				}
			}

			totalClearLength += clearLength;
		}

		if (totalClearLength > 0)
		{
			SendMessage(outputBox, WM_SETREDRAW, FALSE, 0);
			SendMessage(outputBox, EM_SETSEL, 0, totalClearLength);
			SendMessage(outputBox, EM_REPLACESEL, FALSE, 0);
			SendMessage(outputBox, WM_SETREDRAW, TRUE, 0);
		}

		Utils::Hook::Set(0x64A38B8, totalChars - totalClearLength);
	}

	void __declspec(naked) Console::Sys_PrintStub()
	{
		__asm
		{
			pushad
			push edi
			call MakeRoomForText
			pop edi
			popad

			// Go back to AppendText
			push 0x4F57F8
			ret
		}
	}

	LRESULT CALLBACK Console::ConWndProc(HWND hWnd, UINT Msg, WPARAM wParam, unsigned int lParam)
	{
		switch (Msg)
		{

		
		case WM_CREATE:
		{
			BOOL darkMode = true;

#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
			if (SUCCEEDED(DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, reinterpret_cast<LPCVOID>(&darkMode), sizeof(darkMode))))
			{
				// cool !
			}

			break;
		}

		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLOREDIT:
		{
			SetBkColor(reinterpret_cast<HDC>(wParam), BackgroundColor);
			SetTextColor(reinterpret_cast<HDC>(wParam), TextColor);
			return reinterpret_cast<LRESULT>(BackgroundBrush);
		}

		case WM_SIZE:
			RECT rect;

			if (GetWindowRect(hWnd, &rect))
			{
				EnumChildWindows(hWnd, ResizeChildWindow, reinterpret_cast<LPARAM>(&rect));
			}

			return 0;
		}

		// Fall through to basegame
		return Utils::Hook::Call<LRESULT CALLBACK(HWND, UINT, WPARAM, unsigned int)>(0x64DC50)(hWnd, Msg, wParam, lParam);
	}

	ATOM CALLBACK Console::RegisterClassHook(WNDCLASSA* lpWndClass)
	{
		DeleteObject(lpWndClass->hbrBackground);
		HBRUSH brush = CreateSolidBrush(BackgroundColor);
		lpWndClass->hbrBackground = brush;

		return RegisterClassA(lpWndClass);
	}

	void Console::ApplyConsoleStyle() 
	{
		Utils::Hook::Set<BYTE>(0x428A8E, 0);    // Adjust logo Y pos
		Utils::Hook::Set<BYTE>(0x428A90, 0);    // Adjust logo X pos
		Utils::Hook::Set<BYTE>(0x428AF2, 67);   // Adjust output Y pos
		Utils::Hook::Set<DWORD>(0x428AC5, 397); // Adjust input Y pos
		Utils::Hook::Set<DWORD>(0x428951, 609); // Reduce window width
		Utils::Hook::Set<DWORD>(0x42895D, 423); // Reduce window height
		Utils::Hook::Set<DWORD>(0x428AC0, 597); // Reduce input width
		Utils::Hook::Set<DWORD>(0x428AED, 596); // Reduce output width

		DWORD fontsInstalled;
		CustomConsoleFont = AddFontMemResourceEx(const_cast<void*>(reinterpret_cast<const void*>(Font::Terminus::DATA)), Font::Terminus::LENGTH, 0, &fontsInstalled);

		if (fontsInstalled > 0)
		{
			Utils::Hook::Nop(0x428A44, 6);
			Utils::Hook(0x428A44, ReplaceFont, HOOK_CALL).install()->quick();
		}

		Utils::Hook::Nop(0x42892D, 6);
		Utils::Hook(0x42892D, RegisterClassHook, HOOK_CALL).install()->quick();

		Utils::Hook::Set(0x4288E6 + 4, &ConWndProc);

		auto style = WS_CAPTION | WS_SIZEBOX | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
		Utils::Hook::Set(0x42893F + 1, style);
		Utils::Hook::Set(0x4289E2 + 1, style);

#ifdef REMOVE_HEADERBAR
		// Remove that hideous header window -rox
		Utils::Hook::Set(0x428A7C, static_cast<char>(0xEB));
		Utils::Hook::Set(0X428AF1 + 1, static_cast<char>(10));
#endif

		// Never reset text
		Utils::Hook::Nop(0x4F57DF, 0x4F57F6 - 0x4F57DF);
		Utils::Hook(0x4F57DF, Console::Sys_PrintStub, HOOK_JUMP).install()->quick();

	}

	void Console::ConsoleRunner()
	{
		Console::SkipShutdown = false;
		Game::Sys_ShowConsole();

		MSG message;
		while (IsWindow(Console::GetWindow()) != FALSE && GetMessageA(&message, nullptr, 0, 0))
		{
			TranslateMessage(&message);
			DispatchMessageA(&message);
		}

		if (Console::SkipShutdown) return;

		if (Game::Sys_Milliseconds() - Console::LastRefresh > 100 &&
			MessageBoxA(nullptr, "The application is not responding anymore, do you want to force its termination?", "Application is not responding", MB_ICONEXCLAMATION | MB_YESNO) == IDYES)
		{
			// Force process termination
			// if the main thread is not responding
			OutputDebugStringA("Process termination forced, as the main thread is not responding!");

			// We can not force the termination in this thread
			// The destructor would be called in this thread
			// and would try to join this thread, which is impossible
			TerminateProcess(GetCurrentProcess(), 0xFFFFFFFF);
		}
		else
		{
			// Send quit command to safely terminate the application
			Command::Execute("wait 200;quit\n", false);
		}
	}

	void Console::StdOutPrint(const char* message)
	{
		printf("%s", message);
		fflush(stdout);
	}

	void Console::StdOutError(const char* fmt, ...)
	{
		char buffer[4096] = {0};

		va_list ap;
		va_start(ap, fmt);
		_vsnprintf_s(buffer, _TRUNCATE, fmt, ap);
		va_end(ap);

		perror(buffer);
		fflush(stderr);

		ExitProcess(1);
	}

	__declspec(naked) void Console::DrawSolidConsoleStub()
	{
		__asm
		{
			pushad
			call Console::StoreSafeArea
			popad

			// We need esi preserved here, so we have to backup 'all' registers when storing the safearea
			call Game::Con_DrawSolidConsole

			pushad
			call Console::RestoreSafeArea
			popad
			retn
		}
	}

	void Console::StoreSafeArea()
	{
		// Backup the original safe area
		Console::OriginalSafeArea = *Game::safeArea;

		// Apply new safe area and border
		float border = 6.0f;
		Game::safeArea->top = border;
		Game::safeArea->left = border;
		Game::safeArea->bottom = static_cast<float>(Renderer::Height()) - border;
		Game::safeArea->right = static_cast<float>(Renderer::Width()) - border;

		Game::safeArea->textHeight = static_cast<int>((Game::safeArea->bottom - Game::safeArea->top - (2 * Game::safeArea->fontHeight) - 24.0) / Game::safeArea->fontHeight);
		Game::safeArea->textWidth = static_cast<int>(Game::safeArea->right - Game::safeArea->left - 10.0f - 18.0);
	}

	void Console::RestoreSafeArea()
	{
		// Restore the initial safe area
		*Game::safeArea = Console::OriginalSafeArea;
	}

	void Console::SetSkipShutdown()
	{
		Console::SkipShutdown = true;
	}

	void Console::FreeNativeConsole()
	{
		if (!Monitor::IsEnabled() && !Flags::HasFlag("stdout") && (!Dedicated::IsEnabled() || Flags::HasFlag("console")) && !Loader::IsPerformingUnitTests())
		{
			FreeConsole();
		}
	}

	HWND Console::GetWindow()
	{
		return *reinterpret_cast<HWND*>(0x64A3288);
	}

	void Console::ShowAsyncConsole()
	{
		Console::ConsoleThread = std::thread(Console::ConsoleRunner);
	}

	Game::dvar_t* Console::RegisterConColor(const char* dvarName, float r, float g, float b, float a, float min,
		float max, unsigned __int16 flags, const char* description)
	{
		static struct
		{
			const char* name;
			float color[4];
		} patchedColors[] =
		{
			{ "con_inputBoxColor",     { 0.20f, 0.20f, 0.20f, 1.00f } },
			{ "con_inputHintBoxColor", { 0.30f, 0.30f, 0.30f, 1.00f } },
			{ "con_outputBarColor",    { 0.50f, 0.50f, 0.50f, 0.60f } },
			{ "con_outputSliderColor", { 0.70f, 1.00f, 0.00f, 1.00f } },
			{ "con_outputWindowColor", { 0.25f, 0.25f, 0.25f, 0.85f } },
		};

		for (std::size_t i = 0; i < ARRAYSIZE(patchedColors); ++i)
		{
			if (std::strcmp(dvarName, patchedColors[i].name) == 0)
			{
				r = patchedColors[i].color[0];
				g = patchedColors[i].color[1];
				b = patchedColors[i].color[2];
				a = patchedColors[i].color[3];
				break;
			}
		}

		return reinterpret_cast<Game::Dvar_RegisterVec4_t>(0x471500)(dvarName, r, g, b, a, min, max, flags, description);
	}

	void Console::Con_ToggleConsole()
	{
		Game::Field_Clear(Game::g_consoleField);
		if (Game::conDrawInputGlob->matchIndex >= 0 && Game::conDrawInputGlob->autoCompleteChoice[0] != '\0')
		{
			Game::conDrawInputGlob->matchIndex = -1;
			Game::conDrawInputGlob->autoCompleteChoice[0] = '\0';
		}

		Game::g_consoleField->fixedSize = 1;
		Game::con->outputVisible = false;
		Game::g_consoleField->widthInPixels = *Game::g_console_field_width;
		Game::g_consoleField->charHeight = *Game::g_console_char_height;

		for (std::size_t localClientNum = 0; localClientNum < Game::MAX_LOCAL_CLIENTS; ++localClientNum)
		{
			assert((Game::clientUIActives[0].keyCatchers & Game::KEYCATCH_CONSOLE) == (Game::clientUIActives[localClientNum].keyCatchers & Game::KEYCATCH_CONSOLE));
			Game::clientUIActives[localClientNum].keyCatchers ^= 1;
		}
	}

	void Console::AddConsoleCommand()
	{
		Command::Add("con_echo", []
		{
			Console::Con_ToggleConsole();
			Game::I_strncpyz(Game::g_consoleField->buffer, "\\echo ", sizeof(Game::field_t::buffer));
			Game::g_consoleField->cursor = static_cast<int>(std::strlen(Game::g_consoleField->buffer));
			Game::Field_AdjustScroll(Game::ScrPlace_GetFullPlacement(), Game::g_consoleField);
		});
	}

	Console::Console()
	{
		AssertOffset(Game::clientUIActive_t, connectionState, 0x9B8);
		AssertOffset(Game::clientUIActive_t, keyCatchers, 0x9B0);

		// Console '%s: %s> ' string
		Utils::Hook::Set<const char*>(0x5A44B4, "IW4x MP: " VERSION "> ");

		// Patch console color
		static float consoleColor[] = { 0.70f, 1.00f, 0.00f, 1.00f };
		Utils::Hook::Set<float*>(0x5A451A, consoleColor);
		Utils::Hook::Set<float*>(0x5A4400, consoleColor);
		
		// Remove the need to type '\' or '/' to send a console command
		Utils::Hook::Set<BYTE>(0x431565, 0xEB);
		
		// Internal console
		Utils::Hook(0x4F690C, Console::Con_ToggleConsole, HOOK_CALL).install()->quick();
		Utils::Hook(0x4F65A5, Console::Con_ToggleConsole, HOOK_JUMP).install()->quick();

		// Patch safearea for ingame-console
		Utils::Hook(0x5A50EF, Console::DrawSolidConsoleStub, HOOK_CALL).install()->quick();

		// Check for bad food ;)
		Utils::Hook(0x4CB9F4, Console::GetAutoCompleteFileList, HOOK_CALL).install()->quick();

		// Patch console dvars
		Utils::Hook(0x4829AB, Console::RegisterConColor, HOOK_CALL).install()->quick();
		Utils::Hook(0x4829EE, Console::RegisterConColor, HOOK_CALL).install()->quick();
		Utils::Hook(0x482A31, Console::RegisterConColor, HOOK_CALL).install()->quick();
		Utils::Hook(0x482A7A, Console::RegisterConColor, HOOK_CALL).install()->quick();
		Utils::Hook(0x482AC3, Console::RegisterConColor, HOOK_CALL).install()->quick();

		// Modify console style
		ApplyConsoleStyle();

		// Don't resize the console
		Utils::Hook(0x64DC6B, 0x64DCC2, HOOK_JUMP).install()->quick();

#ifdef _DEBUG
		Console::AddConsoleCommand();
#endif

		if (Dedicated::IsEnabled() && !ZoneBuilder::IsEnabled())
		{
			Scheduler::Loop(Console::RefreshStatus, Scheduler::Pipeline::MAIN);
		}

		// Code below is not necessary when performing unit tests!
		if (Loader::IsPerformingUnitTests()) return;

		// External console
		if (Flags::HasFlag("stdout") || Monitor::IsEnabled())
		{
#ifndef DEBUG
			if (!Monitor::IsEnabled())
#endif
			{
				Utils::Hook(0x4B2080, Console::StdOutPrint, HOOK_JUMP).install()->quick();
				Utils::Hook(0x43D570, Console::StdOutError, HOOK_JUMP).install()->quick();
			}
		}
		else if (Flags::HasFlag("console") || ZoneBuilder::IsEnabled()) // ZoneBuilder uses the game's console, until the native one is adapted.
		{
			Utils::Hook::Nop(0x60BB58, 11);

			// Redirect input (]command)
			Utils::Hook(0x47025A, 0x4F5770, HOOK_CALL).install()->quick();

			Utils::Hook(0x60BB68, []()
			{
				Console::ShowAsyncConsole();
			}, HOOK_CALL).install()->quick();

			Utils::Hook(0x4D69A2, []()
			{
				Console::SetSkipShutdown();

				// Sys_DestroyConsole
				Utils::Hook::Call<void()>(0x4528A0)();

				if (Console::ConsoleThread.joinable())
				{
					Console::ConsoleThread.join();
				}
			}, HOOK_CALL).install()->quick();

			Scheduler::Loop([]
			{
				Console::LastRefresh = Game::Sys_Milliseconds();
			}, Scheduler::Pipeline::MAIN);
		}
		else if (Dedicated::IsEnabled()/* || ZoneBuilder::IsEnabled()*/)
		{
			DWORD type = GetFileType(GetStdHandle(STD_INPUT_HANDLE));
			if (type != FILE_TYPE_CHAR)
			{
				MessageBoxA(nullptr, "Console not supported, please use '-stdout' or '-console' flag!", "ERRROR", MB_ICONERROR);
				TerminateProcess(GetCurrentProcess(), 1);
			}

			Utils::Hook::Nop(0x60BB58, 11);

			Utils::Hook(0x4305E0, Console::Create, HOOK_JUMP).install()->quick();
			Utils::Hook(0x4528A0, Console::Destroy, HOOK_JUMP).install()->quick();
			Utils::Hook(0x4B2080, Console::Print, HOOK_JUMP).install()->quick();
			Utils::Hook(0x43D570, Console::Error, HOOK_JUMP).install()->quick();
			Utils::Hook(0x4859A5, Console::Input, HOOK_CALL).install()->quick();
		}
		else if(!Loader::IsPerformingUnitTests())
		{
			FreeConsole();
		}
	}

	Console::~Console()
	{
		Console::SetSkipShutdown();
		if (Console::ConsoleThread.joinable())
		{
			Console::ConsoleThread.join();
		}
	}
}
