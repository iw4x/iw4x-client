namespace Components
{
	class ModList : public Component
	{
	public:
		ModList();
		~ModList();
		const char* GetName() { return "Mods"; };

	private:
		struct modInfo_t
		{
			char** mods;
			int max;
			int current;
		};

		static bool hasMod(const char* modName);

		static modInfo_t modInfo;

		static unsigned int GetItemCount();
		static const char* GetItemText(unsigned int index, int column);
		static void Select(unsigned int index);
		static void UIScript_LoadMods();
		static void UIScript_RunMod();
		static void UIScript_ClearMods();
	};
}