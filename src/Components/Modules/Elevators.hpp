#pragma once

namespace Components
{
	class Elevators : public Component
	{
	public:
		Elevators();
		~Elevators();

	private:
		static Game::dvar_t* SV_DisableElevators;

		static void PM_GroundTraceStub();
	};
}
