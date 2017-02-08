#include "STDInclude.hpp"

namespace Components
{
	std::recursive_mutex Localization::LocalizeMutex;
	Dvar::Var Localization::UseLocalization;
	Utils::Memory::Allocator Localization::MemAllocator;
	std::unordered_map<std::string, Game::LocalizeEntry*> Localization::LocalizeMap;
	std::unordered_map<std::string, Game::LocalizeEntry*> Localization::TempLocalizeMap;

	void Localization::Set(std::string key, std::string value)
	{
		std::lock_guard<std::recursive_mutex> _(Localization::LocalizeMutex);

		if (Localization::LocalizeMap.find(key) != Localization::LocalizeMap.end())
		{
			Game::LocalizeEntry* entry = Localization::LocalizeMap[key];

			char* newStaticValue = Localization::MemAllocator.duplicateString(value);
			if (!newStaticValue) return;
			if (entry->value) Localization::MemAllocator.free(entry->value);
			entry->value = newStaticValue;
			return;
		}

		Game::LocalizeEntry* entry = Localization::MemAllocator.allocate<Game::LocalizeEntry>();
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

		Game::LocalizeEntry* entry = nullptr;
		std::lock_guard<std::recursive_mutex> _(Localization::LocalizeMutex);

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
			entry = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_LOCALIZE_ENTRY, key).localize;
		}

		if (entry && entry->value)
		{
			return entry->value;
		}

		return key;
	}

	void Localization::SetTemp(std::string key, std::string value)
	{
		std::lock_guard<std::recursive_mutex> _(Localization::LocalizeMutex);

		if (Localization::TempLocalizeMap.find(key) != Localization::TempLocalizeMap.end())
		{
			Game::LocalizeEntry* entry = Localization::TempLocalizeMap[key];
			if(entry->value) Localization::MemAllocator.free(entry->value);
			entry->value = Localization::MemAllocator.duplicateString(value);
		}
		else
		{
			Game::LocalizeEntry* entry = Localization::MemAllocator.allocate<Game::LocalizeEntry>();
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
		std::lock_guard<std::recursive_mutex> _(Localization::LocalizeMutex);

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
			if (FileSystem::File(Utils::String::VA("localizedstrings/iw4x_%s.str", Game::Win_GetLanguage())).exists())
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

	void Localization::SetCredits()
	{
		static const char* staff[] =
		{
			"Snake",
			"/dev/sr0",
			"/dev/../",
			"/dev/console",
			"/dev/full",
			"/dev//dev/tty0",
			"/dev/urandom",
			//"BORUSSIA",
			//"HelloJim",
			//"Teddy",
			//"Ghost",
		};

		static const char* contributors[] =
		{
			"RezTech",
			"a231",
			"Lithium",
			"AmateurHailbut",
			"Aoki",
			"civil",
			"Dasfonia",
			"Dizzy",
			"HardNougat",
			"Killera",
			"Revo",
			"st0rm",
			"VVLNT",
			"Deity",
		};

		static const char* specials[] =
		{
			"NTAuthority",
			"momo5502",
			"TheApadayo",
			"aerosoul94",
		};

		std::string credits = "^2The IW4x Team:^7\n";

		for (int i = 0; i < ARRAYSIZE(staff); ++i)
		{
			credits.append(staff[i]);
			credits.append("\n");
		}

		credits.append("\n^3Contributors:^7\n");

		for (int i = 0; i < ARRAYSIZE(contributors); ++i)
		{
			credits.append(contributors[i]);
			credits.append("\n");
		}

		credits.append("\n^5Special thanks to:^7\n");

		for (int i = 0; i < ARRAYSIZE(specials); ++i)
		{
			credits.append(specials[i]);
			credits.append("\n");
		}

		// I have no idea why, but the last 2 lines are invisible!
		credits.append("-\n-");

		Localization::Set("IW4X_CREDITS", credits);
	}

	Localization::Localization()
	{
		Localization::SetCredits();

		AssetHandler::OnFind(Game::XAssetType::ASSET_TYPE_LOCALIZE_ENTRY, [] (Game::XAssetType, std::string filename)
		{
			Game::XAssetHeader header = { nullptr };
			std::lock_guard<std::recursive_mutex> _(Localization::LocalizeMutex);

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

		Localization::UseLocalization = Dvar::Register<bool>("ui_localize", true, Game::dvar_flag::DVAR_FLAG_NONE, "Use localization strings");
	}

	Localization::~Localization()
	{
		Localization::ClearTemp();

		Localization::LocalizeMap.clear();
		Localization::MemAllocator.clear();
	}
}
