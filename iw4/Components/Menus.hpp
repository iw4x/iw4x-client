#define MAX_SOURCEFILES	64

namespace Components
{
	class Menus : public Component
	{
	public:
		Menus();
		~Menus();
		const char* GetName() { return "Menus"; };

	private:
		static std::vector<Game::menuDef_t*> MenuList;
		static std::vector<Game::MenuList*> MenuListList;

		static Game::XAssetHeader MenuFileLoad(Game::XAssetType type, const char* filename);
		static Game::MenuList* LoadMenuList(Game::MenuList* menuList);
		static Game::menuDef_t* LoadMenu(Game::menuDef_t* menudef);

		static Game::script_t* LoadMenuScript(std::string buffer);
		static int LoadMenuSource(std::string buffer);
		static int ReserveSourceHandle();

		static void FreeMenuScript(Game::script_t* script);
		static void FreeMenuSource(int handle);

		static void FreeMenuList(Game::MenuList* menuList);
		static void FreeMenu(Game::menuDef_t* menudef);

		static void FreeEverything();
	};
}
