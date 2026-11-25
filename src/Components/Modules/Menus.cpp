#include "Menus.hpp"
#include "Events.hpp"
#include "FastFiles.hpp"

#include "OAT/Shim.hpp"

namespace Components
{
  namespace
  {
    // Check that the game lays the menu structures out the way OpenAssetTools
    // compiles them. This is the game's half of the check (see OAT/Shim.hpp
    // for why there are two).
    //
    namespace Layout = OAT::Layout;

    static_assert(sizeof(Game::windowDef_t) == Layout::windowDef_t);
    static_assert(sizeof(Game::menuDef_t) == Layout::menuDef_t);
    static_assert(offsetof(Game::menuDef_t, items) == Layout::menuDef_t_items);
    static_assert(sizeof(Game::itemDef_s) == Layout::itemDef_s);
    static_assert(offsetof(Game::itemDef_s, parent) == Layout::itemDef_s_parent);
    static_assert(offsetof(Game::itemDef_s, typeData) == Layout::itemDef_s_typeData);
    static_assert(sizeof(Game::Statement_s) == Layout::Statement_s);
    static_assert(offsetof(Game::Statement_s, supportingData) == Layout::Statement_s_supportingData);
    static_assert(sizeof(Game::ExpressionSupportingData) == Layout::ExpressionSupportingData);
    static_assert(sizeof(Game::MenuEventHandlerSet) == Layout::MenuEventHandlerSet);
    static_assert(sizeof(Game::MenuEventHandler) == Layout::MenuEventHandler);
    static_assert(sizeof(Game::ItemKeyHandler) == Layout::ItemKeyHandler);
    static_assert(sizeof(Game::MenuList) == Layout::MenuList);
    static_assert(sizeof(Game::listBoxDef_s) == Layout::listBoxDef_s);
    static_assert(sizeof(Game::multiDef_s) == Layout::multiDef_s);

    // Clip each cell of the server browser's listbox to its column.
    //
    // Item_ListBox_PaintTextElem() draws one cell of a listbox and the only
    // limit it puts on the text is the column's maxChars, which counts
    // characters with no regard to how wide they are. That was presumably
    // good enough for the lists IW4 ships, but server names come with color
    // codes and inline material icons, so in the server browser they
    // routinely run into the map names next to them.
    //
    // IW4 has no notion of clipping a cell, but its renderer does have a
    // scissor, which the UI already uses in a couple of places (see the calls
    // at 0x14025FDD7 and 0x1402688E3). So what we do is set the scissor to the
    // cell's rect, let IW4 draw the cell, and clear the scissor again. The
    // rect is in the item's coordinates and UI_SetScissor() turns it into
    // screen ones the same way IW4 does for the cell's text, which means we
    // also get to clip the icons in the icon columns.
    //
    // We only do this for the server browser, though. The menus IW4 ships
    // count on the text spilling over. The party list, for example, declares
    // its name column 16 units wide, so clipping it leaves the first two
    // letters of a name. And since the browser is the only list we know of
    // with this problem, let's keep it at that instead of extending the menu
    // syntax.
    //
    // Note that clearing the scissor turns it off altogether instead of going
    // back to whatever was there before. So if IW4 ever drew a listbox inside
    // some other scissor, whatever it drew after the listbox would no longer
    // be clipped. We don't know of a case where it does that, so let's not
    // worry about it for now.
    //
    using Item_ListBox_PaintTextElem_t = void (*) (Game::UiContext* dc,
                                                   Game::itemDef_s* item,
                                                   int index,
                                                   int column,
                                                   int max_chars,
                                                   int alignment,
                                                   float x,
                                                   float y,
                                                   float width,
                                                   float height,
                                                   bool selected);

    Item_ListBox_PaintTextElem_t Item_ListBox_PaintTextElem_Original;

    void Item_ListBox_PaintTextElem_Hk(Game::UiContext* dc,
                                       Game::itemDef_s* item,
                                       int index,
                                       int column,
                                       int max_chars,
                                       int alignment,
                                       float x,
                                       float y,
                                       float width,
                                       float height,
                                       bool selected)
    {
      if (item->parent == nullptr || std::strcmp(item->parent->window.name, "pc_join_unranked") != 0)
      {
        Item_ListBox_PaintTextElem_Original(dc, item, index, column, max_chars, alignment, x, y, width, height, selected);
        return;
      }

      const Game::rectDef_s& r (item->window.rect);

      Game::UI_SetScissor(Game::ScrPlace_GetActivePlacement(dc->localClientNum),
                          x,
                          y,
                          width,
                          height,
                          r.horzAlign,
                          r.vertAlign);

      Item_ListBox_PaintTextElem_Original(dc, item, index, column, max_chars, alignment, x, y, width, height, selected);

      Game::UI_ClearScissor();
    }
  }

  bool Menus::ReloadedWithModZone = false;
  bool Menus::UIInitialized = false;
  std::vector<std::string> Menus::ZonesAtReload;

  Dvar::Var Menus::PrintMenuDebug;

  Game::UiContext* Menus::GameUiContexts[] = {
    Game::uiContext,
    Game::cgDC // Ingame context
  };

  std::unordered_map<std::string, Game::menuDef_t*> Menus::MenusFromDisk;
  std::unordered_map<std::string, Game::MenuList*> Menus::MenuListsFromDisk;
  std::unordered_map<std::string, Game::menuDef_t*> Menus::MenusFromModZone;
  std::unordered_map<std::string, Game::menuDef_t*> Menus::MenusFromZones;
  std::unordered_map<Game::menuDef_t*, std::shared_ptr<void>> Menus::MenuFiles;

  std::unordered_map<std::string, Game::menuDef_t*> Menus::OverridenMenus;

  Utils::Memory::Allocator Menus::Allocator;

  // On x86 we parse the menu file with the game's precompiler and menu
  // parser, loading the menus of each loadmenu as we come across it. Here
  // OpenAssetTools compiles the file and whatever it loads in one go, so the
  // recursion in the name is now a bit of a misnomer.
  //
  std::vector<Game::menuDef_t*> Menus::LoadMenuByName_Recursive(const std::string& menu)
  {
    std::vector<Game::menuDef_t*> menus;

    OAT::MenuCallbacks callbacks;

    callbacks.read = [](const std::string& file) -> std::optional<std::string>
    {
      FileSystem::File f(file);

      if (!f.exists())
      {
        return std::nullopt;
      }

      return std::move(f.getBuffer());
    };

    callbacks.material = [](const std::string& name)
    {
      return Game::DB_FindXAssetHeader(Game::ASSET_TYPE_MATERIAL, name.data()).material;
    };

    callbacks.sound = [](const std::string& name)
    {
      return Game::DB_FindXAssetHeader(Game::ASSET_TYPE_SOUND, name.data()).sound;
    };

    callbacks.print = [](const OAT::Message message, const std::string& text)
    {
      switch (message)
      {
        case OAT::Message::debug:
        case OAT::Message::info:
        {
          DebugPrint("{}", text);
          break;
        }
        case OAT::Message::warning:
        {
          Logger::Warning(Game::CON_CHANNEL_UI, "{}\n", text);
          break;
        }
        case OAT::Message::error:
        {
          Logger::PrintError(Game::CON_CHANNEL_UI, "{}\n", text);
          break;
        }
      }
    };

    auto file = OAT::LoadMenuFile(menu, callbacks);

    if (file)
    {
      for (int i = 0; i < file->list->menuCount; ++i)
      {
        if (auto* menuDef = file->list->menus[i])
        {
          menus.emplace_back(menuDef);
          MenuFiles[menuDef] = file->keepalive;
        }
      }
    }

    return menus;
  }

  bool Menus::MenuAlreadyExists(const std::string& name)
  {
    for (size_t i = 0; i < ARRAYSIZE(GameUiContexts); i++)
    {
      if (Game::Menus_FindByName(GameUiContexts[i], name.data()))
      {
        return true;
      }
    }

    return false;
  }

  void Menus::LoadScriptMenu(const char* menu, bool allowNewMenus)
  {
    auto menus = LoadMenuByName_Recursive(menu);

    if (menus.empty())
    {
      Components::Logger::PrintError(Game::CON_CHANNEL_UI, "Could not load menu {}\n", menu);
      return;
    }

    // Leave the menus the mod's fastfile has alone, rather than replacing them
    // with a copy from disk: the disk would otherwise always win, so a mod
    // could not ship a menu in its mod.ff that IW4x also ships loose in its
    // own IWDs (main_text, pc_options_*, ...).
    //
    for (int i = 0; i < static_cast<int>(menus.size()); i++)
    {
      auto* modMenu = FastFiles::FindModZoneAsset(Game::ASSET_TYPE_MENU, menus[i]->window.name).menu;

      if (!modMenu)
      {
        continue;
      }

      DebugPrint("Skipped menu {} from disk, the mod's fastfile has it", menus[i]->window.name);

      // A menu that is new to the game (one of ours, like pc_options_game)
      // only gets into the UI by way of the disk. So we add the one of the
      // mod's fastfile in its place, the same as AfterLoadedMenuFromDisk()
      // would add the one from disk.
      //
      if (allowNewMenus && !MenuAlreadyExists(modMenu->window.name))
      {
        if (Game::uiContext->menuCount >= 0 && static_cast<std::size_t>(Game::uiContext->menuCount) < ARRAYSIZE(Game::uiContext->Menus))
        {
          // Keep track of it as a new menu, so that reloading takes it out
          // again (see ReloadDiskMenus()). Otherwise it would stay in the UI
          // until the next UI_Init(), even with the mod's fastfile gone.
          //
          OverridenMenus[modMenu->window.name] = nullptr;
          MenusFromModZone[modMenu->window.name] = modMenu;

          Game::uiContext->Menus[Game::uiContext->menuCount] = modMenu;
          Game::uiContext->menuCount++;
        }
        else
        {
          Logger::PrintError(Game::CON_CHANNEL_UI, "Cannot add menu {}: MAX_MENUS exceeded\n", modMenu->window.name);
        }
      }

      FreeMenuOnly(menus[i]);
      menus.erase(menus.begin() + i);
      i--;
    }

    // Also leave out the menus that our zones' menu lists already have (see
    // LoadZoneMenuLists()). A menu that is both on disk and in one of our
    // zones is presumably one that the zone was made to replace (a loose
    // copy left in an IWD, say), so we let the zone win. If a mod ever needs
    // to replace one of these from disk, then we will have to revisit this.
    //
    for (int i = 0; i < static_cast<int>(menus.size()); i++)
    {
      if (!MenusFromZones.contains(menus[i]->window.name))
      {
        continue;
      }

      DebugPrint("Skipped menu {} from disk, a zone's menu list has it", menus[i]->window.name);

      FreeMenuOnly(menus[i]);
      menus.erase(menus.begin() + i);
      i--;
    }

    if (menus.empty())
    {
      return;
    }

    if (!allowNewMenus)
    {
      // We remove every menu we loaded that is not going to override something
      for (int i = 0; i < static_cast<int>(menus.size()); i++)
      {
        const auto menuName = menus[i]->window.name;
        if (MenuAlreadyExists(menuName))
        {
          // It's an override, we keep it
        }
        else
        {
          // We are not allowed to keep this one, let's free it
          FreeMenuOnly(menus[i]);
          menus.erase(menus.begin() + i);
          i--;
        }
      }

      if (menus.empty())
      {
        return; // No overrides!
      }
    }

    // Tracking
    for (const auto& loadedMenu : menus)
    {
      // Unload previous loaded-from-disk versions of these menus, if we had any
      const std::string menuName = loadedMenu->window.name;
      if (MenusFromDisk.contains(menuName))
      {
        UnloadMenuFromDisk(menuName);
        MenusFromDisk.erase(menuName);
      }

      // Then mark them as loaded
      MenusFromDisk[menuName] = loadedMenu;

      AfterLoadedMenuFromDisk(loadedMenu);
    }


    // Allocate new menu list
    auto* newList = Allocator.allocate<Game::MenuList>();
    if (!newList)
    {
      Components::Logger::PrintError(Game::CON_CHANNEL_UI, "No more memory to allocate menu list {}\n", menu);
      return;
    }

    newList->menus = Allocator.allocateArray<Game::menuDef_t*>(menus.size());
    if (!newList->menus)
    {
      Components::Logger::PrintError(Game::CON_CHANNEL_UI, "No more memory to allocate menus for {}\n", menu);
      Allocator.free(newList);
      return;
    }

    newList->name = Allocator.duplicateString(menu);
    newList->menuCount = static_cast<int>(menus.size());

    // Copy new menu references
    for (unsigned int i = 0; i < menus.size(); ++i)
    {
      newList->menus[i] = menus[i];
    }

    // Tracking
    {
      const auto menuListName = newList->name;
      if (MenuListsFromDisk.contains(menuListName))
      {
        FreeMenuListOnly(MenuListsFromDisk[menuListName]);
      }

      DebugPrint("Loaded menuList {} at {:X} from disk",
        newList->name,
        reinterpret_cast<std::uintptr_t>(newList)
      );

      MenuListsFromDisk[menuListName] = newList;
    }
  }

  void Menus::PrepareToUnloadMenu(Game::menuDef_t* menu)
  {
    const std::string name = menu->window.name;

    bool isRemoval = OverridenMenus[name] == nullptr || !OverridenMenus.contains(name);
    Game::menuDef_t* replacement = isRemoval ? nullptr : OverridenMenus[name];

    for (size_t contextIndex = 0; contextIndex < ARRAYSIZE(Menus::GameUiContexts); contextIndex++)
    {
      const auto context = Menus::GameUiContexts[contextIndex];

      for (size_t i = 0; i < ARRAYSIZE(context->menuStack); i++)
      {
        if (context->menuStack[i] &&
          context->menuStack[i]->window.name == name)
        {
          DebugPrint("In stack - Restored menu {} ({:X} => {:X})",
            name,
            reinterpret_cast<std::uintptr_t>(context->Menus[i]),
            reinterpret_cast<std::uintptr_t>(replacement)
          );

          if (isRemoval)
          {
            if (context->menuStack[i] == menu)
            {
              context->menuStack[i] = replacement;

              for (int j = static_cast<int>(i); j < context->openMenuCount - 1; j++)
              {
                context->menuStack[j] = context->menuStack[j + 1];
              }

              // Clear the last entry, which we just shifted down. Note that
              // openMenuCount itself may be past the end of the stack.
              //
              context->menuStack[context->openMenuCount - 1] = nullptr;
              context->openMenuCount--;
              assert(context->openMenuCount >= 0);

              i--;
            }
            else
            {
              // The menu I could have overriden got loaded in the meantime - no need to delete them
              // I'm simply going to remove myself
            }
          }
          else
          {
            // Note that the menu we put back is open now, so it takes over
            // the state of ours. Otherwise it stays in the menu stack without
            // being visible (main_text, say, once we leave it to mod.ff).
            //
            replacement->window.dynamicFlags[0] = context->menuStack[i]->window.dynamicFlags[0];
            context->menuStack[i] = replacement;
          }
        }
      }

      for (int i = 0; i < context->menuCount; i++)
      {
        if (context->Menus[i] &&
          context->Menus[i]->window.name == name)
        {
          DebugPrint("In context - Restored menu {} ({:X} => {:X})",
            name,
            reinterpret_cast<std::uintptr_t>(context->Menus[i]),
            reinterpret_cast<std::uintptr_t>(replacement)
          );

          if (isRemoval)
          {
            if (context->Menus[i] == menu)
            {
              context->Menus[i] = replacement;

              for (size_t j = i; j < std::min(static_cast<std::size_t>(context->menuCount), ARRAYSIZE(context->Menus)) - 1; j++)
              {
                context->Menus[j] = context->Menus[j + 1];
              }

              // As above, but menuCount may be past the end of the array.
              //
              context->Menus[context->menuCount - 1] = nullptr;
              context->menuCount--;

              i--;

              assert(context->menuCount >= 0);
            }
            else
            {
              // The menu I could have overriden got loaded in the meantime - no need to delete them
              // I'm simply going to remove myself
            }
          }
          else
          {
            context->Menus[i] = replacement;
          }
        }
      }
    }

    if (OverridenMenus.contains(name))
    {
      OverridenMenus.erase(name);
    }
  }

  void Menus::AfterLoadedMenuFromDisk(Game::menuDef_t* menu)
  {
    DebugPrint("Loaded menu {} at {:X} from disk",
      menu->window.name,
      reinterpret_cast<std::uintptr_t>(menu)
    );

    InstallMenu(menu);
  }

  // Put the menu into the UI in place of each menu of the same name that the
  // UI has, the open ones included, and after the other menus if there is no
  // such menu. Save what it replaces in OverridenMenus, which is where
  // PrepareToUnloadMenu() looks to put it back.
  //
  void Menus::InstallMenu(Game::menuDef_t* menu)
  {
    const std::string name = menu->window.name;

    bool overrode = false;

    for (size_t contextIndex = 0; contextIndex < ARRAYSIZE(Menus::GameUiContexts); contextIndex++)
    {
      const auto context = Menus::GameUiContexts[contextIndex];

      for (size_t i = 0; i < ARRAYSIZE(context->menuStack); i++)
      {
        if (context->menuStack[i] &&
          context->menuStack[i]->window.name == name)
        {
          if (OverridenMenus.contains(name))
          {
            assert(OverridenMenus[name] == context->menuStack[i]);
          }
          else
          {
            OverridenMenus[name] = context->menuStack[i];
          }

          context->menuStack[i] = menu;

          DebugPrint("In stack - Overrode menu {} ({:X} => {:X})",
            name,
            reinterpret_cast<std::uintptr_t>(OverridenMenus[name]),
            reinterpret_cast<std::uintptr_t>(menu)
          );
        }
      }

      for (int i = 0; i < context->menuCount; i++)
      {
        if (context->Menus[i] &&
          context->Menus[i]->window.name == name)
        {
          if (OverridenMenus.contains(name))
          {
            assert(OverridenMenus[name] == context->Menus[i]);
          }
          else
          {
            OverridenMenus[name] = context->Menus[i];
          }

          context->Menus[i] = menu;

          DebugPrint("In context - Overrode menu {} ({:X} => {:X})",
            name,
            reinterpret_cast<std::uintptr_t>(OverridenMenus[name]),
            reinterpret_cast<std::uintptr_t>(menu)
          );

          overrode = true;
        }
      }
    }

    if (!overrode)
    {
      // Make sure there is room for it. This is the same limit (MAX_MENUS)
      // as UI_AddMenu() enforces.
      //
      if (Game::uiContext->menuCount < 0 || static_cast<std::size_t>(Game::uiContext->menuCount) >= ARRAYSIZE(Game::uiContext->Menus))
      {
        Logger::PrintError(Game::CON_CHANNEL_UI, "Cannot add menu {}: MAX_MENUS exceeded\n", name);
        return;
      }

      // A brand new menu! How fancy!
      // We add it to the standard uiContext
      OverridenMenus[name] = nullptr;
      Game::uiContext->Menus[Game::uiContext->menuCount] = menu;
      Game::uiContext->menuCount++;
    }
  }

  // Put the menus of our zones' menu lists into the UI.
  //
  // IW4 only adds a menu list to the UI by name: UI_Init() adds code.txt,
  // menus.txt, and patch_mp_menus.txt, and once a match loads, ingame.txt
  // and the HUD lists follow. Our zones can replace a menu that these lists
  // have, since we let them override stock menus (see
  // AssetHandler::ShouldOverrideAsset()), but they can't add one. Adding the
  // menu to a copy of menus.txt in our zone doesn't work: our copy loads
  // after IW4's, yet the database keeps IW4's list, even though
  // ShouldOverrideAsset() asks for menu lists to be overridden as well. We
  // haven't found out why. So a menu that IW4 doesn't have goes into a list
  // that our zone names after itself (ui_mp/iw4x_localized_ui_mp.txt, say),
  // and these are the lists we go through here.
  //
  // Note that a zone also has a menu list for each .menu file it was built
  // from, named after the file (ui_mp/main.menu, say). The stock zones have
  // hundreds of these, the in-game script menus among them, which we don't
  // want in the UI on the main menu. So we only take the .txt lists of the
  // zones whose name starts with iw4x_, and skip IW4's names.
  //
  // Note also that our menus replace the ones of the same name instead of
  // going after them, the way UI_AddMenuList() would add them. The open
  // menu script command opens the first menu of a name that the UI has, so
  // the one IW4 added would always win.
  //
  void Menus::LoadZoneMenuLists()
  {
    static constexpr std::array<std::string_view, 6> own
    {
      "ui_mp/code.txt",
      "ui_mp/menus.txt",
      "ui_mp/patch_mp_menus.txt",
      "ui_mp/ingame.txt",
      "ui_mp/hud_480.txt",
      "ui_mp/hud_720.txt"
    };

    std::vector<Game::MenuList*> lists;
    Game::DB_EnumXAssets(Game::ASSET_TYPE_MENULIST, [](Game::XAssetHeader header, void* data)
    {
      static_cast<std::vector<Game::MenuList*>*>(data)->push_back(header.menuList);
    }, &lists, false);

    // Sort the lists by name. The database enumerates them in the hash table
    // order, and if two lists have a menu of the same name, then the order
    // decides which one we take (see below).
    //
    std::ranges::sort(lists, [](const Game::MenuList* x, const Game::MenuList* y)
    {
      return std::strcmp(x->name, y->name) < 0;
    });

    // Return the index of the zone that the active asset of the specified
    // type and name comes from, and -1 if there is no such asset.
    //
    const auto zoneOf = [](Game::XAssetType type, const char* name)
    {
      Game::Sys_LockRead(Game::db_hashCritSect);
      const auto* entry = Game::DB_FindXAssetEntry(type, name);
      const int zone = entry ? static_cast<unsigned char>(entry->zoneIndex) : -1;
      Game::Sys_UnlockRead(Game::db_hashCritSect);

      return zone;
    };

    for (const auto* list : lists)
    {
      if (!list->name                                                   ||
          !std::string_view(list->name).ends_with(".txt")               ||
          std::ranges::find(own, std::string_view(list->name)) != own.end())
      {
        continue;
      }

      const int zone = zoneOf(Game::ASSET_TYPE_MENULIST, list->name);

      if (zone < 0 || !std::string_view(Game::g_zones[zone].name).starts_with("iw4x_"))
      {
        continue;
      }

      DebugPrint("Loading menu list {} from zone {}", list->name, Game::g_zones[zone].name);

      for (int i = 0; i < list->menuCount; i++)
      {
        auto* menu = list->menus[i];

        if (!menu || !menu->window.name)
        {
          continue;
        }

        // If an earlier list already had a menu of this name, then keep that
        // one. We don't expect two of our lists to share a menu, so we warn
        // about it.
        //
        if (MenusFromZones.contains(menu->window.name))
        {
          Logger::Warning(Game::CON_CHANNEL_UI, "Menu {} of menu list {} is already in another menu list, skipping\n", menu->window.name, list->name);
          continue;
        }

        MenusFromZones[menu->window.name] = menu;
        InstallMenu(menu);
      }
    }
  }

  // Return the loaded zones whose menus we put into the UI ourselves, that
  // is, mod.ff and ours, in the order they were loaded.
  //
  std::vector<std::string> Menus::OurZones()
  {
    auto zones = Game::DB_GetLoadedZones();

    std::erase_if(zones, [](const std::string& zone)
    {
      return zone != "mod" && !zone.starts_with("iw4x_");
    });

    return zones;
  }

  void Menus::FreeMenuListOnly(Game::MenuList* menuList)
  {
    DebugPrint("Freeing only menuList {} at {:X}",
      menuList->name,
      reinterpret_cast<std::uintptr_t>(menuList)
    );

    Allocator.free(menuList->name);
    Allocator.free(menuList->menus);
    Allocator.free(menuList);
  }

  // On x86 this is where we free what the game's parser allocated for the
  // menu. OpenAssetTools owns the memory of what it compiles, though, so all
  // we do is let go of our share of the file the menu came from.
  //
  void Menus::FreeMenuOnly(Game::menuDef_t* menu)
  {
    DebugPrint("Freeing only menu {} at {:X}",
      menu->window.name,
      reinterpret_cast<std::uintptr_t>(menu)
    );

    MenuFiles.erase(menu);
  }

  void Menus::UnloadMenuFromDisk(const std::string& menuName)
  {
    const auto menu = MenusFromDisk[menuName];
    PrepareToUnloadMenu(menu);
    FreeMenuOnly(menu);
    MenusFromDisk.erase(menuName);
  }

  // This is fired up on Vid_restart / filesystem restart, like changing mod
  void Menus::ReloadDiskMenus_OnUIInitialization()
  {
    // At this point, this contains garbage data only!
    // This is because UI_Init does a MemSet on sharedUIInfo
    //  which contains the whole UIContext, and that destroys all pointers there
    // The game doesn't care about memory because it uses LargeLocalReset to free it all.. from what i've seen
    OverridenMenus.clear();
    MenusFromModZone.clear();
    UIInitialized = true;

    // Note that on x86 we also clear our copy of the game's supporting data
    // here. OpenAssetTools compiles the supporting data of each file along
    // with its menus, so we have none of our own.
    //

    // Free the CGDC - The game doesn't do it, but it _should_
    // Otherwise it's full of weird garbage. It's never used until CGame starts anyway!
    {
      // At this point our menus are already tracked, so we can still free
      // them, and the game frees the HUD menus in bulk.
      //
      Game::cgDC->menuCount = 0;
    }

    // And so this will only free our stuff anyway
    ReloadDiskMenus();
  }


  // This is fired up _right before the game starts_, we need to do it once again to load "ingame" menus that we might have skipped prior
  void Menus::ReloadDiskMenus_OnCGameStart()
  {
    ReloadDiskMenus();
  }

  void Menus::ReloadDiskMenus()
  {
    const auto connectionState = Game::CL_GetLocalClientConnectionState(0);

    // We only allow non-menulist menus when we're ingame
    // Otherwise we load a ton of ingame-only menus that are glitched on main_text
    const bool allowStrayMenus = connectionState > Game::connstate_t::CA_DISCONNECTED
      && Game::CL_IsCgameInitialized();

    DebugPrint("Reloading disk menus...");

    // Step 1: unload everything
    const auto listsFromDisk = MenuListsFromDisk;
    for (const auto& menuList : listsFromDisk)
    {
      FreeMenuListOnly(menuList.second);
      MenuListsFromDisk.erase(menuList.first);
    }

    const auto menusFromDisk = MenusFromDisk;
    for (const auto& element : menusFromDisk)
    {
      UnloadMenuFromDisk(element.first);
    }

    // The menus of the mod's fastfile we added go the same way, except that
    // the fastfile keeps their memory.
    //
    const auto menusFromModZone = MenusFromModZone;
    for (const auto& element : menusFromModZone)
    {
      PrepareToUnloadMenu(element.second);
      MenusFromModZone.erase(element.first);
    }

    // Take the menus of our zones' menu lists out of the UI as well.
    //
    const auto menusFromZones = MenusFromZones;
    for (const auto& element : menusFromZones)
    {
      PrepareToUnloadMenu(element.second);
      MenusFromZones.erase(element.first);
    }

#if DEBUG
    if (Allocator.empty() || allowStrayMenus)
    {
      // good
    }
    else
    {
      __debugbreak();
      Logger::Print("Warning - menu leak? Expected allocator to be empty after reload, but it's not!\n");
    }
#endif

    if (OverridenMenus.empty())
    {
      // good
    }
    else
    {
      Logger::Print("Warning - menu leak? Expected overriden menus to be empty after reload, but they're not!\n");
      OverridenMenus.clear();
    }

    // Step 2: load everything. Do our zones' menu lists first, since
    // LoadScriptMenu() looks at MenusFromZones to tell which of the menus on
    // disk to skip.
    //
    LoadZoneMenuLists();

    {
      const auto menus = FileSystem::GetFileList("ui_mp", "menu", Game::FS_LIST_ALL);

      // Load standalone menus
      for (const auto& filename : menus)
      {
        const std::string fullPath = std::format("ui_mp\\{}", filename);

        LoadScriptMenu(fullPath.c_str(), allowStrayMenus);
      }

      if (allowStrayMenus)
      {
        const auto scriptmenus = FileSystem::GetFileList("ui_mp\\scriptmenus", "menu", Game::FS_LIST_ALL);

        // Load standalone menus
        for (const auto& filename : scriptmenus)
        {
          const std::string fullPath = std::format("ui_mp\\scriptmenus\\{}", filename);

          LoadScriptMenu(fullPath.c_str(), allowStrayMenus);
        }
      }
    }

    {
      const auto menuLists = FileSystem::GetFileList("ui_mp", "txt", Game::FS_LIST_ALL);

      // Load menu list
      for (const auto& filename : menuLists)
      {
        const std::string fullPath = std::format("ui_mp\\{}", filename);
        LoadScriptMenu(fullPath.c_str(), true);
      }
    }

    // If the mod's fastfile hasn't finished loading yet, which is usually the
    // case when the UI comes up along with it, we couldn't tell which menus it
    // has. So we do this all over again once it has (see the constructor).
    //
    ReloadedWithModZone = FastFiles::ModZoneIndex() >= 0 && FastFiles::Ready();

    // Remember which of our zones we saw. UI_Init() doesn't wait for the
    // database to finish loading, so one of our zones may still be loading
    // when the UI comes up, and its menus would then be missing until the
    // next reload (see the constructor).
    //
    ZonesAtReload = OurZones();

    // Step 3 - Keep supporting data around

    // Debug-only check
    CheckMenus();
  }

  void Menus::CheckMenus()
  {
#if DEBUG
    // Give a hand to the poor programmer there

    {
      // Uniqueness check - each unique menu should have a unique name for this whole circus to run
      std::unordered_map<std::string, void*> names{};

      // Note that on x86 we also check here that our copy of the game's
      // supporting data stays in sync with it and that our menus use it. With
      // OpenAssetTools each file gets supporting data of its own, so there is
      // no copy to keep in sync. Whether that ever bites us (say, with a menu
      // that relies on the game's supporting data) we don't know yet.
      //
      for (size_t contextIndex = 0; contextIndex < ARRAYSIZE(Menus::GameUiContexts); contextIndex++)
      {
        const auto context = Menus::GameUiContexts[contextIndex];

        for (size_t i = 0; i < ARRAYSIZE(context->Menus); i++)
        {
          if (context->Menus[i] && static_cast<int>(i) < context->menuCount)
          {
            const auto name = context->Menus[i]->window.name;

            if (names.contains(name))
            {
              if (names[name] != context->Menus[i])
              {
                assert(false && "Two menus were loaded with the same name!");
              }
              else
              {
                // This behaviour is actually normal in the basegame
              }
            }
            else
            {
              names[name] = context->Menus[i];
            }
          }
          else
          {
            assert(static_cast<int>(i) >= context->menuCount && "Unexpected NULL data where the game expects a menu!");
          }
        }
      }
    }
#endif
  }

  Menus::Menus()
  {
    if (Dedicated::IsEnabled()) return;

    // The stock ASSET_TYPE_MENU clone handler copies runtime state from the existing menu to its
    // replacement, assuming both menus have identical item layouts. Disable it to prevent state
    // from being copied between unrelated items when the layouts differ.
    Utils::Hook::Set<Game::DB_DynamicCloneXAssetHandler_t>(
      &Game::DB_DynamicCloneXAssetHandler[Game::ASSET_TYPE_MENU], nullptr);

    // Clip the server browser's cells to their columns (see
    // Item_ListBox_PaintTextElem_Hk() for details).
    //
    // Note that the first 5 bytes, which the detour moves into the
    // trampoline, are mov rax, rsp followed by two pushes. One would expect
    // trouble here since IW4 builds the rest of its frame from rax, but the
    // trampoline runs that mov with rsp exactly where IW4 would have it, so
    // we end up with the same rax.
    //
    Item_ListBox_PaintTextElem_Original = Utils::Hook::Detour(0x14025F4A0, 5, &Item_ListBox_PaintTextElem_Hk);

    Components::Events::OnCGameInit(ReloadDiskMenus_OnCGameStart);
    Components::Events::AfterUIInit(ReloadDiskMenus_OnUIInitialization);

    // Reload once more when the mod's fastfile finishes loading, so that we
    // can tell which menus it has (see LoadScriptMenu()), and whenever our
    // zones changed since the last reload, so that their menu lists get into
    // the UI. The database may tell us on the render thread, so we reload on
    // the main one.
    //
    Events::AfterDatabaseLoaded([]
    {
      Scheduler::Once([]
      {
        if (!UIInitialized || !FastFiles::Ready())
        {
          return;
        }

        if (!ReloadedWithModZone && FastFiles::ModZoneIndex() >= 0)
        {
          DebugPrint("The mod's fastfile finished loading, reloading menus");
          ReloadDiskMenus();
        }
        else if (OurZones() != ZonesAtReload)
        {
          DebugPrint("Our zones changed since the last reload, reloading menus");
          ReloadDiskMenus();
        }
      }, Scheduler::Pipeline::MAIN);
    });

    Components::Scheduler::Once([]() {
      PrintMenuDebug = Dvar::Register<bool>("g_log_menu_allocations", false, Game::DVAR_SAVED, "Prints all menu allocations and swapping in the console");
      }, Components::Scheduler::Pipeline::MAIN);

    // Hunk size (stock is 0xA00000). First raised to 0xB00000 for heavy-loaded menus. Large
    // ported maps also exhaust it once PMem is big enough to let them get that far, dying on
    // allocations of a few hundred bytes: "Hunk_AllocAlign failed on 380 bytes (total 11 MB...)".
    // Like PMem, Com_InitHunkMemory reserves this and commits on demand, so the cost is
    // address space rather than memory.
    Utils::Hook::Replace32(0x14027D854, 5, 0xA00000, 0x4000000); // Com_InitHunkMemory, 64 MiB
    Utils::Hook::Replace32(0x14027D859, 11, 0xA00000, 0x4000000);

    // Don't open connect menu twice - it gets stuck!
    Utils::Hook::Nop(0x14026AC4B, 5);

    // Note that on x86 we also disable the two tokens of ItemParse_rect that
    // IW4 added. x64 compiles them out along with the rest of the parser, so
    // there is nothing to disable.
    //

    // make Com_Error and similar go back to main_text instead of menu_xboxlive.
    Utils::Hook::SetString(0x140379490, "main_text");

    Command::Add("openmenu", [](const Command::Params* params)
      {
        if (params->size() != 2)
        {
          Logger::Print("USAGE: openmenu <menu name>\n");
          return;
        }

        // Not quite sure if we want to do this if we're not ingame, but it's only needed for ingame menus.
        if ((*Game::cl_ingame)->current.enabled)
        {
          Game::Key_SetCatcher(0, Game::KEYCATCH_UI);
        }

        const char* menuName = params->get(1);

        Game::Menus_OpenByName(Game::uiContext, menuName);
      });

    // Complete the argument of openmenu with the menus it can open,
    // that is, the ones in the UI right now. These change with every
    // menu reload, so we collect them each time the console asks. Could
    // we keep a list and update it on reload instead? We could, but
    // that would only save a few hundred compares per frame while
    // typing, which doesn't seem worth the trouble of keeping the two
    // in sync.
    //
    // Note that the UI may have two menus of the same name since IW4
    // appends the menu lists it loads rather than replace the menus it
    // has. Presumably Menus_OpenByName() opens the first one, so we
    // only list each name once.
    //
    Game::Cmd_SetArgumentCompletion("openmenu", []
    {
      auto menus = std::span(Game::uiContext->Menus, Game::uiContext->menuCount)
                 | std::views::filter([](const Game::menuDef_t* m) { return m && m->window.name; })
                 | std::views::transform([](const Game::menuDef_t* m) { return std::string(m->window.name); });

      std::vector<std::string> names(menus.begin(), menus.end());

      std::ranges::sort(names, [](const std::string& x, const std::string& y)
      {
        return _stricmp(x.c_str(), y.c_str()) < 0;
      });

      const auto [first, last] = std::ranges::unique(names, [](const std::string& x, const std::string& y)
      {
        return _stricmp(x.c_str(), y.c_str()) == 0;
      });

      names.erase(first, last);

      return names;
    });
  }
}
