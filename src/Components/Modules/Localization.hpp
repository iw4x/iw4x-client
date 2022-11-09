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

	private:
		static std::recursive_mutex LocalizeMutex;
		static std::unordered_map<std::string, Game::LocalizeEntry*> LocalizeMap;
		static Dvar::Var UseLocalization;

		static void __stdcall SetStringStub(const char* key, const char* value, bool isEnglish);
		static void SetCredits();

		static const char* SEH_LocalizeTextMessageStub(const char* pszInputBuffer, const char* pszMessageType, Game::msgLocErrType_t errType);
	};
}
