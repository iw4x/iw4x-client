#pragma once

namespace Components
{
	class Elevators : public Component
	{
	public:
		Elevators();
		~Elevators();

	private:
		static Game::dvar_t* SV_EnableEasyElevators;

		static void PM_CorrectAllSolidStub();
		static void PM_CheckDuckStub();
	};
}
