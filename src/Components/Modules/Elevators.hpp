#pragma once

namespace Components
{
	class Elevators : public Component
	{
	public:
		Elevators();

	private:
		enum ElevatorSettings { DISABLED, ENABLED, EASY };
		static Dvar::Var BG_Elevators;

		static int PM_CorrectAllSolid(Game::pmove_s* move, Game::pml_t* pml, Game::trace_t* trace);
		static void PM_CorrectAllSolidStub();
		static void PM_Trace_Hk(Game::pmove_s*, Game::trace_t*, const float*, const float*, const Game::Bounds*, int, int);
	};
}
