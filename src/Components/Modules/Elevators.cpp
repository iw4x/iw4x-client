#include "STDInclude.hpp"

namespace Components
{
	Dvar::Var Elevators::BG_Elevators;

	int Elevators::PM_CorrectAllSolid(Game::pmove_s* pm, Game::pml_t* pml, Game::trace_t* trace)
	{
		assert(pm != nullptr);
		assert(pm->ps != nullptr);

		Game::vec3_t point;
		auto* ps = pm->ps;

		auto i = 0;
		const auto elevatorSetting = Elevators::BG_Elevators.get<int>();
		while (true)
		{
			point[0] = ps->origin[0] + Game::CorrectSolidDeltas[i][0];
			point[1] = ps->origin[1] + Game::CorrectSolidDeltas[i][1];
			point[2] = ps->origin[2] + Game::CorrectSolidDeltas[i][2];

			Game::PM_playerTrace(pm, trace, point, point, &pm->bounds, ps->clientNum, pm->tracemask);

			// If the player wishes to glitch without effort they can do so
			if (!trace->startsolid || elevatorSetting == Elevators::EASY)
			{
				ps->origin[0] = point[0];
				ps->origin[1] = point[1];
				ps->origin[2] = point[2];
				point[2] = (ps->origin[2] - 1.0f) - 0.25f;

				Game::PM_playerTrace(pm, trace, ps->origin, point, &pm->bounds, ps->clientNum, pm->tracemask);

				// If elevators are disabled we need to check that startsolid is false before proceeding
				// like later versions of the game do
				if (!trace->startsolid || elevatorSetting >= Elevators::ENABLED)
				{
					break;
				}
			}	

			i += 1;
			if (i >= 26)
			{
				ps->groundEntityNum = Game::ENTITYNUM_NONE;
				pml->groundPlane = 0;
				pml->almostGroundPlane = 0;
				pml->walking = 0;
				Game::Jump_ClearState(ps);
				return 0;
			}
		}

		pml->groundTrace = *trace;

		const auto fraction = trace->fraction;
		ps->origin[0] += fraction * (point[0] - ps->origin[0]);
		ps->origin[1] += fraction * (point[1] - ps->origin[1]);
		ps->origin[2] += fraction * (point[2] - ps->origin[2]);

		return 1;
	}

	void Elevators::PM_Trace_Hk(Game::pmove_s* pm, Game::trace_t* trace, const float* f3,
		const float* f4, const Game::Bounds* bounds, int a6, int a7)
	{
		Game::PM_Trace(pm, trace, f3, f4, bounds, a6, a7);

		// Allow the player to stand even when there is no headroom
		if (Elevators::BG_Elevators.get<int>() == Elevators::EASY)
		{
			trace->allsolid = false;
		}
	}

	__declspec(naked) void Elevators::PM_CorrectAllSolidStub()
	{
		__asm
		{
			push eax
			pushad

			push [esp + 0x8 + 0x24]
			push [esp + 0x8 + 0x24]
			push eax
			call Elevators::PM_CorrectAllSolid
			add esp, 0xC

			mov [esp + 0x20], eax
			popad
			pop eax

			ret
		}
	}

	Elevators::Elevators()
	{
		Dvar::OnInit([]
		{
			static const char* values[] =
			{
				"off",
				"normal",
				"easy",
				nullptr
			};

			Elevators::BG_Elevators = Game::Dvar_RegisterEnum("bg_elevators", values,
				Elevators::ENABLED, Game::DVAR_FLAG_REPLICATED, "Elevators glitch settings");
		});

		//Replace PM_CorrectAllSolid
		Utils::Hook(0x57369E, Elevators::PM_CorrectAllSolidStub, HOOK_CALL).install()->quick();

		// Place hooks in PM_CheckDuck. If the elevators dvar is set to easy the
		// flags for duck/prone will always be removed from the player state
		Utils::Hook(0x570EC5, Elevators::PM_Trace_Hk, HOOK_CALL).install()->quick();
		Utils::Hook(0x570E0B, Elevators::PM_Trace_Hk, HOOK_CALL).install()->quick();
		Utils::Hook(0x570D70, Elevators::PM_Trace_Hk, HOOK_CALL).install()->quick();
	}

	Elevators::~Elevators()
	{
	}
}
