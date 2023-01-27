#pragma once

namespace Components
{
	class StartupMessages : public Component
	{
	public:
		StartupMessages();

		static void AddMessage(const std::string& message);

	private:
		static int TotalMessages;
		static std::list<std::string> MessageList;

		static Dvar::Var UIStartupMessage;
		static Dvar::Var UIStartupMessageTitle;
		static Dvar::Var UIStartupNextButtonText;
	};
}
