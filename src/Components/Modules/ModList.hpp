#pragma once

namespace Components
{
	class ModList : public Component
	{
	public:
		ModList();

		static void RunMod(const std::string& mod);

	private:
		static std::vector<std::string> Mods;
		static unsigned int CurrentMod;

		static bool HasMod(const std::string& modName);

		static unsigned int GetItemCount();
		static const char* GetItemText(unsigned int index, int column);
		static void Select(unsigned int index);
		static void UIScript_LoadMods(UIScript::Token);
		static void UIScript_RunMod(UIScript::Token);
		static void UIScript_ClearMods(UIScript::Token);
	};
}
