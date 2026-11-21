#include "Localization.hpp"
#include "ArenaLength.hpp"
#include "AssetHandler.hpp"
#include "Controller.hpp"
#include "GSC/Script.hpp"

namespace Components
{
  std::recursive_mutex Localization::LocalizeMutex;
  Dvar::Var Localization::UseLocalization;
  void(*Localization::SEH_UpdateLanguageInfo_Original)();
  std::unordered_map<std::string, Game::LocalizeEntry*> Localization::LocalizeMap;

  std::optional<std::string> Localization::PrefixOverride;
  std::function<void(Game::LocalizeEntry*)> Localization::ParseCallback;

  void Localization::Set(const std::string& psLocalReference, const std::string& psNewString)
  {
    std::lock_guard _(LocalizeMutex);
    Utils::Memory::Allocator* allocator = Utils::Memory::GetAllocator();

    auto key = psLocalReference;
    if (PrefixOverride.has_value())
    {
      key.insert(0, PrefixOverride.value());
    }

    if (LocalizeMap.contains(key))
    {
      auto* entry = LocalizeMap[key];

      const auto* newStaticValue = allocator->duplicateString(psNewString);
      if (!newStaticValue) return;

      if (entry->value) allocator->free(entry->value);
      entry->value = newStaticValue;

      SaveParseOutput(entry);

      return;
    }

    auto* entry = allocator->allocate<Game::LocalizeEntry>();
    if (!entry) return;

    entry->name = allocator->duplicateString(key);
    if (!entry->name)
    {
      allocator->free(entry);
      return;
    }

    entry->value = allocator->duplicateString(psNewString);
    if (!entry->value)
    {
      allocator->free(entry->name);
      allocator->free(entry);
      return;
    }

    SaveParseOutput(entry);

    LocalizeMap[key] = entry;
  }

  const char* Localization::Get(const char* key)
  {
    if (!UseLocalization.get<bool>()) return key;

    if (key && std::strncmp(key, "PLATFORM_", 9) == 0)
    {
      if (const auto* controllerString = Controller::PlatformString(key))
      {
        return controllerString;
      }
    }

    Game::LocalizeEntry* entry = nullptr;

    {
      std::lock_guard _(LocalizeMutex);

      if (LocalizeMap.contains(key))
      {
        entry = LocalizeMap[key];
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

  // Return an entry whose value is what Get() finds for the key.
  //
  // Note that x64 inlines SEH_StringEd_GetString(), which we replace with
  // Get() on x86, into each of its callers. So we hook the lookup of the
  // entry in each copy, which then reads the value from what we return.
  //
  Game::XAssetHeader Localization::DB_FindLocalizeEntry_Hk([[maybe_unused]] Game::XAssetType type, const char* key)
  {
    thread_local Game::LocalizeEntry entry;

    entry.value = Get(key);
    entry.name = key;

    Game::XAssetHeader header;
    header.localize = &entry;
    return header;
  }

  void Localization::ParseOutput(const std::function<void(Game::LocalizeEntry*)>& callback)
  {
    ParseCallback = callback;
  }

  void Localization::SaveParseOutput(Game::LocalizeEntry* asset)
  {
    if (ParseCallback)
    {
      ParseCallback(asset);
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
      "Evan/Eve",
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

    Set("IW4X_CREDITS", credits);
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
        auto iTokenLen = static_cast<int>(pszIn - pszTokenStart);
        Game::I_strncpyz_s(szTokenBuf, sizeof(szTokenBuf), pszTokenStart, pszIn - pszTokenStart);
        if (bLocOn)
        {
          if (!Game::SEH_GetLocalizedTokenReference(szTokenBuf, szTokenBuf, pszMessageType, errType))
          {
            return nullptr;
          }

          iTokenLen = static_cast<int>(std::strlen(szTokenBuf));
        }

        if (iTokenLen + iLen >= szStringSize)
        {
          Game::Com_Printf(Game::CON_CHANNEL_SYSTEM, "%s too long when translated\n", pszMessageType);
          return nullptr;
        }

        for (i = 0; i < iTokenLen - 2; ++i)
        {
          if (!std::strncmp(&szTokenBuf[i], "&&", 2) && std::isdigit(static_cast<unsigned char>(szTokenBuf[i + 2])))
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
            if (!std::strncmp(&pszString[i], "&&", 2) && std::isdigit(static_cast<unsigned char>(pszString[i + 2])))
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

  const char* Localization::LocalizeMapName(const char* mapName)
  {
    // The game loads the arenas on first use, and so must we: we may be
    // asked before any menu has needed them (say, when we connect on
    // startup).
    //
    if (!Dedicated::IsEnabled())
      Game::UI_LoadArenas();

    for (int i = 0; i < *Game::arenaCount; ++i)
    {
      if (!_stricmp(ArenaLength::NewArenas[i].mapName, mapName))
      {
        auto* uiName = &ArenaLength::NewArenas[i].uiName[0];
        if ((uiName[0] == 'M' && uiName[1] == 'P') || (uiName[0] == 'P' && uiName[1] == 'A')) // MPUI/PATCH
        {
          return Get(uiName);
        }

        return uiName;
      }
    }

    return mapName;
  }

  void Localization::GSCr_LocalizeText()
  {
    if (Game::Scr_GetNumParam() != 1)
    {
      Game::Scr_Error("GSCr_LocalizeText: missing key!");
      return;
    }

    const char* str = Game::Scr_GetString(0);

    const auto result = Game::UI_SafeTranslateString(str);

    Game::Scr_AddString(result);
  }

  void Localization::GSCr_LocalizeGametype()
  {
    if (Game::Scr_GetNumParam() != 1)
    {
      Game::Scr_Error("GSCr_LocalizeGametype: missing gametype!");
      return;
    }

    const char* gametype = Game::Scr_GetString(0);

    const auto result = Game::UI_GetGameTypeDisplayName(gametype);

    Game::Scr_AddString(result);
  }


  namespace
  {
    // Return the next line of a string file, without its trailing space and
    // its comment, which starts at a "//" outside of quotes.
    //
    // Note that the line itself ends at a '\n' and the next one starts past
    // any '\r' and '\n' that follow.
    //
    std::string_view ReadLine(std::string_view& text)
    {
      auto line (text.substr(0, text.find('\n')));
      text.remove_prefix(line.size());

      while (!text.empty() && (text.front() == '\r' || text.front() == '\n'))
      {
        text.remove_prefix(1);
      }

      auto trim ([&line]
      {
        while (!line.empty() && std::isspace(static_cast<unsigned char>(line.back())))
        {
          line.remove_suffix(1);
        }
      });

      trim();

      auto quoted (false);
      for (std::size_t i (0); i + 1 < line.size(); ++i)
      {
        if (line[i] == '"')
        {
          quoted = !quoted;
        }
        else if (!quoted && line[i] == '/' && line[i + 1] == '/')
        {
          line = line.substr(0, i);
          trim();
          break;
        }
      }

      return line;
    }

    // If the line starts with the keyword, in any case, skip it and the
    // blanks after it, and return true.
    //
    // Note that the game doesn't check that the keyword ends there.
    //
    bool ReadKeyword(std::string_view& line, std::string_view keyword)
    {
      if (line.size() < keyword.size() || _strnicmp(line.data(), keyword.data(), keyword.size()) != 0)
      {
        return false;
      }

      line.remove_prefix(keyword.size());

      while (!line.empty() && (line.front() == ' ' || line.front() == '\t'))
      {
        line.remove_prefix(1);
      }

      return true;
    }

    // Return the value that follows a keyword, without its blanks and
    // quotes.
    //
    std::string ReadValue(std::string_view line)
    {
      while (!line.empty() && (line.front() == ' ' || line.front() == '\t'))
      {
        line.remove_prefix(1);
      }

      if (!line.empty() && line.front() == '"')
      {
        line.remove_prefix(1);
      }

      while (!line.empty() && (line.back() == ' ' || line.back() == '\t'))
      {
        line.remove_suffix(1);
      }

      if (!line.empty() && line.back() == '"')
      {
        line.remove_suffix(1);
      }

      return std::string(line);
    }

    // Return true if each "&&N" placeholder in the string is a digit from 1
    // to 9 used once, which is what the game's formatting expects.
    //
    bool IsFormatValid(std::string_view value)
    {
      std::array<bool, 9> used{};

      for (auto i (value.find("&&")); i != std::string_view::npos; i = value.find("&&", i + 3))
      {
        const auto digit (i + 2 < value.size() ? value[i + 2] : '\0');

        if (digit < '1' || digit > '9' || std::exchange(used[digit - '1'], true))
        {
          return false;
        }
      }

      return true;
    }
  }

  std::optional<std::string> Localization::LoadStringFile(const std::string& path, bool forceEnglish)
  {
    FileSystem::File file(path);

    if (!file.exists())
    {
      return std::format("Unable to load \"{}\"!", path);
    }

    std::string_view text(file.getBuffer());
    std::string reference;
    auto ended (false);

    // Parse one line, returning the error if there is one.
    //
    auto parse ([&](std::string_view line) -> std::optional<std::string>
    {
      if (ReadKeyword(line, "VERSION"))
      {
        if (const auto version = std::atol(ReadValue(line).c_str()); version != 1)
        {
          return std::format("Unexpected version number {}, expecting {}!", version, 1);
        }
      }
      else if (ReadKeyword(line, "CONFIG") || ReadKeyword(line, "FILENOTES") || ReadKeyword(line, "NOTES") || ReadKeyword(line, "FLAGS"))
      {
      }
      else if (ReadKeyword(line, "REFERENCE"))
      {
        reference = ReadValue(line);
      }
      else if (ReadKeyword(line, "ENDMARKER"))
      {
        ended = true;
      }
      else if (line.size() >= 5 && _strnicmp(line.data(), "LANG_", 5) == 0)
      {
        if (reference.empty())
        {
          return "Error parsing file: Unexpected \"LANG_\"";
        }

        line.remove_prefix(5);

        const auto language (line.substr(0, std::min(line.find_first_of(" \t"), line.size())));
        line.remove_prefix(language.size());

        auto string (ReadValue(line));
        Utils::String::Replace(string, "\\n", "\n");

        if (!IsFormatValid(string))
        {
          return std::format("Illegal string format \"{}\"", string);
        }

        const auto english (language.size() == 7 && _strnicmp(language.data(), "english", 7) == 0);

        if (english || !forceEnglish)
        {
          Set(reference, string);
        }
      }
      else
      {
        return std::format("Unknown keyword at linestart: \"{}\"", line);
      }

      return std::nullopt;
    });

    while (!text.empty())
    {
      if (const auto line = ReadLine(text); !line.empty())
      {
        if (const auto error = parse(line))
        {
          return std::format("{} in {}", *error, path);
        }
      }
    }

    if (!ended)
    {
      return std::format("Truncated file, failed to find \"{}\" at file end!", "ENDMARKER");
    }

    return std::nullopt;
  }

  void Localization::ListStringFiles(const std::string& path, std::vector<std::string>& files)
  {
    for (const auto& folder : FileSystem::GetFileList(path, "/"))
    {
      if (!folder.empty() && folder.front() != '.')
      {
        ListStringFiles(path + "/" + folder, files);
      }
    }

    for (const auto& name : FileSystem::GetFileList(path, "str"))
    {
      files.push_back(path + "/" + name);
    }
  }

  void Localization::SEH_UpdateLanguageInfo_Hk()
  {
    SEH_UpdateLanguageInfo_Original();

    const auto* language (*reinterpret_cast<const Game::dvar_t* const*>(0x14651EF98)); // loc_language
    const auto* forceEnglish (*reinterpret_cast<const Game::dvar_t* const*>(0x14651EFA0)); // loc_forceEnglish

    // Note that the game falls back to English if no language is available,
    // which it then doesn't load strings for.
    //
    const auto index (language->current.integer);
    const auto available (*reinterpret_cast<const int*>(0x140424020 + 0x18 * index + 8));

    if (!available)
    {
      return;
    }

    std::vector<std::string> files;
    ListStringFiles("localizedstrings", files);

    for (const auto& path : files)
    {
      if (const auto error = LoadStringFile(path, forceEnglish->current.enabled))
      {
        Logger::Print(Game::CON_CHANNEL_SYSTEM, "WARNING: Could not load localization strings for {}: {}\n", Game::SEH_GetLanguageName(index), *error);
        return;
      }
    }
  }

  Localization::Localization()
  {
    SetCredits();

    AssetHandler::OnFind(Game::XAssetType::ASSET_TYPE_LOCALIZE_ENTRY, [](Game::XAssetType, const std::string& name)
    {
      Game::XAssetHeader header = { nullptr };
      std::lock_guard _(LocalizeMutex);

      if (const auto itr = LocalizeMap.find(name); itr != LocalizeMap.end())
      {
        header.localize = itr->second;
      }

      return header;
    });

    // Resolving hook
    //
    // Note that the third copy of SEH_StringEd_GetString() is in
    // SEH_LocalizeTextMessage(), which we replace below.
    //
    Utils::Hook(0x14024D819, DB_FindLocalizeEntry_Hk, HOOK_CALL).install()->quick(); // SEH_StringEd_GetString
    Utils::Hook(0x14024D71E, DB_FindLocalizeEntry_Hk, HOOK_CALL).install()->quick(); // SE_GetString

    // Note that SEH_LocalizeTextMessage() starts with a 5-byte store of an
    // argument register to its home slot.
    //
    Utils::Hook(0x14024D0B0, SEH_LocalizeTextMessageStub, HOOK_JUMP).install()->quick();

    UseLocalization = Dvar::Register<bool>("ui_localize", true, Game::DVAR_NONE, "Use localization strings");

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
          Utils::String::Replace(value, "1", std::to_string(i)); // Pretty ugly, but it should work

          Set(key, value);
        }
      }
    });

    // Read the string files in localizedstrings, a mod's for one, whenever
    // the game reads its language.
    //
    // Note that x64 compiles out the reading of the files, which we take the
    // strings from on x86 by replacing the game's SetString(), so we read
    // them as the game does on x86. It reads them only for a language that
    // is available, with loc_forceEnglish only taking the English ones, and
    // stops at the first error.
    //
    SEH_UpdateLanguageInfo_Original = Utils::Hook::Detour(0x14024D840, 5, SEH_UpdateLanguageInfo_Hk);

    // gsc: LocalizeText(key)  (e.g. MP_MIDDLE_SQUAD)
    Components::GSC::Script::AddFunction("LocalizeText", GSCr_LocalizeText);

    // gsc: LocalizeGametype(gametype)  (e.g.: dm, war, koth)
    Components::GSC::Script::AddFunction("LocalizeGametype", GSCr_LocalizeGametype);
  }

  Localization::~Localization()
  {
    LocalizeMap.clear();
  }
}
