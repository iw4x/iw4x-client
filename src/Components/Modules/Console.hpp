#pragma once

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

		static std::thread ConsoleThread;

		static Game::SafeArea OriginalSafeArea;

		static bool isCommand;

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

		static void DrawSolidConsoleStub();
		static void StoreSafeArea();
		static void RestoreSafeArea();

		static const char** GetAutoCompleteFileList(const char *path, const char *extension, Game::FsListBehavior_e behavior, int *numfiles, int allocTrackType);

		static void Con_ToggleConsole();
		static void AddConsoleCommand();

		static Game::dvar_t* RegisterConColor(const char* dvarName, float r, float g, float b, float a, float min, float max, unsigned __int16 flags, const char* description);

		static bool Con_IsDvarCommand_Stub(const char* cmd);
		static void Cmd_ForEach_Stub(void(*callback)(const char* str));
	
		static LRESULT CALLBACK ConWndProc(HWND hWnd, UINT Msg, WPARAM wParam, unsigned int lParam);
		static ATOM CALLBACK RegisterClassHook(WNDCLASSA* lpWndClass);
		static BOOL CALLBACK ResizeChildWindow(HWND hwndChild, LPARAM lParam);
		static HFONT CALLBACK ReplaceFont(int cHeight, int cWidth, int cEscapement, int cOrientation, int cWeight, DWORD bItalic, DWORD bUnderline, DWORD bStrikeOut, DWORD iCharSet, DWORD iOutPrecision, DWORD iClipPrecision, DWORD iQuality, DWORD iPitchAndFamily, LPCSTR pszFaceName);
		static void ApplyConsoleStyle();
		static void GetWindowPos(HWND hWnd, int* x, int* y);
		static void Sys_PrintStub();
		static void MakeRoomForText(int addedCharacters);
		static float GetDpiScale(const HWND hWnd);
	};
}
