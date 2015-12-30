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

	Localization::Localization()
	{
		Utils::Hook(0x629B90, Localization::Get, HOOK_JUMP).Install()->Quick();

		Localization::Set("MENU_SEARCHINGFORGAMES_100MS", "");
		Localization::Set("MP_SEARCHING_FOR_PLAYER", "Waiting");
		Localization::Set("MENU_WAITING_FOR_MORE_PLAYERS_TEAMS", "Waiting for more players to balance teams");

		// Don't perform non-english localization here, do it in fastfiles instead
		//Localization::Set("MP_SEARCHING_FOR_PLAYER", "Warte");
		//Localization::Set("MENU_WAITING_FOR_MORE_PLAYERS_TEAMS", "Auf weitere Spieler zum Teamausgleich warten");

		Localization::UseLocalization = Dvar::Register<bool>("ui_localize", true, Game::dvar_flag::DVAR_FLAG_NONE, "Use localization strings");
	}

	Localization::~Localization()
	{
		Localization::LocalizeMap.clear();
	}
}
