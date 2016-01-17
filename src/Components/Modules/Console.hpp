#define OUTPUT_HEIGHT 250
#define OUTPUT_MAX_TOP (OUTPUT_HEIGHT - (Console::Height - 2))

namespace Components
{
	class Console : public Component
	{
	public:
		Console();
		const char* GetName() { return "Console"; };

	private:
		static void ToggleConsole();
		static char** GetAutoCompleteFileList(const char *path, const char *extension, Game::FsListBehavior_e behavior, int *numfiles, int allocTrackType);

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

		static void ShowPrompt();
		static void RefreshStatus();
		static void RefreshOutput();
		static void ScrollOutput(int amount);

		static const char* Input();
		static void Print(const char* message);
		static void Error(const char* format, ...);
		static void Create();
		static void Destroy();
	};
}
