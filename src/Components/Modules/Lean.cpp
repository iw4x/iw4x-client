#include <STDInclude.hpp>

#include "Events.hpp"
#include "Lean.hpp"

namespace Components
{
	Dvar::Var Lean::BGLean;

	Game::kbutton_t Lean::in_leanleft;
	Game::kbutton_t Lean::in_leanright;

	void Lean::IN_LeanLeft_Up()
	{
		Game::IN_KeyUp(&in_leanleft);
	}

	void Lean::IN_LeanLeft_Down()
	{
		Game::IN_KeyDown(&in_leanleft);
	}

	void Lean::IN_LeanRight_Up()
	{
		Game::IN_KeyUp(&in_leanright);
	}

	void Lean::IN_LeanRight_Down()
	{
		Game::IN_KeyDown(&in_leanright);
	}

	void Lean::ApplyLeanFlags(Game::usercmd_s* cmd)
	{
		if ((in_leanleft.active || in_leanleft.wasPressed) && BGLean.get<bool>())
		{
			cmd->buttons |= Game::CMD_BUTTON_LEAN_LEFT;
		}

		if ((in_leanright.active || in_leanright.wasPressed) && BGLean.get<bool>())
		{
			cmd->buttons |= Game::CMD_BUTTON_LEAN_RIGHT;
		}

		in_leanleft.wasPressed = false;
		in_leanright.wasPressed = false;
	}

	void Lean::PM_UpdateLean_Stub(Game::playerState_s* ps, float msec, Game::usercmd_s* cmd, void(*capsuleTrace)(Game::trace_t*, const float*, const float*, const Game::Bounds*, int, int))
	{
		if (BGLean.get<bool>())
		{
			Game::PM_UpdateLean(ps, msec, cmd, capsuleTrace);
		}
	}

	Lean::Lean()
	{
		Command::AddRaw("+leanleft", IN_LeanLeft_Down, true);
		Command::AddRaw("-leanleft", IN_LeanLeft_Up, true);

		Command::AddRaw("+leanright", IN_LeanRight_Down, true);
		Command::AddRaw("-leanright", IN_LeanRight_Up, true);

		Events::OnClientCmdButtons(ApplyLeanFlags);

		Utils::Hook(0x4A0C72, PM_UpdateLean_Stub, HOOK_CALL).install()->quick();
		Utils::Hook(0x4A0D72, PM_UpdateLean_Stub, HOOK_CALL).install()->quick();

		BGLean = Dvar::Register<bool>("bg_lean", true, Game::DVAR_CODINFO, "Enable CoD4 leaning");
	}
}
