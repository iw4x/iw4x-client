#pragma once

#undef LoadMenuByName_Recursive

namespace Components
{
	class Menus : public Component
	{
	public:
		Menus();

		void preDestroy() override;

		static void Add(const std::string& menu);

		static std::vector<Game::menuDef_t*> LoadMenuByName_Recursive(const std::string& menu);

	private:
		static std::unordered_map<std::string, Game::menuDef_t*> MenusFromDisk;
		static std::unordered_map<std::string, Game::MenuList*> MenuListsFromDisk;

		// Those two point to the ORIGINAL reference of the menu or menu list that was overriden
		static std::unordered_map<std::string, Game::menuDef_t*> OverridenMenus;

		static std::vector<std::string> CustomIW4xMenus;

		static Utils::Memory::Allocator Allocator;

		static void PrepareToUnloadMenu(Game::menuDef_t * menu);
		static void AfterLoadedMenuFromDisk(Game::menuDef_t * menu);

		static Game::Statement_s* ReallocateExpressionLocally(Game::Statement_s* statement, bool andFree);

		static void FreeMenuListOnly(Game::MenuList* menuList);
		static void FreeMenuOnly(Game::menuDef_t * menu);
		static void FreeExpression(Game::Statement_s* statement);
		static void FreeItem(Game::itemDef_s* item);

		static void UnloadMenuFromDisk(const std::string & menuName);

		static void ReloadDiskMenus();

		static Game::XAssetHeader MenuFindHook(Game::XAssetType type, const std::string& filename);
		static Game::XAssetHeader MenuListFindHook(Game::XAssetType type, const std::string& filename);

		static void LoadScriptMenu(const char* menu, bool allowNewMenus);

		static Game::script_s* LoadMenuScript(const std::string& name, const std::string& buffer);
		static int LoadMenuSource(const std::string& name, const std::string& buffer);

		static int ReserveSourceHandle();
		static bool IsValidSourceHandle(int handle);

		static Game::menuDef_t* ParseMenu(int handle);

		static void FreeScript(Game::script_s* script);
		static void FreeMenuSource(int handle);
	};
}
