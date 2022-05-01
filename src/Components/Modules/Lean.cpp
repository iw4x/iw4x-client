#include <STDInclude.hpp>

namespace Components
{
	Game::kbutton_t Lean::in_leanleft;
	Game::kbutton_t Lean::in_leanright;

	void Lean::IN_LeanLeft_Up()
	{
		Game::IN_KeyUp(&Lean::in_leanleft);
	}

	void Lean::IN_LeanLeft_Down()
	{
		Game::IN_KeyDown(&Lean::in_leanleft);
	}

	void Lean::IN_LeanRight_Up()
	{
		Game::IN_KeyUp(&Lean::in_leanright);
	}

	void Lean::IN_LeanRight_Down()
	{
		Game::IN_KeyDown(&Lean::in_leanright);
	}

	void Lean::SetLeanFlags(Game::usercmd_s* cmds)
	{
		if (Lean::in_leanleft.active || Lean::in_leanleft.wasPressed)
		{
			cmds->buttons |= BUTTON_FLAG_LEANLEFT;
		}

		if (Lean::in_leanright.active || Lean::in_leanright.wasPressed)
		{
			cmds->buttons |= BUTTON_FLAG_LEANRIGHT;
		}

		Lean::in_leanleft.wasPressed = false;
		Lean::in_leanright.wasPressed = false;
	}

	void __declspec(naked) Lean::CL_CmdButtonsStub()
	{
		__asm
		{
			// CL_CmdButtons
			mov ecx, 5A6510h
			call ecx

			pushad
			push esi
			call Lean::SetLeanFlags
			pop esi
			popad
			retn
		}
	}

	Lean::Lean()
	{
		Command::AddRaw("+leanleft", Lean::IN_LeanLeft_Down, true);
		Command::AddRaw("-leanleft", Lean::IN_LeanLeft_Up, true);

		Command::AddRaw("+leanright", Lean::IN_LeanRight_Down, true);
		Command::AddRaw("-leanright", Lean::IN_LeanRight_Up, true);

		Utils::Hook(0x5A6D84, Lean::CL_CmdButtonsStub, HOOK_CALL).install()->quick();
	}
}
