#pragma once

namespace Components
{
	class Discovery : public Component
	{
	public:
		Discovery();
		~Discovery();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "Discovery"; };
#endif

		void preDestroy() override;

		static void Perform();

	private:
		static bool IsTerminating;
		static bool IsPerforming;
		static std::thread Thread;
		static std::string Challenge;
	};
}
