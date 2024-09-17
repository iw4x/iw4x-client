#include <STDInclude.hpp>

#include "Events.hpp"
#include "StartupMessages.hpp"

namespace Components
{
	int StartupMessages::TotalMessages = -1;
	std::list<std::string> StartupMessages::MessageList;

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
			if (MessageList.empty()) return;

			if (TotalMessages < 1)
			{
				TotalMessages = static_cast<int>(MessageList.size());
			}

			const auto& message = MessageList.front();

			UIStartupMessage.set(message);
			UIStartupMessageTitle.set(std::format("Messages ({}/{})", TotalMessages - MessageList.size(), TotalMessages));
			UIStartupNextButtonText.set(MessageList.empty() ? "Close" : "Next");
			Game::Cbuf_AddText(0, "openmenu startup_messages\n");

			MessageList.pop_front();
		});
	}

	void StartupMessages::AddMessage(const std::string& message)
	{
		MessageList.push_back(message);
	}
}
