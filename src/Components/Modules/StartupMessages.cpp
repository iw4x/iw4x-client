
#include "Events.hpp"
#include "StartupMessages.hpp"

namespace Components
{
	int StartupMessages::TotalMessages = -1;
	std::list<std::tuple<std::string, std::string>> StartupMessages::MessageList;

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
		if (MessageList.empty()) return;

		if (TotalMessages < 1)
		{
			TotalMessages = static_cast<int>(MessageList.size());
		}

		const auto& message = MessageList.front();

		UIStartupMessage.set(std::get<1>(message));
		UIStartupMessageTitle.set(std::format("{} ({}/{})", std::get<0>(message), TotalMessages - static_cast<int>(MessageList.size()) + 1, TotalMessages));
		UIStartupNextButtonText.set(MessageList.size() <= 1 ? "Close" : "Next");

		MessageList.pop_front();
		Command::Execute("openmenu startup_messages", false);
	}

	void StartupMessages::AddMessage(const std::string& message)
	{
		MessageList.push_back(std::make_tuple("Messages", message));
	}

	void StartupMessages::AddMessage(const std::string& message, const std::string& title)
	{
		MessageList.push_back(std::make_tuple(title, message));
	}
}
