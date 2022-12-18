#pragma once

namespace Components
{
	class Discovery : public Component
	{
	public:
		Discovery();

		void preDestroy() override;

		static void Perform();

	private:
		static bool IsTerminating;
		static bool IsPerforming;
		static std::thread Thread;
		static std::string Challenge;

		static Dvar::Var NetDiscoveryPortRangeMin;
		static Dvar::Var NetDiscoveryPortRangeMax;
	};
}
