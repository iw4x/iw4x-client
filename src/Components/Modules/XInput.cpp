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

	void XInput::CL_GamepadMove(int, Game::usercmd_s* cmd)
	{
		if (XInput::xiPlayerNum != -1)
		{
			XINPUT_STATE* xiState = &xiStates[xiPlayerNum];

			cmd->rightmove = static_cast<BYTE>(xiState->Gamepad.sThumbLX / 256);
			cmd->forwardmove = static_cast<BYTE>(xiState->Gamepad.sThumbLY / 256);
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

	__declspec(naked) void XInput::MSG_WriteDeltaUsercmdKeyStub()
	{
		__asm
		{
			// fix stack pointer
			add esp, 0Ch

			// put both forward move and rightmove values in the movement button
			mov   dl, byte ptr [edi+1Ah] // to_forwardMove
			mov   dh, byte ptr [edi+1Bh] // to_rightMove

			mov     [esp+30h], dx // to_buttons

			mov   dl, byte ptr [ebp+1Ah] // from_forwardMove
			mov   dh, byte ptr [ebp+1Bh] // from_rightMove

			mov     [esp+2Ch], dx // from_buttons
			
			// return back
			push 0x60E40E
			retn
		}
	}

	__declspec(naked) void XInput::MSG_ReadDeltaUsercmdKeyStub()
	{

	}

	XInput::XInput()
	{
		Utils::Hook(0x486970, XInput::CL_FrameStub, HOOK_JUMP).install()->quick();

		Utils::Hook(0x5A6DB9, XInput::CL_CreateCmdStub, HOOK_JUMP).install()->quick();

		// package the forward and right move components in the move buttons
		Utils::Hook(0x60E38D, XInput::MSG_WriteDeltaUsercmdKeyStub, HOOK_JUMP).install()->quick();

		// send two bytes instead of one for sending movement data
		Utils::Hook::Set<BYTE>(0x60E501, 8);
		Utils::Hook::Set<BYTE>(0x60E5CD, 8);

		//Utils::Hook(0x5A6DB9, XInput::MSG_ReadDeltaUsercmdKeyStub, HOOK_JUMP).install()->quick();
	}
}
