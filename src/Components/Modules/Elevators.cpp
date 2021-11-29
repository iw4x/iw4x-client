#include "STDInclude.hpp"

namespace Components
{
	Game::dvar_t* Elevators::SV_DisableElevators;

	__declspec(naked) void Elevators::PM_GroundTraceStub()
	{
		__asm
		{
			push eax
			mov eax, Elevators::SV_DisableElevators
			cmp byte ptr [eax + 16], 1
			pop eax

			// Always skip PM_CorrectAllSolid if SV_DisableElevators is set to 1
			je noElevators

			// Original code
			cmp byte ptr [esp + 0x50], 0
			rep movsd
			mov esi, [esp + 0x58]

			// Original code flow
			push 0x573694 
			retn

		noElevators:

			// Original code
			rep movsd
			mov esi, [esp + 0x58]

			// Jump over call to PM_CorrectAllSolid
			push 0x5736AE
			retn
		}
	}

	Elevators::Elevators()
	{
		Dvar::OnInit([]
		{
			Elevators::SV_DisableElevators = Game::Dvar_RegisterBool("sv_disableElevators",
				false, Game::DVAR_FLAG_CHEAT | Game::DVAR_FLAG_REPLICATED,
				"Disable elevators");
		});

		// Hook PM_GroundTrace so me way skip PM_CorrectAllSolid (disable elevators)
		Utils::Hook(0x573689, Elevators::PM_GroundTraceStub, HOOK_JUMP).install()->quick();
	}

	Elevators::~Elevators()
	{
	}
}
