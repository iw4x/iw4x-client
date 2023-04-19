#include <STDInclude.hpp>
#include "Changelog.hpp"
#include "News.hpp"

#define NEWS_MOTD_DEFAULT "Welcome to IW4x Multiplayer!"

namespace Components
{
	bool News::Terminate;
	std::thread News::Thread;

	bool News::unitTest()
	{
		bool result = true;

		if (Thread.joinable())
		{
			Logger::Debug("Awaiting thread termination...");
			Thread.join();

			if (!std::strcmp(Localization::Get("MPUI_MOTD_TEXT"), NEWS_MOTD_DEFAULT))
			{
				Logger::Print("Failed to fetch motd!\n");
				result = false;
			}
			else
			{
				Logger::Print("Successfully fetched motd");
			}
		}

		return result;
	}

	const char* News::GetNewsText()
	{
		return Localization::Get("MPUI_MOTD_TEXT");
	}

	News::News()
	{
		if (ZoneBuilder::IsEnabled() || Dedicated::IsEnabled()) return; // Maybe also dedi?

		Dvar::Register<bool>("g_firstLaunch", true, Game::DVAR_ARCHIVE, "");

		UIScript::Add("checkFirstLaunch", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
		{
			if (Dvar::Var("g_firstLaunch").get<bool>())
			{
				Command::Execute("openmenu menu_first_launch", false);
				//Dvar::Var("g_firstLaunch").set(false); // The menus should set it
			}
		});

		UIScript::Add("visitWebsite", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
		{
			Utils::OpenUrl(Utils::Cache::GetUrl(Utils::Cache::Urls[1], {}));
		});

		Localization::Set("MPUI_CHANGELOG_TEXT", "Loading...");
		Localization::Set("MPUI_MOTD_TEXT", NEWS_MOTD_DEFAULT);

		// make newsfeed (ticker) menu items not cut off based on safe area
		Utils::Hook::Nop(0x63892D, 5);

		// hook for getting the news ticker string
		Utils::Hook::Nop(0x6388BB, 2); // skip the "if (item->text[0] == '@')" localize check
		Utils::Hook(0x6388C1, GetNewsText, HOOK_CALL).install()->quick();

		if (!Utils::IsWineEnvironment() && !Loader::IsPerformingUnitTests())
		{
			Terminate = false;
			Thread = std::thread([]()
			{
				Changelog::LoadChangelog();
				if (Terminate) return;

				const auto data = Utils::Cache::GetFile("/iw4/motd.txt");
				if (!data.empty())
				{
					Localization::Set("MPUI_MOTD_TEXT", data);
				}

				if (!Loader::IsPerformingUnitTests() && !Terminate)
				{
					while (!Terminate)
					{
						// Sleep for 3 minutes
						for (int i = 0; i < 180 && !Terminate; ++i)
						{
							Game::Sys_Sleep(1);
						}
					}
				}
			});
		}
	}

	void News::preDestroy()
	{
		Terminate = true;

		if (Thread.joinable())
		{
			Thread.join();
		}
	}
}
