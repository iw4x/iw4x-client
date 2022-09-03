#pragma once

namespace Components
{
	class Localization : public Component
	{
	public:
		Localization();
		~Localization();

		static void Set(const std::string& key, const std::string& value);
		static const char* Get(const char* key);

		static void SetTemp(const std::string& key, const std::string& value);
		static void ClearTemp();

	private:
		static std::recursive_mutex LocalizeMutex;
		static std::unordered_map<std::string, Game::LocalizeEntry*> LocalizeMap;
		static std::unordered_map<std::string, Game::LocalizeEntry*> TempLocalizeMap;
		static Dvar::Var UseLocalization;

		static void __stdcall SetStringStub(const char* key, const char* value, bool isEnglish);
		static void LoadLanguageStrings();
		static void SELoadLanguageStub();
		static void SetCredits();

		static const char* SEH_LocalizeTextMessageStub(const char* pszInputBuffer, const char* pszMessageType, Game::msgLocErrType_t errType);
	};
}
