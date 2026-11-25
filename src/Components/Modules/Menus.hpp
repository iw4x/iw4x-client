#pragma once

namespace Components
{
  // Note that x64 has neither the precompiler nor the menu parser that we
  // load the menus on disk with on x86. So we compile them with
  // OpenAssetTools instead (see OAT/Shim.hpp), and everything we do with them
  // after that stays as on x86.
  //
  class Menus : public Component
  {
  public:
    Menus();

    static std::vector<Game::menuDef_t*> LoadMenuByName_Recursive(const std::string& menu);

  private:
    static std::unordered_map<std::string, Game::menuDef_t*> MenusFromDisk;
    static std::unordered_map<std::string, Game::MenuList*> MenuListsFromDisk;

    // The menus of the mod's fastfile that we added to the UI in place of
    // their copy from disk (see LoadScriptMenu()). The fastfile owns their
    // memory, so all we free is their place in the UI.
    //
    static std::unordered_map<std::string, Game::menuDef_t*> MenusFromModZone;

    // The menus that LoadZoneMenuLists() put into the UI. The zones keep
    // their memory, so all we ever take back is their place in the UI.
    //
    static std::unordered_map<std::string, Game::menuDef_t*> MenusFromZones;

    // The memory of the file each menu from disk was compiled from.
    //
    // On x86 we free the menus from disk one by one, since the game's parser
    // allocates each of them on its own. OpenAssetTools, on the other hand,
    // compiles all the menus of a file into memory they share. So each menu
    // holds a share of its file, and the file goes away once we free the last
    // of them. Which means a file may well stay loaded for longer than on
    // x86, but menus are small, so it probably doesn't matter.
    //
    static std::unordered_map<Game::menuDef_t*, std::shared_ptr<void>> MenuFiles;

    static Game::UiContext* GameUiContexts[];

    static Dvar::Var PrintMenuDebug;

    // Those two point to the ORIGINAL reference of the menu or menu list that was overriden
    static std::unordered_map<std::string, Game::menuDef_t*> OverridenMenus;

    // Whether we last reloaded the menus with the mod's fastfile loaded, and
    // whether the UI is up to begin with.
    //
    static bool ReloadedWithModZone;
    static bool UIInitialized;

    // The zones of OurZones() that were loaded when we last reloaded the
    // menus.
    //
    static std::vector<std::string> ZonesAtReload;

    static Utils::Memory::Allocator Allocator;

    static bool MenuAlreadyExists(const std::string& name);

    static void PrepareToUnloadMenu(Game::menuDef_t* menu);
    static void InstallMenu(Game::menuDef_t* menu);
    static void AfterLoadedMenuFromDisk(Game::menuDef_t* menu);

    static void LoadZoneMenuLists();
    static std::vector<std::string> OurZones();

    static void FreeMenuListOnly(Game::MenuList* menuList);
    static void FreeMenuOnly(Game::menuDef_t* menu);

    static void UnloadMenuFromDisk(const std::string& menuName);

    static void ReloadDiskMenus();

    static void LoadScriptMenu(const char* menu, bool allowNewMenus);

    static void ReloadDiskMenus_OnCGameStart();
    static void ReloadDiskMenus_OnUIInitialization();

    static void CheckMenus();

    template <typename... Args>
    static void DebugPrint(const std::string_view& fmt, Args&&... args)
    {
      if (PrintMenuDebug.get<bool>())
      {
        const std::string msg = std::vformat(fmt, std::make_format_args(args...));
        const std::string preformatted = std::format("[MENUS] {:X} {}\n", std::hash<std::jthread::id>{}(std::this_thread::get_id()), msg);
        Logger::Print(preformatted);
      }
    }
  };
}
