#include "STDInclude.hpp"

namespace Components
{
	Game::dvar_t* Elevators::SV_EnableEasyElevators;

	__declspec(naked) void Elevators::PM_CorrectAllSolidStub()
	{
		__asm
		{
			push ecx
			mov ecx, Elevators::SV_EnableEasyElevators
			cmp byte ptr [ecx + 16], 1
			pop ecx

			// Always elevate if SV_EnableEasyElevators is set to 1
			je elevate

			// Original code
			cmp byte ptr [eax + 0x29], 0

			// Original code flow
			jz elevate

			// Go to conditional check for the loop
			push 0x5734FF
			retn

		elevate:
			// Continue with loop execution
			push 0x57353D
			retn
		}
	}

	__declspec(naked) void Elevators::PM_CheckDuckStub()
	{
		__asm
		{
			push eax
			mov eax, Elevators::SV_EnableEasyElevators
			cmp byte ptr [eax + 16], 1
			pop eax

			// Always stand if SV_EnableEasyElevators is set to 1
			je stand

			// Original code
			cmp byte ptr [esp + 0x38], 0

			// Original code flow
			jnz noStand

		stand:
			// Player is allowed to stand
			push 0x570ED4
			retn

		noStand:
			// Player will remain ducked
			push 0x570EF4
			retn
		}
	}

	Elevators::Elevators()
	{
		Dvar::OnInit([]
		{
			Elevators::SV_EnableEasyElevators = Game::Dvar_RegisterBool("sv_enableEasyElevators",
				false, Game::DVAR_FLAG_CHEAT | Game::DVAR_FLAG_REPLICATED,
				"Enable easy elevators for trickshotting");
		});

		// Place hook PM_CorrectAllSolid so we may skip PM_Trace check
		Utils::Hook(0x5734F9, Elevators::PM_CorrectAllSolidStub, HOOK_JUMP).install()->quick();
		Utils::Hook::Nop(0x5734FE, 1);

		// Place hook PM_CheckDuck so we may skip PM_Trace check
		Utils::Hook(0x570ECD, Elevators::PM_CheckDuckStub, HOOK_JUMP).install()->quick();
	}

	Elevators::~Elevators()
	{
	}
}
