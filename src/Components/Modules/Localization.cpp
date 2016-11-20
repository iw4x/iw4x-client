#include "STDInclude.hpp"

namespace Components
{
	std::mutex Localization::LocalizeMutex;
	Dvar::Var Localization::UseLocalization;
	Utils::Memory::Allocator Localization::MemAllocator;
	std::map<std::string, Game::LocalizedEntry*> Localization::LocalizeMap;
	std::map<std::string, Game::LocalizedEntry*> Localization::TempLocalizeMap;

	void Localization::Set(std::string key, std::string value)
	{
		std::lock_guard<std::mutex> _(Localization::LocalizeMutex);

		if (Localization::LocalizeMap.find(key) != Localization::LocalizeMap.end())
		{
			Game::LocalizedEntry* entry = Localization::LocalizeMap[key];

			char* newStaticValue = Localization::MemAllocator.duplicateString(value);
			if (!newStaticValue) return;
			if (entry->value) Localization::MemAllocator.free(entry->value);
			entry->value = newStaticValue;
			return;
		}

		Game::LocalizedEntry* entry = Localization::MemAllocator.allocate<Game::LocalizedEntry>();
		if (!entry) return;

		entry->name = Localization::MemAllocator.duplicateString(key);
		if (!entry->name)
		{
			Localization::MemAllocator.free(entry);
			return;
		}

		entry->value = Localization::MemAllocator.duplicateString(value);
		if (!entry->value)
		{
			Localization::MemAllocator.free(entry->name);
			Localization::MemAllocator.free(entry);
			return;
		}

		Localization::LocalizeMap[key] = entry;
	}

	const char* Localization::Get(const char* key)
	{
		if (!Localization::UseLocalization.get<bool>()) return key;

		Game::LocalizedEntry* entry = nullptr;
		std::lock_guard<std::mutex> _(Localization::LocalizeMutex);

		if (Localization::TempLocalizeMap.find(key) != Localization::TempLocalizeMap.end())
		{
			entry = Localization::TempLocalizeMap[key];
		}
		else if (Localization::LocalizeMap.find(key) != Localization::LocalizeMap.end())
		{
			entry = Localization::LocalizeMap[key];
		}

		if (!entry || !entry->value)
		{
			Localization::LocalizeMutex.unlock();
			entry = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_LOCALIZE, key).localize;
			Localization::LocalizeMutex.lock();
		}

		if (entry && entry->value)
		{
			return entry->value;
		}

		return key;
	}

	void Localization::SetTemp(std::string key, std::string value)
	{
		std::lock_guard<std::mutex> _(Localization::LocalizeMutex);

		if (Localization::TempLocalizeMap.find(key) != Localization::TempLocalizeMap.end())
		{
			Game::LocalizedEntry* entry = Localization::TempLocalizeMap[key];
			if(entry->value) Localization::MemAllocator.free(entry->value);
			entry->value = Localization::MemAllocator.duplicateString(value);
		}
		else
		{
			Game::LocalizedEntry* entry = Localization::MemAllocator.allocate<Game::LocalizedEntry>();
			if (!entry) return;

			entry->name = Localization::MemAllocator.duplicateString(key);
			if (!entry->name)
			{
				Localization::MemAllocator.free(entry);
				return;
			}

			entry->value = Localization::MemAllocator.duplicateString(value);
			if (!entry->value)
			{
				Localization::MemAllocator.free(entry->name);
				Localization::MemAllocator.free(entry);
				return;
			}

			Localization::TempLocalizeMap[key] = entry;
		}
	}

	void Localization::ClearTemp()
	{
		std::lock_guard<std::mutex> _(Localization::LocalizeMutex);

		for (auto i = Localization::TempLocalizeMap.begin(); i != Localization::TempLocalizeMap.end(); ++i)
		{
			if (i->second)
			{
				if (i->second->name)  Localization::MemAllocator.free(i->second->name);
				if (i->second->value) Localization::MemAllocator.free(i->second->value);
				Localization::MemAllocator.free(i->second);
			}
		}

		Localization::TempLocalizeMap.clear();
	}

	void __stdcall Localization::SetStringStub(const char* key, const char* value, bool /*isEnglish*/)
	{
		Localization::Set(key, value);
	}

	void Localization::LoadLanguageStrings()
	{
		//if (ZoneBuilder::IsEnabled())
		{
			if (FileSystem::File(fmt::sprintf("localizedstrings/iw4x_%s.str", Game::Win_GetLanguage())).exists())
			{
				Game::SE_Load(Utils::String::VA("localizedstrings/iw4x_%s.str", Game::Win_GetLanguage()), 0);
			}
			else if (FileSystem::File("localizedstrings/iw4x_english.str").exists())
			{
				Game::SE_Load("localizedstrings/iw4x_english.str", 0);
			}
		}
	}

	__declspec(naked) void Localization::SELoadLanguageStub()
	{
		__asm
		{
			pushad
			call Localization::LoadLanguageStrings
			popad

			push 629E20h
			retn
		}
	}

	Localization::Localization()
	{
		AssetHandler::OnFind(Game::XAssetType::ASSET_TYPE_LOCALIZE, [] (Game::XAssetType, std::string filename)
		{
			Game::XAssetHeader header = { 0 };
			std::lock_guard<std::mutex> _(Localization::LocalizeMutex);

			if (Localization::TempLocalizeMap.find(filename) != Localization::TempLocalizeMap.end())
			{
				header.localize = Localization::TempLocalizeMap[filename];
			}
			else if (Localization::LocalizeMap.find(filename) != Localization::LocalizeMap.end())
			{
				header.localize = Localization::LocalizeMap[filename];
			}

			return header;
		});

		// Resolving hook
		Utils::Hook(0x629B90, Localization::Get, HOOK_JUMP).install()->quick();

		// Set loading entry point
		Utils::Hook(0x41D859, Localization::SELoadLanguageStub, HOOK_CALL).install()->quick();

		// Overwrite SetString
		Utils::Hook(0x4CE5EE, Localization::SetStringStub, HOOK_CALL).install()->quick();

/*
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
		Localization::Set("PLATFORM_REFRESH_LIST_CAPS", "REFRESH LIST ^0- ^/*3F5");
*/

		Localization::UseLocalization = Dvar::Register<bool>("ui_localize", true, Game::dvar_flag::DVAR_FLAG_NONE, "Use localization strings");
	}

	Localization::~Localization()
	{
		Localization::ClearTemp();

		Localization::LocalizeMap.clear();
		Localization::MemAllocator.clear();
	}
}
