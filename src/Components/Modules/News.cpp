#include "STDInclude.hpp"

#define NEWS_MOTD_DEFAULT "Welcome to IW4x Multiplayer!"

namespace Components
{
	bool News::Terminate;
	std::thread News::Thread;
	std::string News::UpdaterArgs;

	bool News::unitTest()
	{
		bool result = true;

		if (News::Thread.joinable())
		{
			Logger::Print("Awaiting thread termination...\n");
			News::Thread.join();

			if (!strcmp(Localization::Get("MPUI_MOTD_TEXT"), NEWS_MOTD_DEFAULT))
			{
				Logger::Print("Failed to fetch motd!\n");
				result = false;
			}
			else
			{
				Logger::Print("Successfully fetched motd.\n");
			}
		}

		return result;
	}

	void News::ExitProcessStub(unsigned int exitCode)
	{
		std::this_thread::sleep_for(10ms);

		STARTUPINFOA        sInfo;
		PROCESS_INFORMATION pInfo;

		ZeroMemory(&sInfo, sizeof(sInfo));
		ZeroMemory(&pInfo, sizeof(pInfo));
		sInfo.cb = sizeof(sInfo);

		CreateProcessA("updater.exe", const_cast<char*>(Utils::String::VA("updater.exe %s", News::UpdaterArgs.data())), nullptr, nullptr, false, NULL, nullptr, nullptr, &sInfo, &pInfo);

		if (pInfo.hThread && pInfo.hThread != INVALID_HANDLE_VALUE) CloseHandle(pInfo.hThread);
		if (pInfo.hProcess && pInfo.hProcess != INVALID_HANDLE_VALUE) CloseHandle(pInfo.hProcess);

		TerminateProcess(GetCurrentProcess(), exitCode);
	}

	const char* News::GetNewsText()
	{
		return Localization::Get("MPUI_MOTD_TEXT");
	}

	void News::CheckForUpdate()
	{
		std::string _client = Utils::Cache::GetFile("/json/client");

		if (!_client.empty())
		{
			std::string error;
			json11::Json client = json11::Json::parse(_client.data(), error);

			int revisionNumber = 0;

			if (client["revision"].is_number())
			{
				revisionNumber = client["revision"].int_value();
			}
			else if (client["revision"].is_string())
			{
				revisionNumber = atoi(client["revision"].string_value().data());
			}
			else return;

			Dvar::Var("cl_updateversion").get<Game::dvar_t*>()->current.integer = revisionNumber;
			Dvar::Var("cl_updateavailable").get<Game::dvar_t*>()->current.boolean = (revisionNumber > REVISION);
		}
	}

	void News::LaunchUpdater(std::string params)
	{
		if (News::Updating()) return;

		News::UpdaterArgs = params;

		Localization::SetTemp("MENU_RECONNECTING_TO_PARTY", "Downloading updater");
		Command::Execute("openmenu popup_reconnectingtoparty", true);

		// Run the updater on shutdown
		Utils::Hook::Set(0x6D72A0, News::ExitProcessStub);

		std::thread([]()
		{
			std::string data = Utils::Cache::GetFile("/iw4/updater.exe");

			if (data.empty())
			{
				Localization::ClearTemp();
				News::UpdaterArgs.clear();
				Command::Execute("closemenu popup_reconnectingtoparty", false);
				Game::ShowMessageBox("Failed to download the updater!", "Error");
			}
			else
			{
				Console::SetSkipShutdown();
				Utils::IO::WriteFile("updater.exe", data);
				Command::Execute("wait 300; quit;", false);
			}
		}).detach();
	}

	bool News::Updating()
	{
		return !News::UpdaterArgs.empty();
	}

	News::News()
	{
		News::UpdaterArgs.clear();

		if (ZoneBuilder::IsEnabled() && Dedicated::Dedicated::IsEnabled()) return; // Maybe also dedi?

		Dvar::Register<bool>("g_firstLaunch", true, Game::DVAR_FLAG_SAVED, "");

		Dvar::Register<int>("cl_updateoldversion", REVISION, REVISION, REVISION, Game::DVAR_FLAG_WRITEPROTECTED, "Current version number.");
		Dvar::Register<int>("cl_updateversion", 0, 0, -1, Game::DVAR_FLAG_WRITEPROTECTED, "New version number.");
		Dvar::Register<bool>("cl_updateavailable", false, Game::DVAR_FLAG_WRITEPROTECTED, "New update is available.");

		UIScript::Add("checkFirstLaunch", [](UIScript::Token)
		{
			if (Dvar::Var("g_firstLaunch").get<bool>())
			{
				Command::Execute("openmenu menu_first_launch", false);
				//Dvar::Var("g_firstLaunch").set(false);
			}
		});

		UIScript::Add("visitWebsite", [](UIScript::Token)
		{
			ShellExecuteA(nullptr, "open", Utils::Cache::GetStaticUrl("").data(), nullptr, nullptr, SW_SHOWNORMAL);
		});

		UIScript::Add("visitWiki", [](UIScript::Token)
		{
			ShellExecuteA(nullptr, "open", Utils::Cache::GetStaticUrl("/wiki/").data(), nullptr, nullptr, SW_SHOWNORMAL);
		});

		Localization::Set("MPUI_CHANGELOG_TEXT", "Loading...");
		Localization::Set("MPUI_MOTD_TEXT", NEWS_MOTD_DEFAULT);

		if (Utils::IO::FileExists("updater.exe"))
		{
			remove("updater.exe");
		}

		// make newsfeed (ticker) menu items not cut off based on safe area
		Utils::Hook::Nop(0x63892D, 5);

		// hook for getting the news ticker string
		Utils::Hook::Nop(0x6388BB, 2); // skip the "if (item->text[0] == '@')" localize check
		Utils::Hook(0x6388C1, News::GetNewsText, HOOK_CALL).install()->quick();

		Command::Add("checkforupdate", [] (Command::Params*)
		{
			News::CheckForUpdate();
		});

		Command::Add("getautoupdate", [] (Command::Params*)
		{
			if (!Dvar::Var("cl_updateavailable").get<Game::dvar_t*>()->current.boolean) return;
			News::LaunchUpdater("-update -c");
		});

		if (!Utils::IsWineEnvironment() && !Loader::PerformingUnitTests() && !Dedicated::Dedicated::IsEnabled())
		{
			News::Terminate = false;
			News::Thread = std::thread([]()
			{
				Changelog::LoadChangelog();

				std::string data = Utils::Cache::GetFile("/iw4/motd.txt");

				if (!data.empty())
				{
					Localization::Set("MPUI_MOTD_TEXT", data);
				}

				if (!Loader::PerformingUnitTests())
				{
					while (!News::Terminate)
					{
						News::CheckForUpdate();

						// Sleep for 3 minutes
						for (int i = 0; i < 180 && !News::Terminate; ++i)
						{
							std::this_thread::sleep_for(1s);
						}
					}
				}
			});
		}
	}

	News::~News()
	{
		News::UpdaterArgs.clear();
	}

	void News::preDestroy()
	{
		News::Terminate = true;

		if (News::Thread.joinable())
		{
			News::Thread.join();
		}

		News::Thread = std::thread();
	}
}
