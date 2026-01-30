#pragma once

namespace Components
{
	class Discovery : public Component
	{
	public:
		Discovery();

		static void Perform();

	private:
		static bool IsPerforming;
		static std::jthread Thread;
		static std::string Challenge;

		static Dvar::Var NetDiscoveryPortRangeMin;
		static Dvar::Var NetDiscoveryPortRangeMax;
	};
}
