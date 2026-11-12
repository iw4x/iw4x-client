#pragma once

extern "C"
{
  void Console_Sys_PrintStub();
  void Console_CL_ConsolePrintStub();
  void Console_CL_KeyEventStub();
  void Console_CL_KeyEventEscapeStub();
  void Console_CL_KeyEventDownStub();
  void Console_CL_KeyEventDisconnectedStub();

  void Console_MakeRoomForText(int addedCharacters);
  bool Console_CL_ConsoleKeyEvent(int localClientNum, int key, int down);
  bool Console_Con_CancelAutoComplete();
  void Console_Console_Key(int localClientNum, int key);
}

#include "Dvar.hpp"

#define OUTPUT_HEIGHT 250
#define OUTPUT_MAX_TOP (OUTPUT_HEIGHT - (Console::Height - 2))

namespace Components
{
  class Console : public Component
  {
  public:
    Console();
    ~Console();

    static void SetSkipShutdown();

    static void FreeNativeConsole();

    static HWND GetWindow();

    static void ShowAsyncConsole();

    // Size multiplier of the in-game console.
    //
    static float GetConsoleScale();

  private:
    friend void (::Console_MakeRoomForText)(int addedCharacters);

    static constexpr int OUTPUT_BOX = 0x64;
    static constexpr int INPUT_BOX = 0x65;

    static int Width;
    static int Height;

    static int OutputTop;
    static int OutBuffer;
    static int LastRefresh;

    static COLORREF TextColor;
    static COLORREF BackgroundColor;
    static HBRUSH ForegroundBrush;
    static HBRUSH BackgroundBrush;

    static HANDLE CustomConsoleFont;

    static char LineBuffer[1024];
    static char LineBuffer2[1024];
    static int LineBufferIndex;

    static bool HasConsole;
    static bool SkipShutdown;

    static std::jthread ConsoleThread;

    static void ShowPrompt();
    static void RefreshStatus();
    static void RefreshOutput();
    static void ScrollOutput(int amount);

    static const char* Input();
    static void Print(const char* message);
    static void Error(const char* fmt, ...);
    static void Create();
    static void Destroy();

    static void StdOutPrint(const char* message);
    static void StdOutError(const char* fmt, ...);

    static void ConsoleRunner();

    static LRESULT CALLBACK ConWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
    static ATOM CALLBACK RegisterClassHook(WNDCLASSA* lpWndClass);
    static LONG_PTR SubclassInputLine(HWND hWnd, int nIndex, LONG_PTR dwNewLong);
    static BOOL CALLBACK ResizeChildWindow(HWND hwndChild, LPARAM lParam);
    static HFONT CALLBACK ReplaceFont(int cHeight, int cWidth, int cEscapement, int cOrientation, int cWeight, DWORD bItalic, DWORD bUnderline, DWORD bStrikeOut, DWORD iCharSet, DWORD iOutPrecision, DWORD iClipPrecision, DWORD iQuality, DWORD iPitchAndFamily, LPCSTR pszFaceName);
    static void ApplyConsoleStyle();
    static void GetWindowPos(HWND hWnd, int* x, int* y);
    static void MakeRoomForText(int addedCharacters);
    static float GetDpiScale(const HWND hWnd);

    static void (*Con_UpdateNotifyLine_Original)(int localClientNum, int channel, bool lineFeed, int flags);

    static void Con_UpdateNotifyLine_Hk(int localClientNum, int channel, bool lineFeed, int flags);
    static void Con_OneTimeInit_Hk();
    static void Con_Init_Hk();
    static void Con_InitChannels_Hk();
    static void SCR_DrawScreenField_Hk();
    static int R_TextHeight_Hk(Game::Font_s* font);
    static int R_TextWidth_Hk(const char* text, int maxChars, Game::Font_s* font);
    static void CL_DrawTextWithCursor_Hk(Game::ScreenPlacement* scrPlace, const char* text, int maxChars, Game::Font_s* font, float x, float y, int horzAlign, int vertAlign, float xScale, float yScale, const float* color, int style, int cursorPos, char cursorChar);
    static Game::dvar_t* RegisterConColor(const char* dvarName, float r, float g, float b, float a, float min, float max, unsigned __int16 flags, const char* description);
  };
}
