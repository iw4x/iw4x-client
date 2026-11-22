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

  std::jthread Console::ConsoleThread;

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
    delwin(OutputWindow);
    delwin(InputWindow);
    delwin(InfoWindow);
    endwin();
    delscreen(SP);

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
    const LONG_PTR id = GetWindowLongPtrA(hwndChild, GWLP_ID);
    const bool isInputBox = id == INPUT_BOX;
    const bool isOutputBox = id == OUTPUT_BOX;

    if (isInputBox || isOutputBox)
    {
      RECT newParentRect = *reinterpret_cast<LPRECT>(lParam);

      RECT childRect;

      if (GetWindowRect(hwndChild, &childRect))
      {

        int childX, childY;

        GetWindowPos(hwndChild, &childX, &childY);

        const float scale = GetDpiScale(Game::s_wcd->hWnd);

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
  //  clear it progressively when we run out of room by truncating the top line by line.
  // A bit of trickery with SETREDRAW is required to avoid having the outputbox jump
  //  around whenever clearing occurs.
  void Console::MakeRoomForText([[maybe_unused]] int addedCharacters)
  {
    constexpr auto maxChars = 0x4000;
    constexpr auto maxAffectedChars = 0x100;
    HWND outputBox = Game::s_wcd->hwndBuffer;

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

    *Game::s_totalChars = totalChars - totalClearLength;
  }

  LRESULT CALLBACK Console::ConWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
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

    return Game::ConWndProc(hWnd, Msg, wParam, lParam);
  }

  ATOM CALLBACK Console::RegisterClassHook(WNDCLASSA* lpWndClass)
  {
    DeleteObject(lpWndClass->hbrBackground);
    HBRUSH brush = CreateSolidBrush(BackgroundColor);
    lpWndClass->hbrBackground = brush;
    lpWndClass->lpfnWndProc = ConWndProc;

    return RegisterClassA(lpWndClass);
  }

  LONG_PTR Console::SubclassInputLine(HWND hWnd, int nIndex, LONG_PTR dwNewLong)
  {
    const LONG_PTR r = SetWindowLongPtrA(hWnd, nIndex, dwNewLong);
    *Game::SysInputLineWndProc = reinterpret_cast<WNDPROC>(r);
    return r;
  }

  void Console::ApplyConsoleStyle()
  {
    Utils::Hook::Set<uint32_t>(0x1402A7D0B, 0);   // Adjust logo Y pos
    Utils::Hook::Set<uint32_t>(0x1402A7D13, 0);   // Adjust logo X pos
    Utils::Hook::Set<uint32_t>(0x1402A7DDB, 67);  // Adjust output Y pos
    Utils::Hook::Set<uint32_t>(0x1402A7D7D, 397); // Adjust input Y pos
    Utils::Hook::Set<uint32_t>(0x1402A7B1C, 609); // Reduce window width
    Utils::Hook::Set<uint32_t>(0x1402A7B24, 423); // Reduce window height
    Utils::Hook::Set<uint32_t>(0x1402A7D75, 597); // Reduce input width
    Utils::Hook::Set<uint32_t>(0x1402A7DD3, 596); // Reduce output width

    DWORD fontsInstalled;
    CustomConsoleFont = AddFontMemResourceEx(const_cast<void*>(reinterpret_cast<const void*>(Font::Terminus::DATA)), Font::Terminus::LENGTH, 0, &fontsInstalled);

    if (fontsInstalled > 0)
    {
      Utils::Hook::Nop(0x1402A7C85, 6);
      Utils::Hook(0x1402A7C85, ReplaceFont, HOOK_CALL).install()->quick();
    }

    // Use our window procedure (and background) for the console window. On
    // x86 we just point the game at ConWndProc(), but on x64 the game loads
    // its procedure with a rip-relative lea, which can't reach our module. So
    // we hook its RegisterClassA() call instead and fill in both in the window
    // class there (see RegisterClassHook()).
    //
    Utils::Hook::Nop(0x1402A7ADF, 6);
    Utils::Hook(0x1402A7ADF, RegisterClassHook, HOOK_CALL).install()->quick();

    const DWORD style = WS_CAPTION | WS_SIZEBOX | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    Utils::Hook::Set(0x1402A7B06 + 1, style);
    Utils::Hook::Set(0x1402A7B9A + 2, style);

#ifdef REMOVE_HEADERBAR
    // Remove that hideous header window -rox
    Utils::Hook::Set<uint8_t>(0x1402A7CCA, 0xEB);
    Utils::Hook::Set<uint32_t>(0x1402A7DDB, 10);
#endif

    // Subclass the input line with full pointers. The game does it with
    // SetWindowLongA(), which takes and returns a 32-bit LONG, and then stores
    // the sign-extended result as the previous window procedure. That only
    // works as long as both procedures happen to fit in 32 bits, which is a
    // bit of a gamble on x64. So we call SetWindowLongPtrA() instead and store
    // the previous procedure ourselves, which is also why we nop the game's
    // store.
    //
    Utils::Hook::Nop(0x1402A7E1E, 6);
    Utils::Hook(0x1402A7E1E, SubclassInputLine, HOOK_CALL).install()->quick();
    Utils::Hook::Nop(0x1402A7E2E, 10);

    // Never reset text
    Utils::Hook::Nop(0x1402A7836, 0x1402A784E - 0x1402A7836);
    Utils::Hook(0x1402A7836, Console_Sys_PrintStub, HOOK_JUMP).install()->quick();

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

  void Console::SetSkipShutdown()
  {
    SkipShutdown = true;
  }

  void Console::FreeNativeConsole()
  {
    if (!Flags::HasFlag("stdout") && (!Dedicated::IsEnabled() || Flags::HasFlag("console")))
    {
      FreeConsole();
    }
  }

  HWND Console::GetWindow()
  {
    return Game::s_wcd->hWnd;
  }

  void Console::ShowAsyncConsole()
  {
    ConsoleThread = std::jthread(ConsoleRunner);
  }

  float Console::GetConsoleScale()
  {
    return Game::Con_GetConsoleScale();
  }

  void (*Console::Con_UpdateNotifyLine_Original)(int localClientNum, int channel, bool lineFeed, int flags);

  void Console::Con_UpdateNotifyLine_Hk(int localClientNum, int channel, bool lineFeed, int flags)
  {
    Game::Con_UpdateConsoleLine(localClientNum, channel, lineFeed, flags);
    Game::Con_UpdateMiniconLine(localClientNum, channel, lineFeed, flags);
    Con_UpdateNotifyLine_Original(localClientNum, channel, lineFeed, flags);
    Game::Con_UpdateErrorLine(localClientNum, channel, lineFeed, flags);
  }

  void Console::Con_OneTimeInit_Hk()
  {
    Game::Con_InitConsoleWindow();
    Con_InitMessageBuffer_Hk();
  }

  void Console::Con_InitMessageBuffer_Hk()
  {
    Utils::Hook::Call<void()>(0x1400ED2C0)(); // Con_InitMessageBuffer
    Game::Con_InitMessageWindows();
  }

  void Console::Con_ClearNotify_Hk(const int localClientNum)
  {
    Utils::Hook::Call<void(int)>(0x1400EC110)(localClientNum); // Con_ClearNotify
    Game::Con_ClearMessageWindows(localClientNum);
  }

  void Console::Con_TimeJumped_Hk(const int localClientNum, const int serverTime)
  {
    Utils::Hook::Call<void(int, int)>(0x1400EDE60)(localClientNum, serverTime); // Con_TimeJumped
    Game::Con_TimeJumpedMessageWindows(localClientNum, serverTime);
  }

  void Console::Con_TimeNudged_Hk(const int localClientNum, const int serverTimeNudge)
  {
    Utils::Hook::Call<void(int, int)>(0x1400EDFD0)(localClientNum, serverTimeNudge); // Con_TimeNudged
    Game::Con_TimeNudgedMessageWindows(localClientNum, serverTimeNudge);
  }

  // x64 inlines Con_UpdateNotifyMessage() into CL_ConsolePrint_AddLine(),
  // and without the mini console and error windows, since it doesn't have
  // them. So we start their messages right before the call.
  //
  // The flag we check is the one CL_ConsolePrint_AddLine() sets while it's
  // already printing, in which case it returns straight away. So we leave
  // the windows be as well, the same as x86 does.
  //
  char Console::CL_ConsolePrint_AddLine_Hk(const int localClientNum, const int channel, const char* txt, const int duration, const int pixelWidth, const char color, const int flags)
  {
    if (!Utils::Hook::Get<int>(0x1406C4F08))
    {
      Game::Con_UpdateMessageWindowsMessage(localClientNum, channel, duration, flags);
    }

    return Utils::Hook::Call<char(int, int, const char*, int, int, char, int)>(0x1400EB890)(localClientNum, channel, txt, duration, pixelWidth, color, flags);
  }

  void Console::Con_Init_Hk()
  {
    Utils::Hook::Call<void()>(0x1400ED0A0)();
    Game::Con_InitInput();
  }

  void Console::Con_InitChannels_Hk()
  {
    Game::Con_InitConsoleChannels();
    Utils::Hook::Call<void()>(0x1400F0030)();
  }

  void Console::SCR_DrawScreenField_Hk()
  {
    if (Game::cls->rendererStarted)
    {
      Game::Con_DrawConsole(0);
    }

    Game::ScrPlace_EndFrame();
  }

  int Console::R_TextHeight_Hk(Game::Font_s* font)
  {
    return static_cast<int>(static_cast<float>(Game::R_TextHeight(font)) * Game::Con_GetFontScale(font));
  }

  int Console::R_TextWidth_Hk(const char* text, int maxChars, Game::Font_s* font)
  {
    return static_cast<int>(static_cast<float>(Game::R_TextWidth(text, maxChars, font)) * Game::Con_GetFontScale(font));
  }

  void Console::CL_DrawTextWithCursor_Hk(Game::ScreenPlacement* scrPlace, const char* text, int maxChars, Game::Font_s* font, float x, float y, int horzAlign, int vertAlign, float xScale, float yScale, const float* color, int style, int cursorPos, char cursorChar)
  {
    const float scale(Game::Con_GetFontScale(font));
    Game::CL_DrawTextWithCursor(scrPlace, text, maxChars, font, x, y, horzAlign, vertAlign, xScale * scale, yScale * scale, color, style, cursorPos, cursorChar);
  }

  Game::dvar_t* Console::RegisterConColor(const char* dvarName, float r, float g, float b, float a, float min, float max, unsigned __int16 flags, const char* description)
  {
    struct ConColor
    {
      const char* name;
      float color[4];
    };

    static constexpr ConColor colors[] =
    {
      {"con_inputBoxColor",     {0.20f, 0.20f, 0.20f, 1.00f}},
      {"con_inputHintBoxColor", {0.30f, 0.30f, 0.30f, 1.00f}},
      {"con_outputBarColor",    {0.50f, 0.50f, 0.50f, 0.60f}},
      {"con_outputSliderColor", {0.70f, 1.00f, 0.00f, 1.00f}},
      {"con_outputWindowColor", {0.25f, 0.25f, 0.25f, 0.85f}},
    };

    for (const ConColor& c: colors)
    {
      if (std::strcmp(dvarName, c.name) == 0)
      {
        r = c.color[0];
        g = c.color[1];
        b = c.color[2];
        a = c.color[3];
        break;
      }
    }

    return Game::Dvar_RegisterVec4(dvarName, r, g, b, a, min, max, flags, description);
  }

  Console::Console()
  {
    // x64 compiles out the in-game console, so we bring our own (see
    // Game::Con_InitConsoleWindow() for why and how). Here we hook it into
    // the places that x86 calls it from. We set it up along with the rest of
    // the console and feed it the lines, then draw it with the screen and
    // hand it the keys.
    //
    Con_UpdateNotifyLine_Original = Utils::Hook::Detour(0x1400EE410, 5, &Con_UpdateNotifyLine_Hk);
    Utils::Hook(0x1400EBC71, Console_CL_ConsolePrintStub, HOOK_JUMP).install()->quick();

    Utils::Hook(0x1400EDDFD, Con_OneTimeInit_Hk, HOOK_CALL).install()->quick();

    // The same goes for the mini console and the error windows. Wherever the
    // game sets up or clears its own message windows, we do the same for
    // ours, and likewise when it moves their messages in time (see
    // Game::Con_InitMessageWindows()).
    //
    Utils::Hook(0x1400F4D73, Con_InitMessageBuffer_Hk, HOOK_CALL).install()->quick();

    for (const auto place : {0x1400EB64C, 0x1400EB6B3, 0x1400F4C29, 0x1400F8948, 0x1400FC480, 0x1400FCABD, 0x1400FFC65})
    {
      Utils::Hook(place, Con_ClearNotify_Hk, HOOK_CALL).install()->quick();
    }

    Utils::Hook(0x1400F475D, Con_TimeJumped_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400F52D1, Con_TimeNudged_Hk, HOOK_CALL).install()->quick();

    for (const auto place : {0x1400EB7F4, 0x1400EB86B, 0x1400EBFAC})
    {
      Utils::Hook(place, CL_ConsolePrint_AddLine_Hk, HOOK_CALL).install()->quick();
    }
    Utils::Hook(0x1400FAF02, Con_Init_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400F0025, Con_InitChannels_Hk, HOOK_JUMP).install()->quick();

    Utils::Hook(0x140101D11, SCR_DrawScreenField_Hk, HOOK_CALL).install()->quick();

    Utils::Hook(0x1400EE892, Console_CL_KeyEventStub, HOOK_JUMP).install()->quick();
    Utils::Hook(0x1400EE95D, Console_CL_KeyEventEscapeStub, HOOK_JUMP).install()->quick();
    Utils::Hook(0x1400EEA62, Console_CL_KeyEventDownStub, HOOK_JUMP).install()->quick();
    Utils::Hook(0x1400EEAC2, Console_CL_KeyEventDisconnectedStub, HOOK_JUMP).install()->quick();
    Utils::Hook(0x1400EE5E0, Game::CL_CharEvent, HOOK_JUMP).install()->quick();

    // Patch console dvars
    //
    Utils::Hook(0x1400ED73C, RegisterConColor, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400ED78D, RegisterConColor, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400ED7E1, RegisterConColor, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400ED82F, RegisterConColor, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400ED881, RegisterConColor, HOOK_CALL).install()->quick();

    // Scale ingame-console with render resolution (raw pixels)
    //
    // Note that on x64 the only game code left that measures console text is
    // Con_CheckResize(), Field_AdjustScroll(), and Field_Draw(), so those are
    // the calls we scale. The rest of what x86 scales here is drawing that x64
    // compiles out, and our own console code does its own scaling.
    //
    Game::con_scale = Dvar::Register<float>("con_scale", 0.0f, 0.0f, 4.0f, Game::DVAR_ARCHIVE, "Size multiplier of the ingame console. 0 derives it from the render resolution.").get<Game::dvar_t*>();

    Utils::Hook(0x1400ED236, R_TextHeight_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400EEB95, R_TextWidth_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400EEC03, R_TextWidth_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400EEC40, R_TextWidth_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400EEC60, R_TextWidth_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400EECF9, R_TextWidth_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400EEFC6, R_TextHeight_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400EF045, CL_DrawTextWithCursor_Hk, HOOK_CALL).install()->quick();

#ifdef _DEBUG
    Command::AddRaw("con_echo", Game::Con_EchoCommand);
#endif

    // Modify console style
    ApplyConsoleStyle();

    // Don't resize the console
    Utils::Hook(0x1402A768B, 0x1402A772B, HOOK_JUMP).install()->quick();

    if (Dedicated::IsEnabled())
    {
      Scheduler::Loop(RefreshStatus, Scheduler::Pipeline::MAIN);
    }

    // External console
    if (Flags::HasFlag("stdout"))
    {
      Utils::Hook(0x1402A78C0, StdOutPrint, HOOK_JUMP).install()->quick();
      Utils::Hook(0x1402A3480, StdOutError, HOOK_JUMP).install()->quick();
    }
    else if (Flags::HasFlag("console"))
    {
      // Redirect input (]command)
      Utils::Hook(0x1402A79B4, Game::Conbuf_AppendText, HOOK_CALL).install()->quick();

      // On x86 we show the console from the block of
      // Com_Init_Try_Block_Function() that only runs on a dedicated server,
      // where the game hides the splash window and shows its console. x64
      // compiles that block out, so there is nothing to hijack and we do both
      // ourselves, right before the call that used to follow the block.
      //
      Utils::Hook(0x1401F50C3, []
      {
        Game::Sys_HideSplashWindow();
        ShowAsyncConsole();
        Utils::Hook::Call<void()>(0x140208B70)();
      }, HOOK_CALL).install()->quick();

      Utils::Hook(0x1402A3DD6, []
      {
        SetSkipShutdown();

        Game::Sys_DestroyConsole();

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

      Utils::Hook(0x1401F50C3, []
      {
        Game::Sys_HideSplashWindow();
        Game::Sys_ShowConsole();
        Utils::Hook::Call<void()>(0x140208B70)();
      }, HOOK_CALL).install()->quick();

      Utils::Hook(0x1402A7F30, Create, HOOK_JUMP).install()->quick();
      Utils::Hook(0x1402A7E90, Destroy, HOOK_JUMP).install()->quick();
      Utils::Hook(0x1402A78C0, Print, HOOK_JUMP).install()->quick();
      Utils::Hook(0x1402A3480, Error, HOOK_JUMP).install()->quick();
      Utils::Hook(0x1402A3708, Input, HOOK_CALL).install()->quick();
    }
    else
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

void Console_MakeRoomForText(int addedCharacters)
{
  Components::Console::MakeRoomForText(addedCharacters);
}

bool Console_CL_ConsoleKeyEvent(int localClientNum, int key, int down)
{
  return Game::CL_ConsoleKeyEvent(localClientNum, key, down);
}

bool Console_Con_CancelAutoComplete()
{
  return Game::Con_CancelAutoComplete();
}

void Console_Console_Key(int localClientNum, int key)
{
  Game::Console_Key(localClientNum, key);
}
