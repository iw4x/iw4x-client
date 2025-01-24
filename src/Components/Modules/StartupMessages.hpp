#pragma once

namespace Components
{
	class StartupMessages : public Component
	{
	public:
		StartupMessages();

		static void AddMessage(const std::string& message);
		static void AddMessage(const std::string& message, const std::string& title);
		static void Show();

	private:
		static int TotalMessages;
		static std::list<std::tuple<std::string, std::string>> MessageList;

		static Dvar::Var UIStartupMessage;
		static Dvar::Var UIStartupMessageTitle;
		static Dvar::Var UIStartupNextButtonText;
	};
}
