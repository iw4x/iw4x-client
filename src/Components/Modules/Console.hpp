#pragma once

extern "C"
{
  void Console_Sys_PrintStub();

  void Console_MakeRoomForText(int addedCharacters);
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
  };
}
