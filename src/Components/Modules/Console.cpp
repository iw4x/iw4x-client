#include <STDInclude.hpp>
#include "Console.hpp"
#include "TextRenderer.hpp"

#include "Terminus_4.49.1.ttf.hpp"

#include <version.hpp>

#ifdef MOUSE_MOVED
	#undef MOUSE_MOVED
#endif

#include <curses.h>

#define REMOVE_HEADERBAR 1

namespace Components
{
	static WINDOW* OutputWindow;
	static WINDOW* InputWindow;
	static WINDOW* InfoWindow;

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
#ifdef _DEBUG
		RGB(255, 200, 117);
#else
		RGB(120, 237, 122);
#endif

	COLORREF Console::BackgroundColor =
#ifdef _DEBUG
		RGB(35, 21, 0);
#else
		RGB(25, 32, 25);
#endif
	HBRUSH Console::ForegroundBrush = CreateSolidBrush(TextColor);
	HBRUSH Console::BackgroundBrush = CreateSolidBrush(BackgroundColor);

	HANDLE Console::CustomConsoleFont;

	std::thread Console::ConsoleThread;

	Game::SafeArea Console::OriginalSafeArea;

	bool Console::isCommand;

	const char** Console::GetAutoCompleteFileList(const char* path, const char* extension, Game::FsListBehavior_e behavior, int* numfiles, int allocTrackType)
	{
		if (path == reinterpret_cast<char*>(0xBAADF00D) || path == reinterpret_cast<char*>(0xCDCDCDCD) || ::Utils::Memory::IsBadReadPtr(path)) return nullptr;
		return Game::FS_ListFiles(path, extension, behavior, numfiles, allocTrackType);
	}

	void Console::RefreshStatus()
	{
		const std::string mapname = (*Game::sv_mapname)->current.string;
		const auto hostname = TextRenderer::StripColors((*Game::sv_hostname)->current.string);

		if (HasConsole)
		{
			SetConsoleTitleA(hostname.data());

			auto clientCount = 0;
			auto maxClientCount = *Game::svs_clientCount;

			if (maxClientCount)
			{
				for (auto i = 0; i < maxClientCount; ++i)
				{
					if (Game::svs_clients[i].header.state >= Game::CS_CONNECTED)
					{
						++clientCount;
					}
				}
			}
			else
			{
				maxClientCount = *Game::party_maxplayers ? (*Game::party_maxplayers)->current.integer : 18;
				clientCount = Game::PartyHost_CountMembers(Game::g_lobbyData);
			}

			wclear(InfoWindow);
			wprintw(InfoWindow, "%s : %d/%d players : map %s", hostname.data(), clientCount, maxClientCount, (!mapname.empty()) ? mapname.data() : "none");
			wnoutrefresh(InfoWindow);
		}
		else if (IsWindow(GetWindow()) != FALSE)
		{
#ifdef EXPERIMENTAL_BUILD
			SetWindowTextA(GetWindow(), Utils::String::Format("IW4x " REVISION_STR "-develop : {}", hostname));
#else
			SetWindowTextA(GetWindow(), Utils::String::Format("IW4x " REVISION_STR " : {}", hostname));
#endif
		}
	}

	void Console::ShowPrompt()
	{
		wattron(InputWindow, COLOR_PAIR(10) | A_BOLD);
#ifdef EXPERIMENTAL_BUILD
		wprintw(InputWindow, "%s-develop> ", REVISION_STR);
#else
		wprintw(InputWindow, "%s> ", REVISION_STR);
#endif
	}

	void Console::RefreshOutput()
	{
		prefresh(OutputWindow, ((OutputTop > 0) ? (OutputTop - 1) : 0), 0, 1, 0, Height - 2, Width - 1);
	}

	void Console::ScrollOutput(int amount)
	{
		OutputTop += amount;

		if (OutputTop > OUTPUT_MAX_TOP)
		{
			OutputTop = OUTPUT_MAX_TOP;
		}
		else if (OutputTop < 0)
		{
			OutputTop = 0;
		}

		// make it only scroll the top if there's more than HEIGHT lines
		if (OutBuffer >= 0)
		{
			OutBuffer += amount;

			if (OutBuffer >= Height)
			{
				OutBuffer = -1;
			}

			if (OutputTop < Height)
			{
				OutputTop = 0;
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
			dpi = static_cast<int>(getDpiForWindow(hWnd));
		}
		else if (getDpiForMonitor)
		{
			HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
			UINT xdpi, ydpi;
			getDpiForMonitor(hMonitor, 0, &xdpi, &ydpi);
			
			dpi = 96;
		}
		else
		{
			HDC hDC = GetDC(hWnd);
			INT ydpi = GetDeviceCaps(hDC, LOGPIXELSY);
			ReleaseDC(nullptr, hDC);
			
			dpi = ydpi;
		}

		constexpr auto unawareDpi = 96.0f;
		return static_cast<float>(dpi) / unawareDpi;
	}


	const char* Console::Input()
	{
		if (!HasConsole)
		{
			ShowPrompt();
			wrefresh(InputWindow);
			HasConsole = true;
		}

		auto currentTime = static_cast<int>(GetTickCount64()); // Make our compiler happy
		if ((currentTime - LastRefresh) > 250)
		{
			RefreshOutput();
			LastRefresh = currentTime;
		}

		auto c = wgetch(InputWindow);

		if (c == ERR)
		{
			return nullptr;
		}

		switch (c)
		{
		case '\r':
		case 459: // keypad enter
		{
			wattron(OutputWindow, COLOR_PAIR(10) | A_BOLD);
			wprintw(OutputWindow, "%s", "]");

			if (LineBufferIndex)
			{
				wprintw(OutputWindow, "%s", LineBuffer);
			}

			wprintw(OutputWindow, "%s", "\n");
			wattroff(OutputWindow, A_BOLD);
			wclear(InputWindow);

			ShowPrompt();

			wrefresh(InputWindow);

			ScrollOutput(1);
			RefreshOutput();

			if (LineBufferIndex)
			{
				strcpy_s(LineBuffer2, LineBuffer);
				strcat_s(LineBuffer, "\n");
				LineBufferIndex = 0;
				return LineBuffer;
			}

			break;
		}
		case 'c' - 'a' + 1: // ctrl-c
		case 27:
		{
			LineBuffer[0] = '\0';
			LineBufferIndex = 0;

			wclear(InputWindow);

			ShowPrompt();

			wrefresh(InputWindow);
			break;
		}
		case 8: // backspace
		{
			if (LineBufferIndex > 0)
			{
				LineBufferIndex--;
				LineBuffer[LineBufferIndex] = '\0';

				wprintw(InputWindow, "%c %c", static_cast<char>(c), static_cast<char>(c));
				wrefresh(InputWindow);
			}
			break;
		}
		case KEY_PPAGE:
		{
			ScrollOutput(-1);
			RefreshOutput();
			break;
		}
		case KEY_NPAGE:
		{
			ScrollOutput(1);
			RefreshOutput();
			break;
		}
		case KEY_UP:
		{
			wclear(InputWindow);
			ShowPrompt();
			wprintw(InputWindow, "%s", LineBuffer2);
			wrefresh(InputWindow);

			strcpy_s(LineBuffer, LineBuffer2);
			LineBufferIndex = static_cast<int>(std::strlen(LineBuffer));
			break;
		}
		default:
			if (c <= 127 && LineBufferIndex < 1022)
			{
				// temporary workaround, find out what overwrites our index later on
				//consoleLineBufferIndex = strlen(consoleLineBuffer);

				LineBuffer[LineBufferIndex++] = static_cast<char>(c);
				LineBuffer[LineBufferIndex] = '\0';
				wprintw(InputWindow, "%c", static_cast<char>(c));
				wrefresh(InputWindow);
			}
			break;
		}

		return nullptr;
	}

	void Console::Destroy()
	{
		__try
		{
			delwin(OutputWindow);
			delwin(InputWindow);
			delwin(InfoWindow);
			endwin();
			delscreen(SP);
		}
		__finally {}

		OutputWindow = nullptr;
		InputWindow = nullptr;
		InfoWindow = nullptr;
	}

	void Console::Create()
	{
		OutputTop = 0;
		OutBuffer = 0;
		LastRefresh = 0;
		LineBufferIndex = 0;
		HasConsole = false;

		CONSOLE_SCREEN_BUFFER_INFO info;
		if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info))
		{
			Width = info.dwSize.X;
			Height = info.srWindow.Bottom - info.srWindow.Top + 1;
		}
		else
		{
			Height = 25;
			Width = 80;
		}

		initscr();
		raw();
		noecho();

		OutputWindow = newpad(Height - 1, Width);
		InputWindow = newwin(1, Width, Height - 1, 0);
		InfoWindow = newwin(1, Width, 0, 0);

		scrollok(OutputWindow, true);
		idlok(OutputWindow, true);
		scrollok(InputWindow, true);
		nodelay(InputWindow, true);
		keypad(InputWindow, true);

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

		wbkgd(InfoWindow, COLOR_PAIR(1));

		wrefresh(InfoWindow);
		wrefresh(InputWindow);

		RefreshOutput();
	}

	void Console::Error(const char* fmt, ...)
	{
		char buf[4096] = {0};

		va_list va;
		va_start(va, fmt);
		vsnprintf_s(buf, _TRUNCATE, fmt, va);
		va_end(va);

		Logger::PrintError(Game::CON_CHANNEL_ERROR, "{}\n", buf);

		RefreshOutput();

#ifdef _DEBUG
		if (IsDebuggerPresent())
		{
			while (true)
			{
				std::this_thread::sleep_for(5s);
			}
		}
#endif

		TerminateProcess(GetCurrentProcess(), EXIT_FAILURE);
	}

	void Console::Print(const char* message)
	{
		if (!OutputWindow) return;

		const char* p = message;
		while (*p != '\0')
		{
			if (*p == '^')
			{
				++p;

				const char color = (*p - '0');
				if (color < 9 && color > 0)
				{
					wattron(OutputWindow, COLOR_PAIR(color + 2));
					++p;
					continue;
				}
			}

			waddch(OutputWindow, *p);

			++p;
		}

		wattron(OutputWindow, COLOR_PAIR(9));

		RefreshOutput();
	}

	HFONT CALLBACK Console::ReplaceFont([[maybe_unused]] int cHeight, int cWidth, int cEscapement, int cOrientation, [[maybe_unused]] int cWeight, DWORD bItalic, DWORD bUnderline,
	                                    DWORD bStrikeOut, DWORD iCharSet, [[maybe_unused]] DWORD iOutPrecision, DWORD iClipPrecision, [[maybe_unused]] DWORD iQuality,
	                                    [[maybe_unused]] DWORD iPitchAndFamily, [[maybe_unused]] LPCSTR pszFaceName)
	{
		HFONT font = CreateFontA(12, cWidth, cEscapement, cOrientation, 700, bItalic,
		                         bUnderline, bStrikeOut, iCharSet, OUT_RASTER_PRECIS,
		                         iClipPrecision, NONANTIALIASED_QUALITY, 0x31,
		                         "Terminus (TTF)"
		);

		return font;
	}

	void Console::GetWindowPos(HWND hWnd, int* x, int* y)
	{
		HWND hWndParent = GetParent(hWnd);
		POINT p{};

		MapWindowPoints(hWnd, hWndParent, &p, 1);

		(*x) = p.x;
		(*y) = p.y;
	}

	BOOL CALLBACK Console::ResizeChildWindow(HWND hwndChild, LPARAM lParam)
	{
		auto id = GetWindowLong(hwndChild, GWL_ID);
		auto isInputBox = id == INPUT_BOX;
		auto isOutputBox = id == OUTPUT_BOX;

		if (isInputBox || isOutputBox) 
		{
			RECT newParentRect = *reinterpret_cast<LPRECT>(lParam);

			RECT childRect;

			if (GetWindowRect(hwndChild, &childRect)) 
			{

				int childX, childY;

				GetWindowPos(hwndChild, &childX, &childY);

				HWND parent = Utils::Hook::Get<HWND>(0x64A3288);

				auto scale = GetDpiScale(parent);

				if (isInputBox) 
				{

					auto newX = childX; // No change!
					auto newY = static_cast<int>((newParentRect.bottom - newParentRect.top) - 65 * scale);
					auto newWidth = static_cast<int>((newParentRect.right - newParentRect.left) - 29 * scale);
					auto newHeight = static_cast<int>((childRect.bottom - childRect.top) * scale); // No change!

					MoveWindow(hwndChild, newX, newY, newWidth, newHeight, TRUE);
				}
				
				if (isOutputBox)
				{
					auto newX = childX; // No change!
					auto newY = childY; // No change!
					auto newWidth = static_cast<int>((newParentRect.right - newParentRect.left) - 29);

#ifdef REMOVE_HEADERBAR
					constexpr auto margin = 10;
#else
					constexpr auto margin = 70;
#endif
					auto newHeight = static_cast<int>((newParentRect.bottom - newParentRect.top) - 74 * scale - margin);

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
		constexpr auto maxChars = 0x4000;
		constexpr auto maxAffectedChars = 0x100;
		HWND outputBox = Utils::Hook::Get<HWND>(0x64A328C);

		auto totalClearLength = 0;

		char str[maxAffectedChars];
		const auto fetchedCharacters = GetWindowTextA(outputBox, str, maxAffectedChars);

		auto totalChars = GetWindowTextLengthA(outputBox);
		while (totalChars - totalClearLength > maxChars)
		{
			auto clearLength = maxAffectedChars; // Default to full clear

			for (auto i = 0; i < fetchedCharacters; i++)
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
			SendMessageA(outputBox, WM_SETREDRAW, FALSE, 0);
			SendMessageA(outputBox, EM_SETSEL, 0, totalClearLength);
			SendMessageA(outputBox, EM_REPLACESEL, FALSE, 0);
			SendMessageA(outputBox, WM_SETREDRAW, TRUE, 0);
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
			BOOL darkMode = TRUE;
			constexpr auto DWMWA_USE_IMMERSIVE_DARK_MODE = 20;
			DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));
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
		Utils::Hook::Set<std::uint8_t>(0x428A8E, 0);    // Adjust logo Y pos
		Utils::Hook::Set<std::uint8_t>(0x428A90, 0);    // Adjust logo X pos
		Utils::Hook::Set<std::uint8_t>(0x428AF2, 67);   // Adjust output Y pos
		Utils::Hook::Set<std::uint32_t>(0x428AC5, 397); // Adjust input Y pos
		Utils::Hook::Set<std::uint32_t>(0x428951, 609); // Reduce window width
		Utils::Hook::Set<std::uint32_t>(0x42895D, 423); // Reduce window height
		Utils::Hook::Set<std::uint32_t>(0x428AC0, 597); // Reduce input width
		Utils::Hook::Set<std::uint32_t>(0x428AED, 596); // Reduce output width

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
		Utils::Hook(0x4F57DF, Sys_PrintStub, HOOK_JUMP).install()->quick();

	}

	void Console::ConsoleRunner()
	{
		SkipShutdown = false;
		Game::Sys_ShowConsole();

		MSG message;
		while (IsWindow(GetWindow()) != FALSE && GetMessageA(&message, nullptr, 0, 0))
		{
			TranslateMessage(&message);
			DispatchMessageA(&message);
		}

		if (SkipShutdown) return;

		if (Game::Sys_Milliseconds() -LastRefresh > 100 &&
			MessageBoxA(nullptr, "The application is not responding anymore, do you want to force its termination?", "Application is not responding", MB_ICONEXCLAMATION | MB_YESNO) == IDYES)
		{
			// Force process termination
			// if the main thread is not responding
#ifdef _DEBUG
			OutputDebugStringA("Process termination was forced as the main thread is not responding!");
#endif

			// We can not force the termination in this thread
			// The destructor would be called in this thread
			// and would try to join this thread, which is impossible
			TerminateProcess(GetCurrentProcess(), EXIT_FAILURE);
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
		vsnprintf_s(buffer, _TRUNCATE, fmt, ap);
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
		OriginalSafeArea = *Game::safeArea;

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
		*Game::safeArea = OriginalSafeArea;
	}

	void Console::SetSkipShutdown()
	{
		SkipShutdown = true;
	}

	void Console::FreeNativeConsole()
	{
		if (!Flags::HasFlag("stdout") && (!Dedicated::IsEnabled() || Flags::HasFlag("console")) && !Loader::IsPerformingUnitTests())
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
		ConsoleThread = std::thread(ConsoleRunner);
	}

	Game::dvar_t* Console::RegisterConColor(const char* dvarName, float r, float g, float b, float a, float min, float max, unsigned __int16 flags, const char* description)
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

	bool Console::Con_IsDvarCommand_Stub(const char* cmd)
	{
		isCommand = Game::Con_IsDvarCommand(cmd);
		return isCommand;
	}

	void Console::Cmd_ForEach_Stub(void(*callback)(const char* str))
	{
		if (!isCommand)
		{
			Game::Cmd_ForEach(callback);
		}
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
			Con_ToggleConsole();
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
#ifdef EXPERIMENTAL_BUILD
		Utils::Hook::Set<const char*>(0x5A44B4, "IW4x MP: " REVISION_STR "-develop> ");
#else
		Utils::Hook::Set<const char*>(0x5A44B4, "IW4x MP: " REVISION_STR "> ");
#endif

		// Patch console color
		static float consoleColor[] = { 0.70f, 1.00f, 0.00f, 1.00f };
		Utils::Hook::Set<float*>(0x5A451A, consoleColor);
		Utils::Hook::Set<float*>(0x5A4400, consoleColor);
		
		// Remove the need to type '\' or '/' to send a console command
		Utils::Hook::Set<std::uint8_t>(0x431565, 0xEB);
		
		// Internal console
		Utils::Hook(0x4F690C, Con_ToggleConsole, HOOK_CALL).install()->quick();
		Utils::Hook(0x4F65A5, Con_ToggleConsole, HOOK_JUMP).install()->quick();

		// Allow the client console to always be opened (sv_allowClientConsole)
		Utils::Hook::Nop(0x4F68EC, 2);

		// Patch safearea for ingame-console
		Utils::Hook(0x5A50EF, DrawSolidConsoleStub, HOOK_CALL).install()->quick();

		// Check for bad food ;)
		Utils::Hook(0x4CB9F4, GetAutoCompleteFileList, HOOK_CALL).install()->quick();

		// Patch console dvars
		Utils::Hook(0x4829AB, RegisterConColor, HOOK_CALL).install()->quick();
		Utils::Hook(0x4829EE, RegisterConColor, HOOK_CALL).install()->quick();
		Utils::Hook(0x482A31, RegisterConColor, HOOK_CALL).install()->quick();
		Utils::Hook(0x482A7A, RegisterConColor, HOOK_CALL).install()->quick();
		Utils::Hook(0x482AC3, RegisterConColor, HOOK_CALL).install()->quick();

		// Modify console style
		ApplyConsoleStyle();

		// Don't resize the console
		Utils::Hook(0x64DC6B, 0x64DCC2, HOOK_JUMP).install()->quick();

		// Con_DrawInput
		Utils::Hook(0x5A45BD, Con_IsDvarCommand_Stub, HOOK_CALL).install()->quick();
		Utils::Hook(0x5A466C, Cmd_ForEach_Stub, HOOK_CALL).install()->quick();

#ifdef _DEBUG
		AddConsoleCommand();
#endif

		if (Dedicated::IsEnabled() && !ZoneBuilder::IsEnabled())
		{
			Scheduler::Loop(RefreshStatus, Scheduler::Pipeline::MAIN);
		}

		// Code below is not necessary when performing unit tests!
		if (Loader::IsPerformingUnitTests()) return;

		// External console
		if (Flags::HasFlag("stdout"))
		{
			Utils::Hook(0x4B2080, StdOutPrint, HOOK_JUMP).install()->quick();
			Utils::Hook(0x43D570, StdOutError, HOOK_JUMP).install()->quick();
		}
		else if (Flags::HasFlag("console") || ZoneBuilder::IsEnabled()) // ZoneBuilder uses the game's console, until the native one is adapted.
		{
			Utils::Hook::Nop(0x60BB58, 11);

			// Redirect input (]command)
			Utils::Hook(0x47025A, 0x4F5770, HOOK_CALL).install()->quick();

			Utils::Hook(0x60BB68, []
			{
				ShowAsyncConsole();
			}, HOOK_CALL).install()->quick();

			Utils::Hook(0x4D69A2, []
			{
				SetSkipShutdown();

				// Sys_DestroyConsole
				Utils::Hook::Call<void()>(0x4528A0)();

				if (ConsoleThread.joinable())
				{
					ConsoleThread.join();
				}
			}, HOOK_CALL).install()->quick();

			Scheduler::Loop([]
			{
				LastRefresh = Game::Sys_Milliseconds();
			}, Scheduler::Pipeline::MAIN);
		}
		else if (Dedicated::IsEnabled())
		{
			DWORD type = GetFileType(GetStdHandle(STD_INPUT_HANDLE));
			if (type != FILE_TYPE_CHAR)
			{
				MessageBoxA(nullptr, "Console not supported, please use '-stdout' or '-console' flag!", "ERRROR", MB_ICONERROR);
				TerminateProcess(GetCurrentProcess(), EXIT_FAILURE);
			}

			Utils::Hook::Nop(0x60BB58, 11);

			Utils::Hook(0x4305E0, Create, HOOK_JUMP).install()->quick();
			Utils::Hook(0x4528A0, Destroy, HOOK_JUMP).install()->quick();
			Utils::Hook(0x4B2080, Print, HOOK_JUMP).install()->quick();
			Utils::Hook(0x43D570, Error, HOOK_JUMP).install()->quick();
			Utils::Hook(0x4859A5, Input, HOOK_CALL).install()->quick();
		}
		else if(!Loader::IsPerformingUnitTests())
		{
			FreeConsole();
		}
	}

	Console::~Console()
	{
		SetSkipShutdown();
		if (ConsoleThread.joinable())
		{
			ConsoleThread.join();
		}
	}
}
