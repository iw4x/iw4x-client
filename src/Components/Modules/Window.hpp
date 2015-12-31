namespace Components
{
	class Window : public Component
	{
	public:
		Window();
		const char* GetName() { return "Window"; };

		static Dvar::Var NoBorder;
		static Dvar::Var NativeCursor;
		static void Window::StyleHookStub();

	private:
		static BOOL CursorVisible;

		static int WINAPI ShowCursorHook(BOOL show);
		static void DrawCursorStub(void *scrPlace, float x, float y, float w, float h, int horzAlign, int vertAlign, const float *color, Game::Material *material);

		static HWND MainWindow;

		static HWND WINAPI CreateMainWindow(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
	};
}
