#include <STDInclude.hpp>

namespace Components
{
	int StartupMessages::TotalMessages = -1;
	std::list<std::string> StartupMessages::MessageList;

	StartupMessages::StartupMessages()
	{
		Scheduler::Once([]
		{
			Dvar::Register<const char*>("ui_startupMessage", "", Game::DVAR_EXTERNAL | Game::DVAR_INIT, "");
			Dvar::Register<const char*>("ui_startupMessageTitle", "", Game::DVAR_EXTERNAL | Game::DVAR_INIT, "");
			Dvar::Register<const char*>("ui_startupNextButtonText", "", Game::DVAR_EXTERNAL | Game::DVAR_INIT, "");
		}, Scheduler::Pipeline::MAIN);

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
			Game::Cbuf_AddText(0, "openmenu startup_messages\n");

			StartupMessages::MessageList.pop_front();
		});
	}

	void StartupMessages::AddMessage(const std::string& message)
	{
		StartupMessages::MessageList.push_back(message);
	}
}
