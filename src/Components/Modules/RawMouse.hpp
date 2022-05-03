#pragma once

namespace Components
{
	class RawMouse : public Component
	{
	public:
		RawMouse();

		static void IN_MouseMove();

	private:
		static Dvar::Var M_RawInput;
		static int MouseRawX, MouseRawY;

		static void IN_ClampMouseMove();
		static BOOL OnRawInput(LPARAM lParam, WPARAM);
		static void IN_RawMouseMove();
		static void IN_RawMouse_Init();
		static void IN_Init();
	};
}
