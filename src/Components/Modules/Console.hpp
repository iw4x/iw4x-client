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
		// Text-based console stuff
		static WINDOW* OutputWindow;
		static WINDOW* InputWindow;
		static WINDOW* InfoWindow;

		static int Width;
		static int Height;

		static int OutputTop;
		static int OutBuffer;
		static int LastRefresh;

		static char LineBuffer[1024];
		static char LineBuffer2[1024];
		static int LineBufferIndex;

		static bool HasConsole;
		static bool SkipShutdown;

		static std::thread ConsoleThread;

		static Game::SafeArea OriginalSafeArea;

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

		static void ToggleConsole();
		static char** GetAutoCompleteFileList(const char *path, const char *extension, Game::FsListBehavior_e behavior, int *numfiles, int allocTrackType);

		static Game::dvar_t* RegisterConColor(const char* dvarName, float r, float g, float b, float a, float min, float max, unsigned __int16 flags, const char* description);
	};
}
