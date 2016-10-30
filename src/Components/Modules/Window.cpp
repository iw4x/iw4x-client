#include "STDInclude.hpp"

namespace Components
{
	Dvar::Var Window::NoBorder;
	Dvar::Var Window::NativeCursor;

	HWND Window::MainWindow = 0;
	BOOL Window::CursorVisible = TRUE;

	int Window::Width()
	{
		return Window::Width(Window::MainWindow);
	}

	int Window::Height()
	{
		return Window::Height(Window::MainWindow);
	}

	int Window::Width(HWND window)
	{
		RECT rect;
		Window::Dimension(window, &rect);
		return (rect.right - rect.left);
	}

	int Window::Height(HWND window)
	{
		RECT rect;
		Window::Dimension(window, &rect);
		return (rect.bottom - rect.top);
	}

	void Window::Dimension(RECT* rect)
	{
		Window::Dimension(Window::MainWindow, rect);
	}

	void Window::Dimension(HWND window, RECT* rect)
	{
		if (rect)
		{
			ZeroMemory(rect, sizeof(RECT));

			if (window && IsWindow(window))
			{
				GetWindowRect(window, rect);
			}
		}
	}

	bool Window::IsCursorWithin(HWND window)
	{
		RECT rect;
		POINT point;
		Window::Dimension(window, &rect);

		GetCursorPos(&point);

		return ((point.x - rect.left) > 0 && (point.y - rect.top) > 0 && (rect.right - point.x) > 0 && (rect.bottom - point.y) > 0);
	}

	int Window::IsNoBorder()
	{
		return Window::NoBorder.Get<bool>();
	}

	__declspec(naked) void Window::StyleHookStub()
	{
		__asm
		{
			call Window::IsNoBorder
			test al, al
			jz setBorder

			mov ebp, WS_VISIBLE | WS_POPUP
			retn
			
		setBorder:
			mov ebp, WS_VISIBLE | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX
			retn
		}
	}

	void Window::DrawCursorStub(void *scrPlace, float x, float y, float w, float h, int horzAlign, int vertAlign, const float *color, Game::Material *material)
	{
		if (Window::NativeCursor.Get<bool>())
		{
			Window::CursorVisible = TRUE;
		}
		else
		{
			Game::UI_DrawHandlePic(scrPlace, x, y, w, h, horzAlign, vertAlign, color, material);
		}
	}

	int WINAPI Window::ShowCursorHook(BOOL show)
	{
		if (Window::NativeCursor.Get<bool>() && IsWindow(Window::MainWindow) && GetForegroundWindow() == Window::MainWindow && Window::IsCursorWithin(Window::MainWindow))
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
		Window::NativeCursor = Dvar::Register<bool>("ui_nativeCursor", false, Game::dvar_flag::DVAR_FLAG_SAVED, "Display native cursor");

		Utils::Hook(0x507643, Window::StyleHookStub, HOOK_CALL).Install()->Quick();

		// Main window creation
		Utils::Hook::Nop(0x5076AA, 1);
		Utils::Hook(0x5076AB, Window::CreateMainWindow, HOOK_CALL).Install()->Quick();

		// Mark the cursor as visible
		Utils::Hook(0x48E5D3, Window::DrawCursorStub, HOOK_CALL).Install()->Quick();

		// Draw the cursor if necessary
		Renderer::OnFrame([] ()
		{
			if (Window::NativeCursor.Get<bool>() && IsWindow(Window::MainWindow) && GetForegroundWindow() == Window::MainWindow && Window::IsCursorWithin(Window::MainWindow))
			{
				int value = 0;

				if (Window::CursorVisible)
				{
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
