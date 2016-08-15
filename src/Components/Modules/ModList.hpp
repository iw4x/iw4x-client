namespace Components
{
	class ModList : public Component
	{
	public:
		ModList();
		~ModList();

#ifdef DEBUG
		const char* GetName() { return "ModList"; };
#endif

		static void RunMod(std::string mod);

	private:
		static std::vector<std::string> Mods;
		static unsigned int CurrentMod;

		static bool HasMod(std::string modName);

		static unsigned int GetItemCount();
		static const char* GetItemText(unsigned int index, int column);
		static void Select(unsigned int index);
		static void UIScript_LoadMods();
		static void UIScript_RunMod();
		static void UIScript_ClearMods();
	};
}