#include "..\..\STDInclude.hpp"

namespace Components
{
	Dvar::Var Window::NoBorder;
	HWND Window::MainWindow = 0;
	BOOL Window::CursorVisible = TRUE;

	void __declspec(naked) Window::StyleHookStub()
	{
		if (Window::NoBorder.Get<bool>())
		{
			__asm mov ebp, WS_VISIBLE | WS_POPUP
		}
		else
		{
			__asm mov ebp, WS_VISIBLE | WS_SYSMENU | WS_CAPTION
		}

		__asm retn
	}

	void Window::DrawCursorStub()
	{
		Window::CursorVisible = TRUE;
	}

	int WINAPI Window::ShowCursorHook(BOOL show)
	{
		if (GetForegroundWindow() == Window::MainWindow)
		{
			static int count = 0;
			(show ? ++count : --count);

			if (count >= 0)
			{
				Window::CursorVisible = TRUE;
			}

			return count;
		}

		return ShowCursor(show);
	}

	HWND WINAPI Window::CreateMainWindow(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
	{
		Window::MainWindow = CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		return Window::MainWindow;
	}

	Window::Window()
	{
		// Borderless window
		Window::NoBorder = Dvar::Register<bool>("r_noborder", true, Game::dvar_flag::DVAR_FLAG_SAVED, "Do not use a border in windowed mode");
		Utils::Hook(0x507643, Window::StyleHookStub, HOOK_CALL).Install()->Quick();

		// Main window creation
		Utils::Hook::Nop(0x5076AA, 1);
		Utils::Hook(0x5076AB, Window::CreateMainWindow, HOOK_CALL).Install()->Quick();

		// Mark the cursor as visible
		Utils::Hook(0x48E5D3, Window::DrawCursorStub, HOOK_CALL).Install()->Quick();

		// Draw the cursor if necessary
		Renderer::OnFrame([] ()
		{
			if (GetForegroundWindow() == Window::MainWindow)
			{
				int value = 0;

				if (Window::CursorVisible)
				{
					// TODO: Apply custom cursor
					SetCursor(LoadCursor(NULL, IDC_ARROW));

					while ((value = ShowCursor(TRUE)) < 0);
					while (value > 0) { value = ShowCursor(FALSE); } // Set display counter to 0
				}
				else
				{
					while ((value = ShowCursor(FALSE)) >= 0);
					while (value < -1) { value = ShowCursor(TRUE); } // Set display counter to -1
				}

				Window::CursorVisible = FALSE;
			}
		});

		// Don't let the game interact with the native cursor
		Utils::Hook::Set(0x6D7348, Window::ShowCursorHook);
	}
}
