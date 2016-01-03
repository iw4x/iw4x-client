#include "..\..\STDInclude.hpp"

namespace Components
{
	Dvar::Var Localization::UseLocalization;
	std::map<std::string, std::string> Localization::LocalizeMap;

	void Localization::Set(const char* key, const char* value)
	{
		Localization::LocalizeMap[key] = value;
	}

	const char* Localization::Get(const char* key)
	{
		if (!Localization::UseLocalization.Get<bool>()) return key;

		if (Localization::LocalizeMap.find(key) != Localization::LocalizeMap.end())
		{
			return Localization::LocalizeMap[key].data();
		}

		Game::localizedEntry_s* entry = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_LOCALIZE, key).localize;

		if (entry)
		{
			return entry->value;
		}

		return key;
	}

	void __stdcall Localization::SetStringStub(const char* key, const char* value, bool isEnglish)
	{
		Localization::Set(key, value);
	}

	DWORD Localization::SELoadLanguageStub()
	{
		//'official' iw4m localized strings
		Game::SE_Load("localizedstrings/iw4m.str", 0);

		return Utils::Hook::Call<DWORD()>(0x629E20)();
	}

	Localization::Localization()
	{
		// Resolving hook
		Utils::Hook(0x629B90, Localization::Get, HOOK_JUMP).Install()->Quick();

		// Set loading entry point
		Utils::Hook(0x41D859, Localization::SELoadLanguageStub, HOOK_CALL).Install()->Quick();

		// Overwrite SetString
		Utils::Hook(0x4CE5EE, Localization::SetStringStub, HOOK_CALL).Install()->Quick();

		// TODO: Get rid of those!
		Localization::Set("MENU_SEARCHINGFORGAMES_100MS", "");
		Localization::Set("MP_SEARCHING_FOR_PLAYER", "Waiting");
		Localization::Set("MENU_WAITING_FOR_MORE_PLAYERS_TEAMS", "Waiting for more players to balance teams");
		Localization::Set("MENU_MOTD", "News");
		Localization::Set("MENU_MOTD_CAPS", "NEWS");
		Localization::Set("MENU_MODS", "Mods");
		Localization::Set("MENU_MODS_CAPS", "MODS");
		Localization::Set("MPUI_DESC_MODS", "Browse your Mods.");
		Localization::Set("MENU_THEATER", "Theater");
		Localization::Set("MENU_THEATER_CAPS", "THEATER");
		Localization::Set("MPUI_DESC_THEATER", "View your played matches.");
		Localization::Set("MENU_FOV", "Field of View");
		Localization::Set("MENU_NOBORDER", "Disable Window Border");
		Localization::Set("MENU_NATIVECURSOR", "Display native cursor");
		Localization::Set("MENU_MAXPACKETS", "Max. Packets per frame");
		Localization::Set("MENU_SNAPS", "Snapshot rate");
		Localization::Set("MENU_LAGOMETER", "Show Lagometer");
		Localization::Set("MENU_DRAWFPS", "Show FPS");
		Localization::Set("MENU_FPSLABELS", "Show FPS Labels");
		Localization::Set("MENU_NEWCOLORS", "Use new color codes");
		Localization::Set("MPUI_DESC_OPTIONS", "Set your game options.");
		Localization::Set("MPUI_DESC_QUIT", "Quit the game.");

		Localization::Set("PLATFORM_REFRESH_LIST", "Refresh List ^0- ^3F5");
		Localization::Set("PLATFORM_REFRESH_LIST_CAPS", "REFRESH LIST ^0- ^3F5");

		Localization::UseLocalization = Dvar::Register<bool>("ui_localize", true, Game::dvar_flag::DVAR_FLAG_NONE, "Use localization strings");
	}

	Localization::~Localization()
	{
		Localization::LocalizeMap.clear();
	}
}
