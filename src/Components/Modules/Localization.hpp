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
		static std::mutex LocalizeMutex;
		static Utils::Memory::Allocator MemAllocator;
		static std::map<std::string, Game::LocalizedEntry*> LocalizeMap;
		static std::map<std::string, Game::LocalizedEntry*> TempLocalizeMap;
		static Dvar::Var UseLocalization;

		static void __stdcall SetStringStub(const char* key, const char* value, bool isEnglish);
		static DWORD SELoadLanguageStub();
	};
}
