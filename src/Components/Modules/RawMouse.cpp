#include "RawMouse.hpp"
#include "Gamepad.hpp"
#include "Window.hpp"
#include "Logger.hpp"

namespace Components
{
	Dvar::Var RawMouse::M_RawInput;
	int RawMouse::MouseRawX = 0;
	int RawMouse::MouseRawY = 0;

	void RawMouse::IN_ClampMouseMove()
	{
		tagRECT rc;
		tagPOINT curPos;

		GetCursorPos(&curPos);
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

	LRESULT RawMouse::OnRawInput(LPARAM lParam, WPARAM)
	{
		// The LPARAM contains a handle to a RAWINPUT structure, which we access in
		// two stages:
		//
		//   1. First, we query the size of the input data via GetRawInputData()
		//      with a null buffer. This allows us to allocate an appropriately
		//      sized buffer for the actual data payload.
		//
		//   2. Then, we call GetRawInputData() again to retrieve the full RAWINPUT
		//      structure into that buffer.
		//
		// This two-step process is necessary because the size of the input data may
		// vary depending on device type and driver configuration, and cannot be
		// safely predicted in advance. This behavior is documented in the WinAPI,
		// and we have also encountered it in practice; see:
		//
		//   https://github.com/iw4x/iw4x-client/pull/297
		//
		// There, failing to query the size beforehand led to buffer overruns or
		// data truncation on certain hardware configurations.

		UINT dwSize = 0;

		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER)) == (UINT)-1)
		{
			Logger::Warning(Game::CON_CHANNEL_SYSTEM, "WinAPI: GetRawInputData failed: {}\n", GetLastError());
			return 0;
		}

		// Note that a zero-size payload is technically valid (though unexpected)
		// and implies there is no data to process.
		//
		if (dwSize == 0)
			return 0;

		std::vector<BYTE> buffer(dwSize);

		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer.data(), &dwSize, sizeof(RAWINPUTHEADER)) == (UINT)-1)
		{
			Logger::Warning(Game::CON_CHANNEL_SYSTEM, "WinAPI: GetRawInputData failed: {}\n", GetLastError());
			return 0;
		}

		RAWINPUT* raw = reinterpret_cast<RAWINPUT *>(buffer.data());

		if (raw->header.dwType == RIM_TYPEMOUSE)
		{
			const auto& mouse = raw->data.mouse;

			if (mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
			{
				MouseRawX = mouse.lLastX;
				MouseRawY = mouse.lLastY;
			}
			else
			{
				MouseRawX += mouse.lLastX;
				MouseRawY += mouse.lLastY;
			}
		}

		return 0;
	}

	void RawMouse::IN_RawMouseMove()
	{
		static auto r_fullscreen = Dvar::Var("r_fullscreen");

		if (GetForegroundWindow() == Window::GetWindow())
		{
			if (r_fullscreen.get<bool>())
			{
				IN_ClampMouseMove();
			}

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

			if (recenterMouse)
			{
				Game::IN_RecenterMouse();
			}
		}
	}

	void RawMouse::IN_RawMouse_Init()
	{
		if (Window::GetWindow() && M_RawInput.get<bool>() && !Gamepad::IsGamePadInUse())
		{
			Logger::Debug("Raw Mouse Init");

			RAWINPUTDEVICE Rid[1];
			Rid[0].usUsagePage = 0x01; // HID_USAGE_PAGE_GENERIC
			Rid[0].usUsage = 0x02; // HID_USAGE_GENERIC_MOUSE
			Rid[0].dwFlags = RIDEV_INPUTSINK;
			Rid[0].hwndTarget = Window::GetWindow();

			RegisterRawInputDevices(Rid, ARRAYSIZE(Rid), sizeof(Rid[0]));
		}
	}

	void RawMouse::IN_Init()
	{
		Game::IN_Init();
		IN_RawMouse_Init();
	}

	void RawMouse::IN_MouseMove()
	{
		if (M_RawInput.get<bool>() && !Gamepad::IsGamePadInUse())
		{
			IN_RawMouseMove();
		}
		else
		{
			Game::IN_MouseMove();
		}
	}

	RawMouse::RawMouse()
	{
		Utils::Hook(0x475E65, IN_MouseMove, HOOK_JUMP).install()->quick();
		Utils::Hook(0x475E8D, IN_MouseMove, HOOK_JUMP).install()->quick();

		Utils::Hook(0x467C03, IN_Init, HOOK_CALL).install()->quick();
		Utils::Hook(0x64D095, IN_Init, HOOK_JUMP).install()->quick();

		M_RawInput = Dvar::Register<bool>("m_rawinput", true, Game::DVAR_ARCHIVE, "Use raw mouse input, Improves accuracy & has better support for higher polling rates");

		Window::OnWndMessage(WM_INPUT, OnRawInput);
		Window::OnCreate(IN_RawMouse_Init);
	}
}
