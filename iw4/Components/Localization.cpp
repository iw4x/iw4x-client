#include "..\STDInclude.hpp"

namespace Components
{
	std::map<std::string, std::string> Localization::LocalizeMap;

	void Localization::Set(const char* key, const char* value)
	{
		Localization::LocalizeMap[key] = value;
	}

	const char* Localization::Get(const char* key)
	{
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

		Localization::Set("MENU_MULTIPLAYER_CAPS", "^5Fotze");
	}

	Localization::~Localization()
	{
		Localization::LocalizeMap.clear();
	}
}
