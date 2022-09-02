#include <STDInclude.hpp>

namespace Components
{
	std::recursive_mutex Localization::LocalizeMutex;
	Dvar::Var Localization::UseLocalization;
	std::unordered_map<std::string, Game::LocalizeEntry*> Localization::LocalizeMap;
	std::unordered_map<std::string, Game::LocalizeEntry*> Localization::TempLocalizeMap;

	void Localization::Set(const std::string& key, const std::string& value)
	{
		std::lock_guard _(Localization::LocalizeMutex);
		Utils::Memory::Allocator* allocator = Utils::Memory::GetAllocator();

		if (Localization::LocalizeMap.contains(key))
		{
			Game::LocalizeEntry* entry = Localization::LocalizeMap[key];

			char* newStaticValue = allocator->duplicateString(value);
			if (!newStaticValue) return;
			if (entry->value) allocator->free(entry->value);
			entry->value = newStaticValue;
			return;
		}

		Game::LocalizeEntry* entry = allocator->allocate<Game::LocalizeEntry>();
		if (!entry) return;

		entry->name = allocator->duplicateString(key);
		if (!entry->name)
		{
			allocator->free(entry);
			return;
		}

		entry->value = allocator->duplicateString(value);
		if (!entry->value)
		{
			allocator->free(entry->name);
			allocator->free(entry);
			return;
		}

		Localization::LocalizeMap[key] = entry;
	}

	const char* Localization::Get(const char* key)
	{
		if (!Localization::UseLocalization.get<bool>()) return key;

		Game::LocalizeEntry* entry = nullptr;
		{
			std::lock_guard _(Localization::LocalizeMutex);

			if (Localization::TempLocalizeMap.contains(key))
			{
				entry = Localization::TempLocalizeMap[key];
			}
			else if (Localization::LocalizeMap.contains(key))
			{
				entry = Localization::LocalizeMap[key];
			}
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

	void Localization::SetTemp(const std::string& key, const std::string& value)
	{
		std::lock_guard _(Localization::LocalizeMutex);
		Utils::Memory::Allocator* allocator = Utils::Memory::GetAllocator();

		if (Localization::TempLocalizeMap.contains(key))
		{
			Game::LocalizeEntry* entry = Localization::TempLocalizeMap[key];
			if (entry->value) allocator->free(entry->value);
			entry->value = allocator->duplicateString(value);
		}
		else
		{
			Game::LocalizeEntry* entry = allocator->allocate<Game::LocalizeEntry>();
			if (!entry) return;

			entry->name = allocator->duplicateString(key);
			if (!entry->name)
			{
				allocator->free(entry);
				return;
			}

			entry->value = allocator->duplicateString(value);
			if (!entry->value)
			{
				allocator->free(entry->name);
				allocator->free(entry);
				return;
			}

			Localization::TempLocalizeMap[key] = entry;
		}
	}

	void Localization::ClearTemp()
	{
		std::lock_guard _(Localization::LocalizeMutex);
		Utils::Memory::Allocator* allocator = Utils::Memory::GetAllocator();

		for (auto i = Localization::TempLocalizeMap.begin(); i != Localization::TempLocalizeMap.end(); ++i)
		{
			if (i->second)
			{
				if (i->second->name)  allocator->free(i->second->name);
				if (i->second->value) allocator->free(i->second->value);
				allocator->free(i->second);
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
			"/dev/../",
			"/dev/console",
			"/dev/full",
			"/dev/sdb",
			"/dev/sr0",
			"/dev/tty0",
			"/dev/urandom",
			"Dss0",
			"FutureRave",
			"H3X1C",
			"Homura",
			"Laupetin",
			"Louvenarde",
			"lsb_release -a",
			"quaK",			
		};

		static const char* contributors[] =
		{
			"a231",
			"AmateurHailbut",
			"Aoki",
			"Chase",
			"civil",
			"Dasfonia",
			"Deity",
			"Dizzy",
			"HardNougat",
			"INeedGames",
			"JTAG",
			"Killera",
			"Lithium",
			"OneFourOne",
			"RaidMax",
			"Revo",
			"RezTech",
			"Shadow the Hedgehog",
			"Slykuiper",
			"st0rm",
			"VVLNT",
			"X3RX35",
		};

		static const char* specials[] =
		{
			"NTAuthority",
			"aerosoul94",
			"ReactIW4",
			"IW4Play",
			"V2",
			"luckyy"
		};

		std::string credits = "^2The IW4x Team:^7\n";

		for (std::size_t i = 0; i < ARRAYSIZE(staff); ++i)
		{
			credits.append(staff[i]);
			credits.append("\n");
		}

		credits.append("\n^3Contributors:^7\n");

		for (std::size_t i = 0; i < ARRAYSIZE(contributors); ++i)
		{
			credits.append(contributors[i]);
			credits.append("\n");
		}

		credits.append("\n^5Special thanks to:^7\n");

		for (std::size_t i = 0; i < ARRAYSIZE(specials); ++i)
		{
			credits.append(specials[i]);
			credits.append("\n");
		}

		// I have no idea why, but the last 2 lines are invisible!
		credits.append("-\n-");

		Localization::Set("IW4X_CREDITS", credits);
	}

	const char* Localization::SEH_LocalizeTextMessageStub(const char* pszInputBuffer, const char* pszMessageType, Game::msgLocErrType_t errType)
	{
		constexpr auto szStringCount = 10;
		constexpr auto szStringSize = 1024;

		char szInsertBuf[szStringSize];
		char szTokenBuf[szStringSize];

		static thread_local int iCurrString;
		static thread_local char szStrings[szStringCount][szStringSize];

		iCurrString = (iCurrString + 1) % szStringCount;
		std::memset(szStrings[iCurrString], 0, sizeof(szStrings[0]));
		auto* pszString = szStrings[iCurrString];
		auto iLen = 0;
		auto bLocOn = 1;
		auto bInsertEnabled = 1;
		auto iInsertLevel = 0;
		auto insertIndex = 1;
		auto bLocSkipped = 0;
		const auto* pszTokenStart = pszInputBuffer;
		const auto* pszIn = pszInputBuffer;

		auto i = 0;
		while (*pszTokenStart)
		{
			if (*pszIn && *pszIn != '\x14' && *pszIn != '\x15' && *pszIn != '\x16')
			{
				++pszIn;
				continue;
			}

			if (pszIn > pszTokenStart)
			{
				auto iTokenLen = pszIn - pszTokenStart;
				Game::I_strncpyz_s(szTokenBuf, sizeof(szTokenBuf), pszTokenStart, pszIn - pszTokenStart);
				if (bLocOn)
				{
					if (!Game::SEH_GetLocalizedTokenReference(szTokenBuf, szTokenBuf, pszMessageType, errType))
					{
						return nullptr;
					}

					iTokenLen = std::strlen(szTokenBuf);
				}

				if (iTokenLen + iLen >= szStringSize)
				{
					Game::Com_Printf(Game::CON_CHANNEL_SYSTEM, "%s too long when translated\n", pszMessageType);
					return nullptr;
				}

				for (i = 0; i < iTokenLen - 2; ++i)
				{
					if (!std::strncmp(&szTokenBuf[i], "&&", 2) && std::isdigit(szTokenBuf[i + 2]))
					{
						if (bInsertEnabled)
						{
							++iInsertLevel;
						}
						else
						{
							szTokenBuf[i] = '\x16';
							bLocSkipped = 1;
						}
					}
				}

				if (iInsertLevel <= 0 || iLen <= 0)
				{
					Game::I_strcpy(&pszString[iLen], szStringSize - iLen, szTokenBuf);
				}
				else
				{
					for (i = 0; i < iLen - 2; ++i)
					{
						if (!std::strncmp(&pszString[i], "&&", 2) && std::isdigit(pszString[i + 2]))
						{
							const auto digit = pszString[i + 2] - 48;
							if (!digit)
							{
								Game::Com_Printf(Game::CON_CHANNEL_SYSTEM, "%s cannot have &&0 as conversion format: \"%s\"\n", pszMessageType, pszInputBuffer);
							}
							if (digit == insertIndex)
							{
								Game::I_strcpy(szInsertBuf, sizeof(szInsertBuf), &pszString[i + 3]);
								pszString[i] = 0;
								++insertIndex;
								break;
							}
						}
					}

					Game::I_strcpy(&pszString[i], szStringSize - i, szTokenBuf);
					Game::I_strcpy(&pszString[iTokenLen + i], szStringSize - (iTokenLen + i), szInsertBuf);

					iLen -= 3;
					--iInsertLevel;
				}

				iLen += iTokenLen;
			}

			bInsertEnabled = 1;
			if (*pszIn == '\x14')
			{
				bLocOn = 1;
				++pszIn;
			}
			else if (*pszIn == '\x15')
			{
				bLocOn = 0;
				++pszIn;
			}

			if (*pszIn == '\x16')
			{
				bInsertEnabled = 0;
				++pszIn;
			}

			pszTokenStart = pszIn;
		}

		if (bLocSkipped)
		{
			for (i = 0; i < iLen; ++i)
			{
				if (pszString[i] == '\x16')
				{
					pszString[i] = '%';
				}
			}
		}

		return pszString;
	}

	Localization::Localization()
	{
		Localization::SetCredits();

		AssetHandler::OnFind(Game::XAssetType::ASSET_TYPE_LOCALIZE_ENTRY, [](Game::XAssetType, const std::string& filename)
		{
			Game::XAssetHeader header = { nullptr };
			std::lock_guard _(Localization::LocalizeMutex);

			if (Localization::TempLocalizeMap.contains(filename))
			{
				header.localize = Localization::TempLocalizeMap[filename];
			}
			else if (Localization::LocalizeMap.contains(filename))
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

		Utils::Hook(0x49D4A0, Localization::SEH_LocalizeTextMessageStub, HOOK_JUMP).install()->quick();
		Utils::Hook::Nop(0x49D4A5, 1);

		Localization::UseLocalization = Dvar::Register<bool>("ui_localize", true, Game::DVAR_NONE, "Use localization strings");

		// Generate localized entries for custom classes above 10
		AssetHandler::OnLoad([](Game::XAssetType type, Game::XAssetHeader asset, const std::string& name, bool* /*restrict*/)
		{
			if (type != Game::XAssetType::ASSET_TYPE_LOCALIZE_ENTRY) return;

			if (name == "CLASS_SLOT1"s)
			{
				for (int i = 11; i <= NUM_CUSTOM_CLASSES; ++i)
				{
					std::string key = Utils::String::VA("CLASS_SLOT%i", i);

					std::string value = asset.localize->value;
					Utils::String::Replace(value, "1", Utils::String::VA("%i", i)); // Pretty ugly, but it should work

					Localization::Set(key, value);
				}
			}
		});
	}

	Localization::~Localization()
	{
		Localization::ClearTemp();
		Localization::LocalizeMap.clear();
	}
}
