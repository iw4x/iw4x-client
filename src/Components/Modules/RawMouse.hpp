#pragma once

namespace Components
{
	class RawMouse : public Component
	{
	public:
		RawMouse();
	private:
		static Dvar::Var m_rawinput;
		static int mouseRawX, mouseRawY;

		static void IN_ClampMouseMove();
		static BOOL OnRawInput(LPARAM lParam, WPARAM);
		static void IN_RawMouseMove();
		static void IN_RawMouse_Init();
		static void IN_Init();
		static void IN_MouseMove();
	};
}
