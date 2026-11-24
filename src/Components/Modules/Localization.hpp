#pragma once

namespace Components
{
  class Localization : public Component
  {
  public:
    Localization();
    ~Localization();

    static void Set(const std::string& psLocalReference, const std::string& psNewString);
    static const char* Get(const char* key);

    static std::optional<std::string> PrefixOverride;
    static void ParseOutput(const std::function<void(Game::LocalizeEntry*)>& callback);

    static const char* LocalizeMapName(const char* mapName);

  private:
    static std::recursive_mutex LocalizeMutex;
    static std::unordered_map<std::string, Game::LocalizeEntry*> LocalizeMap;

    // The keys of LocalizeMap whose string we last read from a string file
    // rather than set ourselves (see Get()).
    //
    static std::unordered_set<std::string> FileStrings;

    static Dvar::Var UseLocalization;

    static void Set(const std::string& psLocalReference, const std::string& psNewString, bool fromFile);

    static std::function<void(Game::LocalizeEntry*)> ParseCallback;

    static Game::XAssetHeader DB_FindLocalizeEntry_Hk(Game::XAssetType type, const char* key);

    static void(*SEH_UpdateLanguageInfo_Original)();
    static std::optional<std::string> LoadStringFile(const std::string& path, bool forceEnglish);
    static void ListStringFiles(const std::string& path, std::vector<std::string>& files);
    static void SEH_UpdateLanguageInfo_Hk();

    static void SaveParseOutput(Game::LocalizeEntry* asset);

    static void SetCredits();

    static const char* SEH_LocalizeTextMessageStub(const char* pszInputBuffer, const char* pszMessageType, Game::msgLocErrType_t errType);

    static void GSCr_LocalizeText();
    static void GSCr_LocalizeGametype();
  };
}
