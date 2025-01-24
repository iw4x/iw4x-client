
#include "Events.hpp"
#include "StartupMessages.hpp"

/*
  "StartupMessages::Show()" is initially called by the "checkFirstLaunch" UIScript in News.cpp,
  which is called from main_text.menu once the main menu loads.
  Multiple messages can be added using StartupMessages::AddMessage(message, [overload] title)
  Messages are shown in the order they were added.
*/

namespace Components
{
	int StartupMessages::TotalMessages = -1;
	std::list<std::tuple<std::string, std::string>> StartupMessages::MessageList; // (title, message)

	Dvar::Var StartupMessages::UIStartupMessage;
	Dvar::Var StartupMessages::UIStartupMessageTitle;
	Dvar::Var StartupMessages::UIStartupNextButtonText;

	StartupMessages::StartupMessages()
	{
		Events::OnDvarInit([]
		{
			UIStartupMessage = Dvar::Register<const char*>("ui_startupMessage", "", Game::DVAR_NONE, "");
			UIStartupMessageTitle = Dvar::Register<const char*>("ui_startupMessageTitle", "", Game::DVAR_NONE, "");
			UIStartupNextButtonText = Dvar::Register<const char*>("ui_startupNextButtonText", "", Game::DVAR_NONE, "");
		});

		UIScript::Add("nextStartupMessage", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
		{
				StartupMessages::Show();
		});
	}

	void StartupMessages::Show()
	{
		if (MessageList.empty())
			return;

		int MessageListSize = static_cast<int>(MessageList.size());
		if (TotalMessages < 1)
			TotalMessages = MessageListSize;

		const auto& [title, body] = MessageList.front();

		const int MessageIndex = TotalMessages - MessageListSize + 1;
		const std::string formattedTitle = std::format("{} ({}/{})", title, MessageIndex, TotalMessages);
		const std::string nextButtonText = (MessageListSize <= 1) ? "Close" : "Next";

		UIStartupMessage.set(body);
		UIStartupMessageTitle.set(formattedTitle);
		UIStartupNextButtonText.set(nextButtonText);

		MessageList.pop_front();
		Command::Execute("openmenu startup_messages", false);
	}

	void StartupMessages::AddMessage(const std::string& message)
	{
		AddMessage(message, "Messages");
	}

	void StartupMessages::AddMessage(const std::string& message, const std::string& title)
	{
		MessageList.emplace_back(title, message);
	}
}
