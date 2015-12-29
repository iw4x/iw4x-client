#include "..\..\STDInclude.hpp"

namespace Components
{
	Dvar::Var Window::NoBorder;

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

	Window::Window()
	{
		// Borderless window
		Window::NoBorder = Dvar::Register<bool>("r_noborder", true, Game::dvar_flag::DVAR_FLAG_SAVED, "Do not use a border in windowed mode");
		Utils::Hook(0x507643, Window::StyleHookStub, HOOK_CALL).Install()->Quick();
	}
}
