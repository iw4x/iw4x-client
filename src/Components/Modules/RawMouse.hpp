#pragma once

namespace Components
{
	class RawMouse : public Component
	{
	public:
		RawMouse();

		static void IN_MouseMove();

	private:
		static Dvar::Var M_RawInput, R_FullScreen, R_AutoPriority;
		static int MouseRawX, MouseRawY, MouseRawEvents;
		static bool InRawInput, RawInputSupported;

		static void IN_ClampMouseMove();
		static void ResetMouseRawEvents();
		static void ProcessMouseRawEvent(DWORD usButtonFlags, DWORD flag_down, DWORD mouse_event);
		static BOOL OnRawInput(LPARAM lParam, WPARAM);
		static bool IsMouseInClientBounds();
		static BOOL OnKillFocus(LPARAM lParam, WPARAM);
		static BOOL OnSetFocus(LPARAM lParam, WPARAM);
		static void IN_RawMouseMove();
		static bool ToggleRawInput(bool enable = true);
		static void IN_RawMouse_Init();
		static void IN_Init();
		static void IN_Frame();
		static BOOL IN_RecenterMouse();
	};
}
