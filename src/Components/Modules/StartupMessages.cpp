#include "STDInclude.hpp"

namespace Components
{
	std::list<std::string> startupMessages;

	int totalMessages = -1;

	StartupMessages::StartupMessages()
	{
		Dvar::OnInit([]()
		{
			Dvar::Register<const char*>("ui_startupMessage", "", Game::DVAR_FLAG_USERCREATED | Game::DVAR_FLAG_WRITEPROTECTED, "");
			Dvar::Register<const char*>("ui_startupMessageTitle", "", Game::DVAR_FLAG_USERCREATED | Game::DVAR_FLAG_WRITEPROTECTED, "");
			Dvar::Register<const char*>("ui_startupNextButtonText", "", Game::DVAR_FLAG_USERCREATED | Game::DVAR_FLAG_WRITEPROTECTED, "");
		});

		UIScript::Add("nextStartupMessage", [](UIScript::Token)
		{
			if (!startupMessages.size()) return;

			if (totalMessages < 1)
			{
				totalMessages = startupMessages.size();
			}

			std::string message = startupMessages.front();
			startupMessages.pop_front();

			Game::Dvar_SetStringByName("ui_startupMessage", message.data());
			Game::Dvar_SetStringByName("ui_startupMessageTitle", Utils::String::VA("Messages (%d/%d)", totalMessages - startupMessages.size(), totalMessages));
			Game::Dvar_SetStringByName("ui_startupNextButtonText", startupMessages.size() ? "Next" : "Close"); 
			Game::Cbuf_AddText(0, "openmenu startup_messages");
		});
	}

	void StartupMessages::AddMessage(std::string message)
	{
		startupMessages.push_back(message);
	}

}