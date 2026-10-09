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

		// True when the loaded translation is in a right-to-left language
		static bool IsRtlTranslation();

		// Value of loc_translation, empty for the game's own text
		static std::string GetTranslationName();

	private:
		static std::recursive_mutex LocalizeMutex;
		static std::unordered_map<std::string, Game::LocalizeEntry*> LocalizeMap;
		static Dvar::Var UseLocalization;
		static Dvar::Var Translation;
		static std::atomic_bool RtlTranslation;

		static std::function<void(Game::LocalizeEntry*)> ParseCallback;

		static void __stdcall SetStringStub(const char* psLocalReference, const char* psNewString, int bSentenceIsEnglish);

		static void SaveParseOutput(Game::LocalizeEntry* asset);

		static void SetCredits();

		static void LoadTranslation();
		static void DumpStrings();

		static const char* SEH_LocalizeTextMessageStub(const char* pszInputBuffer, const char* pszMessageType, Game::msgLocErrType_t errType);

		static void GSCr_LocalizeText();
		static void GSCr_LocalizeGametype();
	};
}
