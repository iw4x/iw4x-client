#pragma once

namespace Components
{
	class StartupMessages : public Component
	{
	public:
		StartupMessages();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "StartupMessages"; };
#endif

		static void AddMessage(std::string message);

	private:
		static int TotalMessages = -1;
		static std::list<std::string> MessageList;
	};
}
