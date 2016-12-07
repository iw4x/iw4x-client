#include "STDInclude.hpp"

#define NEWS_MOTD_DEFUALT "Welcome to IW4x Multiplayer!"

namespace Components
{
	bool News::Terminate;
	std::thread News::Thread;

	bool News::unitTest()
	{
		bool result = true;

		if (News::Thread.joinable())
		{
			Logger::Print("Awaiting thread termination...\n");
			News::Thread.join();

			if (!strlen(Localization::Get("MPUI_CHANGELOG_TEXT")) || Localization::Get("MPUI_CHANGELOG_TEXT") == "Loading..."s)
			{
				Logger::Print("Failed to fetch changelog!\n");
				result = false;
			}
			else
			{
				Logger::Print("Successfully fetched changelog.\n");
			}

			if (!strcmp(Localization::Get("MPUI_MOTD_TEXT"), NEWS_MOTD_DEFUALT))
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

		CreateProcessA("updater.exe", NULL, NULL, NULL, false, CREATE_NO_WINDOW, NULL, NULL, &sInfo, &pInfo);

		if (pInfo.hThread && pInfo.hThread != INVALID_HANDLE_VALUE)
		{
			CloseHandle(pInfo.hThread);
		}

		if (pInfo.hProcess && pInfo.hProcess != INVALID_HANDLE_VALUE)
		{
			CloseHandle(pInfo.hProcess);
		}

		TerminateProcess(GetCurrentProcess(), exitCode);
	}

	const char* News::GetNewsText()
	{
		return Localization::Get("MPUI_MOTD_TEXT");
	}

	void News::CheckForUpdate()
	{
		std::string caches = Utils::Cache::GetFile("/iw4/caches.xml");

		if (!caches.empty())
		{
			std::string str = "<Cache ID=\"iw4x\" Version=\"";
			auto pos = caches.find(str);

			if (pos != std::string::npos)
			{
				caches = caches.substr(pos + str.size());
				
				pos = caches.find_first_of("\"");

				if (pos != std::string::npos)
				{
					caches = caches.substr(0, pos);

					int version = atoi(caches.data());

					Dvar::Var("cl_updateversion").get<Game::dvar_t*>()->current.integer = version;
					Dvar::Var("cl_updateavailable").get<Game::dvar_t*>()->current.boolean = (version > REVISION);
				}
			}
		}
	}

	News::News()
	{
		if (ZoneBuilder::IsEnabled()) return; // Maybe also dedi?

		Dvar::Register<int>("cl_updateoldversion", REVISION, REVISION, REVISION, Game::DVAR_FLAG_WRITEPROTECTED, "Current version number.");
		Dvar::Register<int>("cl_updateversion", 0, 0, -1, Game::DVAR_FLAG_WRITEPROTECTED, "New version number.");
		Dvar::Register<bool>("cl_updateavailable", 0, Game::DVAR_FLAG_WRITEPROTECTED, "New update is available.");

		Localization::Set("MPUI_CHANGELOG_TEXT", "Loading...");
		Localization::Set("MPUI_MOTD_TEXT", NEWS_MOTD_DEFUALT);

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

			Localization::SetTemp("MENU_RECONNECTING_TO_PARTY", "Downloading updater");
			Command::Execute("openmenu popup_reconnectingtoparty", true);

			// Run the updater on shutdown
			Utils::Hook::Set(0x6D72A0, News::ExitProcessStub);

			std::thread([] ()
			{
				std::string data = Utils::Cache::GetFile("/iw4/updater.exe");

				if (data.empty())
				{
					Localization::ClearTemp();
					Command::Execute("closemenu popup_reconnectingtoparty", false);
					Game::MessageBox("Failed to download the updater!", "Error");
				}
				else
				{
					Console::SetSkipShutdown();
					Utils::IO::WriteFile("updater.exe", data);
					Command::Execute("wait 300; quit;", false);
				}
			}).detach();
		});

		if (!Utils::IsWineEnvironment())
		{
			News::Terminate = false;
			News::Thread = std::thread([] ()
			{
				Localization::Set("MPUI_CHANGELOG_TEXT", Utils::Cache::GetFile("/iw4/changelog.txt"));

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
		News::Terminate = true;

		if (News::Thread.joinable())
		{
			News::Thread.join();
		}
	}
}
