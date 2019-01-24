#pragma once

#define MAX_SOURCEFILES	64
#define MAX_MENUS_IN_CONTEXT 640
#undef LoadMenu

namespace Components
{
	class Menus : public Component
	{
	public:
		Menus();
		~Menus();

		static void FreeEverything();
        static void RegisterMenuLists();

        // used to load assets for zonebuilder
        static std::vector<Game::menuDef_t*> LoadMenu(const std::string& file);

	private:
		static std::unordered_map<std::string, Game::menuDef_t*> DiskMenuList;

        // Loading
        static int ReserveSourceHandle();
        static bool IsValidSourceHandle(int handle);
        static Game::menuDef_t* ParseMenu(int handle);
        static Game::script_t* LoadMenuScript(const std::string& name, const std::string& buffer);
        static int LoadMenuSource(const std::string& name, const std::string& buffer);

        // Freeing
		static void FreeMenuSource(int handle);
		static void FreeDiskMenu(Game::menuDef_t* menudef);

        // Etc.
		static bool IsMenuVisible(Game::UiContext *dc, Game::menuDef_t *menu);

        // Manage menus in uiContext
        static void AddMenuListToContext(Game::UiContext* ctx, Game::MenuList* list, int close);
        static void ResetContextHook(int a1);

		// Ugly!
		static int KeywordHash(char* key);
	};
}
