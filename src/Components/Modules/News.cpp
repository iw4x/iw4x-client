#include "Changelog.hpp"
#include "News.hpp"

#define NEWS_MOTD_DEFAULT "Welcome to IW4x Multiplayer!"

namespace Components
{
	bool News::unitTest()
	{
		if (!std::strcmp(Localization::Get("MPUI_MOTD_TEXT"), NEWS_MOTD_DEFAULT))
		{
			Logger::Print("Failed to fetch motd!\n");
			return false;
		}

		Logger::Print("Successfully fetched motd");
		return true;
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

		if (!Loader::IsPerformingUnitTests())
		{
			Changelog::LoadChangelog();

			const auto data = Utils::Cache::GetFile("/info/motd/plain");

			if (!data.empty())
				Localization::Set("MPUI_MOTD_TEXT", data);
		}
	}

	void News::preDestroy()
	{
	}
}
