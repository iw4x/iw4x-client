namespace Components
{
	class Localization : public Component
	{
	public:
		Localization();
		~Localization();
		const char* GetName() { return "Localization"; };

		static void Set(std::string key, std::string value);
		static const char* Get(const char* key);

		static void SetTemp(std::string key, std::string value);
		static void ClearTemp();

	private:
		static Utils::Memory::Allocator MemAllocator;
		static std::map<std::string, Game::LocalizedEntry*> LocalizeMap;
		static std::map<std::string, Game::LocalizedEntry*> TempLocalizeMap;
		static Dvar::Var UseLocalization;

		static void __stdcall SetStringStub(const char* key, const char* value, bool isEnglish);
		static DWORD SELoadLanguageStub();
	};
}
