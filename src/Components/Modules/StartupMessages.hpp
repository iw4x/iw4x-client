#pragma once

namespace Components
{
	class StartupMessages : public Component
	{
	public:
		StartupMessages();

		static void AddMessage(std::string message);

	private:
		static int TotalMessages;
		static std::list<std::string> MessageList;
	};
}
