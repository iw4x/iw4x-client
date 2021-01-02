#include "STDInclude.hpp"

namespace Components
{
	XINPUT_STATE XInput::xiStates[XUSER_MAX_COUNT];

	void XInput::PollXInputDevices()
	{
		for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
		{
			XInputGetState(i, &xiStates[i]);
		}
	}

	__declspec(naked) void XInput::CL_FrameStub()
	{
		__asm
		{
			// poll the xinput devices on every client frame
			pusha
			pushad

			call XInput::PollXInputDevices

			popad
			popa

			// execute the code we patched over
			sub     esp, 0Ch
			push    ebx
			push    ebp
			push    esi

			// return back to original code
			push 0x486976
			retn
		}
	}

	XInput::XInput()
	{
		Utils::Hook(0x486970, XInput::CL_FrameStub, HOOK_JUMP).install()->quick();
	}
}
