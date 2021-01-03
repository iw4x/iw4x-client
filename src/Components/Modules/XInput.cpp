#include "STDInclude.hpp"

namespace Components
{
	XINPUT_STATE XInput::xiStates[XUSER_MAX_COUNT];
	XINPUT_STATE XInput::lastxiState = { 0 };
	int XInput::xiPlayerNum = -1;

	void XInput::PollXInputDevices()
	{
		XInput::xiPlayerNum = -1;

		for (int i = XUSER_MAX_COUNT - 1; i >= 0; i--)
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

			Game::cl_angles[0] -= (xiState->Gamepad.sThumbRY / 32767.0f);
			Game::cl_angles[1] -= (xiState->Gamepad.sThumbRX / 32767.0f);

			bool pressingLeftTrigger = xiState->Gamepad.bLeftTrigger / 255.f > 0.5;
			if (pressingLeftTrigger != XInput::lastxiState.Gamepad.bLeftTrigger / 255.f > 0.5)
			{
				if (pressingLeftTrigger)
					Command::Execute("+speed");
				else
					Command::Execute("-speed");
			}

			bool pressingRightTrigger = xiState->Gamepad.bRightTrigger / 255.f > 0.5;
			if (pressingRightTrigger != XInput::lastxiState.Gamepad.bRightTrigger / 255.f > 0.5)
			{
				if (pressingRightTrigger)
					Command::Execute("+attack");
				else
					Command::Execute("-attack");
			}

			bool pressingWeapChange = (xiState->Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0;
			if (pressingWeapChange != ((XInput::lastxiState.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0))
			{
				if (pressingWeapChange)
					Command::Execute("weapnext");
			}

			bool pressingReload = (xiState->Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0;
			if (pressingReload != ((XInput::lastxiState.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0))
			{
				if (pressingReload)
					Command::Execute("+usereload");
				else
					Command::Execute("-usereload");
			}

			bool pressingJump = (xiState->Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;
			if (pressingJump != ((XInput::lastxiState.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0))
			{
				if (pressingJump)
					Command::Execute("+gostand");
				else
					Command::Execute("-gostand");
			}

			bool pressingKnife = (xiState->Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0;
			if (pressingKnife != ((XInput::lastxiState.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0))
			{
				if (pressingKnife)
					Command::Execute("+melee");
				else
					Command::Execute("-melee");
			}

			bool pressingSprint = (xiState->Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0;
			if (pressingSprint != ((XInput::lastxiState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0))
			{
				if (pressingSprint)
					Command::Execute("+breath_sprint");
				else
					Command::Execute("-breath_sprint");
			}

			bool pressingStance = (xiState->Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0;
			if (pressingStance != ((XInput::lastxiState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0))
			{
				if (pressingStance)
					Command::Execute("+stance");
				else
					Command::Execute("-stance");
			}

			bool pressingSmoke = (xiState->Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
			if (pressingSmoke != ((XInput::lastxiState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0))
			{
				if (pressingSmoke)
					Command::Execute("+smoke");
				else
					Command::Execute("-smoke");
			}

			bool pressingFrag = (xiState->Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
			if (pressingFrag != ((XInput::lastxiState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0))
			{
				if (pressingFrag)
					Command::Execute("+frag");
				else
					Command::Execute("-frag");
			}

			bool pressingScore = (xiState->Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0;
			if (pressingScore != ((XInput::lastxiState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0))
			{
				if (pressingScore)
					Command::Execute("+scores");
				else
					Command::Execute("-scores");
			}

			bool pressingAlt = (xiState->Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0;
			if (pressingAlt != ((XInput::lastxiState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0))
			{
				if (pressingAlt)
					Command::Execute("+actionslot 2");
				else
					Command::Execute("-actionslot 2");
			}

			bool pressingKillstreak = (xiState->Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;
			if (pressingKillstreak != ((XInput::lastxiState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0))
			{
				if (pressingKillstreak)
					Command::Execute("+actionslot 3");
				else
					Command::Execute("-actionslot 3");
			}

			bool pressingNight = (xiState->Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
			if (pressingNight != ((XInput::lastxiState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0))
			{
				if (pressingNight)
					Command::Execute("+actionslot 4");
				else
					Command::Execute("-actionslot 4");
			}

			bool pressingUp = (xiState->Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0;
			if (pressingUp != ((XInput::lastxiState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0))
			{
				if (pressingUp)
					Command::Execute("+actionslot 1");
				else
					Command::Execute("-actionslot 1");
			}

			bool pressingStart = (xiState->Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0;
			if (pressingStart != ((XInput::lastxiState.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0))
			{
				if (pressingStart)
					Command::Execute("togglemenu");
			}


			memcpy(&XInput::lastxiState, xiState, sizeof XINPUT_STATE);
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

	void XInput::ApplyMovement(Game::msg_t* msg, int key, Game::usercmd_s* from, Game::usercmd_s* to)
	{
		char forward;
		char right;

		if (Game::MSG_ReadBit(msg))
		{
			short movementBits = static_cast<short>(key ^ Game::MSG_ReadBits(msg, 16));

			forward = static_cast<char>(movementBits);
			right = static_cast<char>(movementBits >> 8);
		}
		else
		{
			forward = from->forwardmove;
			right = from->rightmove;
		}
		
		to->forwardmove = forward;
		to->rightmove = right;
	}

	__declspec(naked) void XInput::MSG_ReadDeltaUsercmdKeyStub()
	{
		__asm
		{
			push ebx // to
			push ebp // from
			push edi // key
			push esi // msg
			call XInput::ApplyMovement
			add     esp, 10h

			// return back
			push 0x4921BF
			ret
		}
	}

	__declspec(naked) void XInput::MSG_ReadDeltaUsercmdKeyStub2()
	{
		__asm
		{
			push ebx // to
			push ebp // from
			push edi // key
			push esi // msg
			call XInput::ApplyMovement
			add     esp, 10h

			// return back
			push 3
			push esi
			push 0x492085
			ret
		}
	}

	XInput::XInput()
	{
		// poll xinput devices every client frame
		Utils::Hook(0x486970, XInput::CL_FrameStub, HOOK_JUMP).install()->quick();

		// use the xinput state when creating a usercmd
		Utils::Hook(0x5A6DB9, XInput::CL_CreateCmdStub, HOOK_JUMP).install()->quick();

		// package the forward and right move components in the move buttons
		Utils::Hook(0x60E38D, XInput::MSG_WriteDeltaUsercmdKeyStub, HOOK_JUMP).install()->quick();

		// send two bytes for sending movement data
		Utils::Hook::Set<BYTE>(0x60E501, 16);
		Utils::Hook::Set<BYTE>(0x60E5CD, 16);

		// make sure to parse the movement data properally and apply it
		Utils::Hook(0x492127, XInput::MSG_ReadDeltaUsercmdKeyStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x492009, XInput::MSG_ReadDeltaUsercmdKeyStub2, HOOK_JUMP).install()->quick();
	}
}
