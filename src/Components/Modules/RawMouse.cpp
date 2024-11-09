#include <STDInclude.hpp>
#include "Gamepad.hpp"
#include "RawMouse.hpp"
#include "Window.hpp"

namespace Components
{
	Dvar::Var RawMouse::M_RawInput;
	int RawMouse::MouseRawX = 0;
	int RawMouse::MouseRawY = 0;
	bool RawMouse::InRawInput = false;
	bool RawMouse::RawInputWasEverEnabled = false;
	bool RawMouse::RawInputSupported = false;

	constexpr const int K_MWHEELUP = 205;
	constexpr const int K_MWHEELDOWN = 206;

#define HIGH_POLLING_FIX 1

	void ClampMousePos(POINT& curPos)
	{
		tagRECT rc;
		GetWindowRect(Window::GetWindow(), &rc);
		auto isClamped = false;
		if (curPos.x >= rc.left)
		{
			if (curPos.x >= rc.right)
			{
				curPos.x = rc.right - 1;
				isClamped = true;
			}
		}
		else
		{
			curPos.x = rc.left;
			isClamped = true;
		}

		if (curPos.y >= rc.top)
		{
			if (curPos.y >= rc.bottom)
			{
				curPos.y = rc.bottom - 1;
				isClamped = true;
			}
		}
		else
		{
			curPos.y = rc.top;
			isClamped = true;
		}

		if (isClamped)
		{
			SetCursorPos(curPos.x, curPos.y);
		}
	}

	void RawMouse::IN_ClampMouseMove()
	{
		tagPOINT curPos;
		GetCursorPos(&curPos);
		ClampMousePos(curPos);
	}

	bool CheckButtonFlag(DWORD usButtonFlags, DWORD flag_down)
	{
		DWORD flag_up = (flag_down << 1);
		DWORD flag = (flag_down | flag_up);
		return (usButtonFlags & flag) != 0u;
	}

	BOOL RawMouse::OnRawInput(LPARAM lParam, WPARAM)
	{
		if (!M_RawInput.get<bool>())
			return TRUE;

		auto dwSize = sizeof(RAWINPUT);
		static BYTE lpb[sizeof(RAWINPUT)];

		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

		auto* raw = reinterpret_cast<RAWINPUT*>(lpb);
		if (raw->header.dwType != RIM_TYPEMOUSE)
			return TRUE;
		// Is there's really absolute mouse on earth?
		if (raw->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
		{
			MouseRawX = raw->data.mouse.lLastX;
			MouseRawY = raw->data.mouse.lLastY;
		}
		else
		{
			MouseRawX += raw->data.mouse.lLastX;
			MouseRawY += raw->data.mouse.lLastY;
		}

#if HIGH_POLLING_FIX
		// we need to repeat this check there, since raw input sends attack if game is alt tabbed.
		if (GetForegroundWindow() != Window::GetWindow())
			return TRUE;

		int mouse_event_flag = CheckButtonFlag(raw->data.mouse.usButtonFlags, RI_MOUSE_BUTTON_1_DOWN);
		if (CheckButtonFlag(raw->data.mouse.usButtonFlags, RI_MOUSE_BUTTON_2_DOWN))
			mouse_event_flag |= 2u;
		if (CheckButtonFlag(raw->data.mouse.usButtonFlags, RI_MOUSE_BUTTON_3_DOWN))
			mouse_event_flag |= 4u;
		if (CheckButtonFlag(raw->data.mouse.usButtonFlags, RI_MOUSE_BUTTON_4_DOWN))
			mouse_event_flag |= 8u;
		if (CheckButtonFlag(raw->data.mouse.usButtonFlags, RI_MOUSE_BUTTON_5_DOWN))
			mouse_event_flag |= 0x10u;

		Game::IN_MouseEvent(mouse_event_flag);

		if (raw->data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
		{
			const SHORT scroll_delta = static_cast<SHORT>(raw->data.mouse.usButtonData);

			if (scroll_delta > 0)
				Game::Sys_QueEvents(Game::sysMsgTime(), 1, K_MWHEELDOWN, 0, 0);
			if (scroll_delta < 0)
				Game::Sys_QueEvents(Game::sysMsgTime(), 1, K_MWHEELUP, 0, 0);
		}
#endif

		return TRUE;
	}

	bool RawMouse::IsMouseInClientBounds()
	{
		POINT curPos;
		GetCursorPos(&curPos);
		ScreenToClient(Window::GetWindow(), &curPos);

		RECT rect;
		Window::Dimension(Window::GetWindow(), &rect);

		return (curPos.y >= 0 && curPos.x >= 0 && (rect.right - rect.left) >= curPos.x && (rect.bottom - rect.top) >= curPos.y);
	}

	BOOL RawMouse::OnKillFocus(LPARAM lParam, WPARAM)
	{
		static auto r_autopriority = Dvar::Var("r_autopriority");
		ToggleRawInput(false);
		if (r_autopriority.get<bool>())
			SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
		SetFocus(nullptr);
		return FALSE;
	}

	BOOL RawMouse::OnSetFocus(LPARAM lParam, WPARAM)
	{
		static auto r_autopriority = Dvar::Var("r_autopriority");
		ToggleRawInput(IsMouseInClientBounds());
		if (r_autopriority.get<bool>())
			SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);

		SetFocus(Window::GetWindow());
		return FALSE;
	}

	void RawMouse::IN_RawMouseMove()
	{
		if (GetForegroundWindow() != Window::GetWindow())
			return;

		static auto oldX = 0, oldY = 0;

		auto dx = MouseRawX - oldX;
		auto dy = MouseRawY - oldY;

		oldX = MouseRawX;
		oldY = MouseRawY;

		// Don't use raw input for menu?
		// Because it needs to call the ScreenToClient
		tagPOINT curPos;
		GetCursorPos(&curPos);
		Game::s_wmv->oldPos = curPos;
		ScreenToClient(Window::GetWindow(), &curPos);

		Gamepad::OnMouseMove(curPos.x, curPos.y, dx, dy);

		auto recenterMouse = Game::CL_MouseEvent(curPos.x, curPos.y, dx, dy);

		ClipCursor(NULL);

		if (recenterMouse)
		{
			RawMouse::IN_RecenterMouse();
		}
	}

	bool RawMouse::ToggleRawInput(bool enable)
	{
		if (!RawInputSupported || !M_RawInput.get<bool>())
		{
			if (!InRawInput)
				return false;

			enable = false;
		}
		else
		{
			if (InRawInput == enable)
				return InRawInput;
		}

#if HIGH_POLLING_FIX
		constexpr DWORD rawinput_flags = RIDEV_INPUTSINK | RIDEV_NOLEGACY;
#else
		constexpr DWORD rawinput_flags = RIDEV_INPUTSINK;
#endif

		RAWINPUTDEVICE Rid[1];
		Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
		Rid[0].dwFlags = (enable ? rawinput_flags : RIDEV_REMOVE);
		Rid[0].hwndTarget = enable ? Window::GetWindow() : NULL;

		bool success = RegisterRawInputDevices(Rid, ARRAYSIZE(Rid), sizeof(Rid[0])) == TRUE;

		if (!success)
			Logger::Warning(Game::CON_CHANNEL_SYSTEM, "RawInputDevices: failed: {}\n", GetLastError());

		if (success && enable) //
			RawInputWasEverEnabled = true;

		InRawInput = success && (Rid[0].dwFlags & RIDEV_REMOVE) == 0;
#ifdef _DEBUG
		if (InRawInput)
			Logger::Debug("Raw Mouse enabled");
		else
			Logger::Debug("Raw Mouse disabled");
#endif
		return true;
	}

	void RawMouse::IN_RawMouse_Init()
	{
		RawInputSupported = false;

		HMODULE user32 = GetModuleHandleA("USER32.dll");
		if (user32)
		{
			PVOID pfnRegisterRawInputDevices = GetProcAddress(user32, "RegisterRawInputDevices");
			PVOID pfnGetRawInputData = GetProcAddress(user32, "GetRawInputData");
		
			if (pfnRegisterRawInputDevices && pfnGetRawInputData)
				RawInputSupported = true;
		}

		if (Window::GetWindow() && ToggleRawInput(true)) {
			Logger::Debug("Raw Mouse Init");
		}
	}

	void RawMouse::IN_Init()
	{
		Game::IN_Init();
		IN_RawMouse_Init();
	}

	void RawMouse::IN_Frame()
	{
		bool focused = GetForegroundWindow() == Window::GetWindow();
		if (focused)
			focused = IsMouseInClientBounds();
		ToggleRawInput(focused);
		return Game::IN_Frame();
	}

	BOOL RawMouse::IN_RecenterMouse()
	{
		RECT clientRect;

		GetClientRect(Window::GetWindow(), &clientRect);

		ClientToScreen(Window::GetWindow(), std::bit_cast<POINT*>(&clientRect.left));
		ClientToScreen(Window::GetWindow(), std::bit_cast<POINT*>(&clientRect.right));

		return ClipCursor(&clientRect);
	}

	void RawMouse::IN_MouseMove()
	{
		static auto r_fullscreen = Dvar::Var("r_fullscreen");

		if (InRawInput)
		{
			return IN_RawMouseMove();
		}

		// IN_MouseMove
		if (GetForegroundWindow() != Window::GetWindow())
			return;

		tagPOINT cursor_pos;
		static tagPOINT prev_cursor;

		GetCursorPos(&cursor_pos);
		if (r_fullscreen.get<bool>())
			ClampMousePos(cursor_pos);

		int delta_x = cursor_pos.x - prev_cursor.x;
		int delta_y = cursor_pos.y - prev_cursor.y;
		prev_cursor = cursor_pos;

		ScreenToClient(Window::GetWindow(), &cursor_pos);
		auto recenterMouse = Game::CL_MouseEvent(cursor_pos.x, cursor_pos.y, delta_x, delta_y);

		if (recenterMouse && (delta_x || delta_y))
		{
			RECT Rect;
			GetWindowRect(Window::GetWindow(), &Rect);

			int window_center_x = (Rect.right + Rect.left) / 2;
			int window_center_y = (Rect.top + Rect.bottom) / 2;
			SetCursorPos(window_center_x, window_center_y);

			prev_cursor.x = window_center_x;
			prev_cursor.y = window_center_y;
		}
	}

	RawMouse::RawMouse()
	{
		Utils::Hook(0x475E65, IN_MouseMove, HOOK_JUMP).install()->quick();
		Utils::Hook(0x475E8D, IN_MouseMove, HOOK_JUMP).install()->quick();
		Utils::Hook(0x475E9E, IN_MouseMove, HOOK_JUMP).install()->quick();

		Utils::Hook(0x467C03, IN_Init, HOOK_CALL).install()->quick();
		Utils::Hook(0x64D095, IN_Init, HOOK_JUMP).install()->quick();

#if HIGH_POLLING_FIX
		Utils::Hook(0x60BFB9, IN_Frame, HOOK_CALL).install()->quick();
		Utils::Hook(0x4A87E2, IN_Frame, HOOK_CALL).install()->quick();
		Utils::Hook(0x48A0E6, IN_Frame, HOOK_CALL).install()->quick();
#endif

		Utils::Hook(0x473517, IN_RecenterMouse, HOOK_CALL).install()->quick();
		Utils::Hook(0x64C520, IN_RecenterMouse, HOOK_CALL).install()->quick();

		M_RawInput = Dvar::Register<bool>("m_rawinput", true, Game::DVAR_ARCHIVE, "Use raw mouse input, Improves accuracy & has better support for higher polling rates");

#if HIGH_POLLING_FIX
		Window::OnWndMessage(WM_KILLFOCUS, OnKillFocus);
		Window::OnWndMessage(WM_SETFOCUS, OnSetFocus);
#endif

		Window::OnWndMessage(WM_INPUT, OnRawInput);
		Window::OnCreate(IN_RawMouse_Init);
	}
}
