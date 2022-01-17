#pragma once

namespace Components
{
	class StartupMessages : public Component
	{
	public:
		StartupMessages();
		~StartupMessages();

		static void AddMessage(const std::string& message);

	private:
		static int TotalMessages;
		static std::list<std::string> MessageList;
	};
}
