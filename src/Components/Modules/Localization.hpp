namespace Components
{
	class Localization : public Component
	{
	public:
		Localization();
		~Localization();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() { return "Localization"; };
#endif

		static void Set(std::string key, std::string value);
		static const char* Get(const char* key);

		static void SetTemp(std::string key, std::string value);
		static void ClearTemp();

	private:
		static std::recursive_mutex LocalizeMutex;
		static Utils::Memory::Allocator MemAllocator;
		static std::unordered_map<std::string, Game::LocalizeEntry*> LocalizeMap;
		static std::unordered_map<std::string, Game::LocalizeEntry*> TempLocalizeMap;
		static Dvar::Var UseLocalization;

		static void __stdcall SetStringStub(const char* key, const char* value, bool isEnglish);
		static void LoadLanguageStrings();
		static void SELoadLanguageStub();
	};
}
