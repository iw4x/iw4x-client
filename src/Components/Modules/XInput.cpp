#include "STDInclude.hpp"

namespace Components
{
	XINPUT_STATE XInput::xiStates[XUSER_MAX_COUNT];
	XINPUT_STATE XInput::lastXiState = { 0 };
	int XInput::xiPlayerNum = -1;
	std::chrono::milliseconds XInput::timeAtFirstHeldMaxLookX = 0ms; // "For how much time in miliseconds has the player been holding a horizontal direction on their stick, fully" (-1.0 or 1.0)
	bool XInput::isHoldingMaxLookX = false;

	float XInput::lockedSensitivityMultiplier = 0.6f;
	float XInput::unlockedSensitivityMultiplier = 1.2f;
	float XInput::generalSensitivityMultiplier = 1.3f;

	std::chrono::milliseconds XInput::msBeforeUnlockingSensitivity = 250ms;

	std::vector<XInput::ActionMapping> mappings = {
		XInput::ActionMapping(XINPUT_GAMEPAD_A, "gostand"),
		//XInput::ActionMapping(XINPUT_GAMEPAD_B, "stance", true, true),
		XInput::ActionMapping(XINPUT_GAMEPAD_X, "usereload"),
		XInput::ActionMapping(XINPUT_GAMEPAD_Y, "weapnext", false),
		XInput::ActionMapping(XINPUT_GAMEPAD_LEFT_SHOULDER, "smoke"),
		XInput::ActionMapping(XINPUT_GAMEPAD_RIGHT_SHOULDER, "frag"),
		XInput::ActionMapping(XINPUT_GAMEPAD_LEFT_THUMB,  "breath_sprint"),
		XInput::ActionMapping(XINPUT_GAMEPAD_RIGHT_THUMB, "melee"),
		XInput::ActionMapping(XINPUT_GAMEPAD_START, "togglemenu", false),
		XInput::ActionMapping(XINPUT_GAMEPAD_BACK, "scores"),
		XInput::ActionMapping(XINPUT_GAMEPAD_DPAD_RIGHT, "actionslot 3"),
		XInput::ActionMapping(XINPUT_GAMEPAD_DPAD_LEFT, "actionslot 2"),
		XInput::ActionMapping(XINPUT_GAMEPAD_DPAD_UP, "actionslot 1"),
		XInput::ActionMapping(XINPUT_GAMEPAD_DPAD_DOWN, "actionslot 4"),
	};


	void XInput::Vibrate(int leftVal, int rightVal)
	{
		// Create a Vibraton State
		XINPUT_VIBRATION Vibration;

		// Zeroise the Vibration
		ZeroMemory(&Vibration, sizeof(XINPUT_VIBRATION));

		// Set the Vibration Values
		Vibration.wLeftMotorSpeed = leftVal;
		Vibration.wRightMotorSpeed = rightVal;

		// Vibrate the controller
		XInputSetState(xiPlayerNum, &Vibration);
	}


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

			// Deadzones
			float moveStickX = abs(xiState->Gamepad.sThumbLX) > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ? xiState->Gamepad.sThumbLX / (float)std::numeric_limits<SHORT>().max() : .0f;
			float moveStickY = abs(xiState->Gamepad.sThumbLY) > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ? xiState->Gamepad.sThumbLY / (float)std::numeric_limits<SHORT>().max() : .0f;

			float viewStickX = abs(xiState->Gamepad.sThumbRX) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ? xiState->Gamepad.sThumbRX / (float)std::numeric_limits<SHORT>().max() : .0f;
			float viewStickY = abs(xiState->Gamepad.sThumbRY) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ? xiState->Gamepad.sThumbRY / (float)std::numeric_limits<SHORT>().max() : .0f;

			cmd->rightmove = moveStickX * std::numeric_limits<char>().max();
			cmd->forwardmove = moveStickY * std::numeric_limits<char>().max();

			// Gamepad horizontal acceleration on view
			if (abs(viewStickX) > 0.9f) {
				if (!XInput::isHoldingMaxLookX) {
					XInput::isHoldingMaxLookX = true;
					XInput::timeAtFirstHeldMaxLookX = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
				}
				else {
					std::chrono::milliseconds hasBeenHoldingLeftXForMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) - XInput::timeAtFirstHeldMaxLookX;
#ifdef STEP_SENSITIVITY
					if (hasBeenHoldingLeftXForMs < XInput::msBeforeUnlockingSensitivity) {
						viewStickX *= XInput::lockedSensitivityMultiplier;
					}
					else {
						viewStickX *= XInput::unlockedSensitivityMultiplier;
					}
#else
					float coeff = std::clamp(hasBeenHoldingLeftXForMs.count()/(float)XInput::msBeforeUnlockingSensitivity.count(), 0.0F, 1.0F);
					viewStickX *= std::lerp(XInput::lockedSensitivityMultiplier, XInput::unlockedSensitivityMultiplier, coeff);
#endif
				}
			}
			else{
				XInput::isHoldingMaxLookX = false;
				XInput::timeAtFirstHeldMaxLookX = 0ms;
			}


			Game::cl_angles[0] -= viewStickY;
			Game::cl_angles[1] -= viewStickX * generalSensitivityMultiplier;

			bool pressingLeftTrigger = xiState->Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD ? true : false;
			if (pressingLeftTrigger != XInput::lastXiState.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
			{
				if (pressingLeftTrigger)
					Command::Execute("+speed");
				else
					Command::Execute("-speed");
			}

			bool pressingRightTrigger = xiState->Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD ? true : false;
			if (pressingRightTrigger != XInput::lastXiState.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
			{
				if (pressingRightTrigger)
					Command::Execute("+attack");
				else
					Command::Execute("-attack");
			}

			// Buttons (on/off) mappings
			for (size_t i = 0; i < mappings.size(); i++)
			{
				auto mapping = mappings[i];
				auto action = mapping.action;
				auto antiAction = mapping.action;

				if (mapping.isReversible) {
					action = "+" + mapping.action;
					antiAction = "-" + mapping.action;
				}
				else if (mapping.wasPressed) {
					if (xiState->Gamepad.wButtons & mapping.input) {
						// Button still pressed, do not send info
						if (mapping.spamWhenHeld) {
							Command::Execute(action.c_str());
						}
					}
					else {
						mappings[i].wasPressed = false;
					}

					continue;
				}

				if (xiState->Gamepad.wButtons & mapping.input) {
					Command::Execute(action.c_str());
					mappings[i].wasPressed = true;
				}
				else if (mapping.isReversible && mapping.wasPressed) {
					mappings[i].wasPressed = false;
					Command::Execute(antiAction.c_str());
				}
			}

			bool pressingStance = (xiState->Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0;
			if (pressingStance != ((XInput::lastXiState.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0))
			{
				if (pressingStance)
					Command::Execute("+stance");
				else
					Command::Execute("-stance");
			}


			memcpy(&XInput::lastXiState, xiState, sizeof XINPUT_STATE);
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
			mov   dl, byte ptr[edi + 1Ah] // to_forwardMove
			mov   dh, byte ptr[edi + 1Bh] // to_rightMove

			mov[esp + 30h], dx // to_buttons

			mov   dl, byte ptr[ebp + 1Ah] // from_forwardMove
			mov   dh, byte ptr[ebp + 1Bh] // from_rightMove

			mov[esp + 2Ch], dx // from_buttons

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
