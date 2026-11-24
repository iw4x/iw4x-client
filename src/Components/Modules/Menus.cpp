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

    // The index of the zone of the mod's fastfile, or -1 when it's not
    // loaded.
    //
    int GetModZoneIndex()
    {
      const auto index = Game::DB_GetZoneIndex("mod");
      return index > 0 ? index : -1;
    }

    // The menu of this name of the mod's fastfile, if it has one. We let such
    // menus be, rather than replacing them with a copy from disk: the disk
    // would otherwise always win, so a mod could not ship a menu in its mod.ff
    // that IW4x also ships loose in its own IWDs (main_text, pc_options_*, ...).
    //
    Game::menuDef_t* FindModZoneMenu(const char* name)
    {
      const auto modZoneIndex = GetModZoneIndex();

      if (modZoneIndex < 0)
      {
        return nullptr;
      }

      Game::menuDef_t* menu = nullptr;

      Game::Sys_LockRead(Game::db_hashCritSect);

      // Note that the entry we find may be the one of another zone that
      // overrides ours, so we follow the overrides as well.
      //
      for (auto* entry = Game::DB_FindXAssetEntry(Game::ASSET_TYPE_MENU, name); entry;
        entry = entry->nextOverride ? &Game::g_assetEntryPool[entry->nextOverride] : nullptr)
      {
        if (static_cast<unsigned char>(entry->zoneIndex) == modZoneIndex)
        {
          menu = entry->asset.header.menu;
          break;
        }
      }

      Game::Sys_UnlockRead(Game::db_hashCritSect);

      return menu;
    }
  }

  /// This variable dispenses us from the horror of having a text file in IWD containing the menus we want to load
  std::vector<std::string> Menus::CustomIW4xMenus;

  bool Menus::ReloadedWithModZone = false;
  bool Menus::UIInitialized = false;

  Dvar::Var Menus::PrintMenuDebug;

  Game::UiContext* Menus::GameUiContexts[] = {
    Game::uiContext,
    Game::cgDC // Ingame context
  };

  std::unordered_map<std::string, Game::menuDef_t*> Menus::MenusFromDisk;
  std::unordered_map<std::string, Game::MenuList*> Menus::MenuListsFromDisk;
  std::unordered_map<std::string, Game::menuDef_t*> Menus::MenusFromModZone;
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

    // Leave the menus the mod's fastfile has alone (see FindModZoneMenu()).
    //
    for (int i = 0; i < static_cast<int>(menus.size()); i++)
    {
      auto* modMenu = FindModZoneMenu(menus[i]->window.name);

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
    const std::string name = menu->window.name;

    DebugPrint("Loaded menu {} at {:X} from disk",
      name,
      reinterpret_cast<std::uintptr_t>(menu)
    );

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

          context->menuStack[i] = MenusFromDisk[name];

          DebugPrint("In stack - Overrode menu {} ({:X} => {:X})",
            name,
            reinterpret_cast<std::uintptr_t>(OverridenMenus[name]),
            reinterpret_cast<std::uintptr_t>(MenusFromDisk[name])
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

          context->Menus[i] = MenusFromDisk[name];

          DebugPrint("In context - Overrode menu {} ({:X} => {:X})",
            name,
            reinterpret_cast<std::uintptr_t>(OverridenMenus[name]),
            reinterpret_cast<std::uintptr_t>(MenusFromDisk[name])
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
      Game::uiContext->Menus[Game::uiContext->menuCount] = MenusFromDisk[name];
      Game::uiContext->menuCount++;
    }
  }

  void Menus::Add(const std::string& menu)
  {
    CustomIW4xMenus.push_back(menu);
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

    // Step 2: Load everything
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

      // "code.txt" for IW4x
      for (const auto& menuName : CustomIW4xMenus)
      {
        LoadScriptMenu(menuName.c_str(), true);
      }
    }

    // If the mod's fastfile hasn't finished loading yet, which is usually the
    // case when the UI comes up along with it, we couldn't tell which menus it
    // has. So we do this all over again once it has (see the constructor).
    //
    ReloadedWithModZone = GetModZoneIndex() >= 0 && FastFiles::Ready();

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

    Components::Events::OnCGameInit(ReloadDiskMenus_OnCGameStart);
    Components::Events::AfterUIInit(ReloadDiskMenus_OnUIInitialization);

    // Reload once more after the mod's fastfile finished loading, so that we
    // can tell which menus it has (see FindModZoneMenu()). The database may
    // tell us so on the render thread, so the reload goes to the main one.
    //
    Events::AfterDatabaseLoaded([]
    {
      Scheduler::Once([]
      {
        if (UIInitialized && !ReloadedWithModZone && GetModZoneIndex() >= 0 && FastFiles::Ready())
        {
          DebugPrint("The mod's fastfile finished loading, reloading menus");
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

    // Define custom menus here
    Add("ui_mp/changelog.menu");
    Add("ui_mp/iw4x_credits.menu");
    Add("ui_mp/menu_first_launch.menu");
    Add("ui_mp/mod_download_popmenu.menu");
    Add("ui_mp/pc_options_game.menu");
    Add("ui_mp/pc_options_gamepad.menu");
    Add("ui_mp/pc_options_gamepad_advanced.menu");
    Add("ui_mp/pc_options_gamepad_sticks.menu");
    Add("ui_mp/pc_options_gamepad_triggers.menu");
    Add("ui_mp/pc_options_gamepad_response.menu");
    Add("ui_mp/pc_options_gamepad_feedback.menu");
    Add("ui_mp/pc_options_gamepad_timing.menu");
    Add("ui_mp/pc_options_multi.menu");
    Add("ui_mp/popup_customclan.menu");
    Add("ui_mp/popup_customtitle.menu");
    Add("ui_mp/popup_friends.menu");
    Add("ui_mp/popup_servername_filter.menu");
    Add("ui_mp/resetclass.menu");
    Add("ui_mp/security_increase_popmenu.menu");
    Add("ui_mp/startup_messages.menu");
    Add("ui_mp/stats_reset.menu");
    Add("ui_mp/stats_unlock.menu");
    Add("ui_mp/stats_mod_warning.menu");
    Add("ui_mp/theater_menu.menu");
  }
}
