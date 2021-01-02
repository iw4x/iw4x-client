#include "STDInclude.hpp"

namespace Components
{
	XINPUT_STATE XInput::xiStates[XUSER_MAX_COUNT];
	int XInput::xiPlayerNum = -1;

	void XInput::PollXInputDevices()
	{
		XInput::xiPlayerNum = -1;

		for (int i = XUSER_MAX_COUNT; i >= 0; i--)
		{
			if (XInputGetState(i, &xiStates[i]) == ERROR_SUCCESS)
				XInput::xiPlayerNum = i;
		}
	}

	__declspec(naked) void XInput::CL_FrameStub()
	{
		__asm
		{
			// poll the xinput devices on every client frame
			call XInput::PollXInputDevices

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

	void XInput::CL_GamepadMove(int localClientNum, Game::usercmd_s* cmd)
	{
		if (XInput::xiPlayerNum != -1)
		{
			XINPUT_STATE* xiState = &xiStates[xiPlayerNum];

			cmd->rightmove = xiState->Gamepad.sThumbLX / 256;
			cmd->forwardmove = xiState->Gamepad.sThumbLY / 256;
		}
	}

	__declspec(naked) void XInput::CL_CreateCmdStub()
	{
		__asm
		{
			// do xinput!
			push esi
			push ebp
			call XInput::CL_GamepadMove
			add     esp, 8h

			// execute code we patched over
			add     esp, 4
			fld     st
			pop     ebx

			// return back
			push 0x5A6DBF
			retn
		}
	}

	XInput::XInput()
	{
		Utils::Hook(0x486970, XInput::CL_FrameStub, HOOK_JUMP).install()->quick();

		Utils::Hook(0x5A6DB9, XInput::CL_CreateCmdStub, HOOK_JUMP).install()->quick();
	}
}
