#include "STDInclude.hpp"

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

	Lean::Lean()
	{
		Game::Cmd_AddCommand("+leanleft", Lean::IN_LeanLeft_Down, Command::Allocate(), 1);
		Game::Cmd_AddCommand("-leanleft", Lean::IN_LeanLeft_Up, Command::Allocate(), 1);

		Game::Cmd_AddCommand("+leanright", Lean::IN_LeanRight_Down, Command::Allocate(), 1);
		Game::Cmd_AddCommand("-leanright", Lean::IN_LeanRight_Up, Command::Allocate(), 1);

		// TODO: Transmit correct button flags in CL_CmdButtons and more?
	}
}
