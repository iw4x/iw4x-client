#include "STDInclude.hpp"

namespace Components
{
	int StartupMessages::TotalMessages = -1;
	std::list<std::string> StartupMessages::MessageList;

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
			if (!StartupMessages::MessageList.size()) return;

			if (StartupMessages::TotalMessages < 1)
			{
				StartupMessages::TotalMessages = StartupMessages::MessageList.size();
			}

			const auto& message = StartupMessages::MessageList.front();

			Game::Dvar_SetStringByName("ui_startupMessage", message.data());
			Game::Dvar_SetStringByName("ui_startupMessageTitle", Utils::String::VA("Messages (%d/%d)", StartupMessages::TotalMessages - StartupMessages::MessageList.size(), StartupMessages::TotalMessages));
			Game::Dvar_SetStringByName("ui_startupNextButtonText", StartupMessages::MessageList.size() ? "Next" : "Close");
			Game::Cbuf_AddText(0, "openmenu startup_messages");

			StartupMessages::MessageList.pop_front();
		});
	}

	StartupMessages::~StartupMessages()
	{
		StartupMessages::MessageList.clear();
	}

	void StartupMessages::AddMessage(const std::string& message)
	{
		StartupMessages::MessageList.push_back(message);
	}
}
