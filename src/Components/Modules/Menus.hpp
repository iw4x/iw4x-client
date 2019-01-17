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
        static Game::MenuList* LoadMenuList(const std::string& file);
        static std::vector<std::pair<bool, Game::menuDef_t*>> LoadMenu(const std::string& file);

	private:
		static std::unordered_map<std::string, Game::menuDef_t*> DiskMenuList;
		static std::unordered_map<std::string, Game::MenuList*> DiskMenuListList;
        static std::unordered_map<std::string, std::pair<int, Game::menuDef_t*>> Menus::UiContextMenus;

        // Loading
        static int ReserveSourceHandle();
        static bool IsValidSourceHandle(int handle);
        static Game::menuDef_t* ParseMenu(int handle);
        static Game::script_t* LoadMenuScript(const std::string& name, const std::string& buffer);
        static int LoadMenuSource(const std::string& name, const std::string& buffer);

        // Freeing
		static void FreeMenuSource(int handle);
		static void FreeDiskMenuList(Game::MenuList* menuList);
		static void FreeDiskMenu(Game::menuDef_t* menudef);

        // Etc.
		static bool IsMenuVisible(Game::UiContext *dc, Game::menuDef_t *menu);

        // Intercept Asset Find Calls
        static Game::XAssetHeader MenuFindHook(Game::XAssetType type, const std::string& filename);
        static Game::XAssetHeader MenuListFindHook(Game::XAssetType type, const std::string& filename);

        // Manage menus in uiContext
        enum MenuContextPriority
        {
            PRIORITY_BUILTIN = 0,
            PRIORITY_IW4X = 1,
            PRIORITY_MOD = 2,
        };
        static std::pair<int, Game::menuDef_t*> FindMenuInContext(Game::UiContext* ctx, const std::string& name);
        static void AddMenuToContext(Game::UiContext* ctx, int priority, Game::menuDef_t* menu);
        static void ReplaceMenuInContext(Game::UiContext* ctx, int priority, Game::menuDef_t* menu);
        static void AddMenuListToContext(Game::UiContext* ctx, Game::MenuList* list, int close);
        static void ResetContextHook(int a1);

		// Ugly!
		static int KeywordHash(char* key);
	};
}
