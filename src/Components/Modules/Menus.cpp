#include <STDInclude.hpp>

namespace Components
{
	std::vector<std::string> Menus::CustomMenus;
	std::unordered_map<std::string, Game::menuDef_t*> Menus::MenuList;
	std::unordered_map<std::string, Game::MenuList*> Menus::MenuListList;

	int Menus::ReserveSourceHandle()
	{
		// Check if a free slot is available
		int i = 1;
		for (; i < MAX_SOURCEFILES; ++i)
		{
			if (!Game::sourceFiles[i])
				break;
		}

		if (i >= MAX_SOURCEFILES)
			return 0;

		// Reserve it, if yes
		Game::sourceFiles[i] = reinterpret_cast<Game::source_t*>(1);

		return i;
	}

	Game::script_t* Menus::LoadMenuScript(const std::string& name, const std::string& buffer)
	{
		Game::script_t* script = Game::Script_Alloc(sizeof(Game::script_t) + 1 + buffer.length());
		if (!script) return nullptr;

		strcpy_s(script->filename, sizeof(script->filename), name.data());
		script->buffer = reinterpret_cast<char*>(script + 1);

		*(script->buffer + buffer.length()) = '\0';

		script->script_p = script->buffer;
		script->lastscript_p = script->buffer;
		script->length = buffer.length();
		script->end_p = &script->buffer[buffer.length()];
		script->line = 1;
		script->lastline = 1;
		script->tokenavailable = 0;

		Game::Script_SetupTokens(script, reinterpret_cast<char*>(0x797F80));
		script->punctuations = reinterpret_cast<Game::punctuation_t*>(0x797F80);

		std::memcpy(script->buffer, buffer.data(), script->length + 1);

		script->length = Game::Script_CleanString(script->buffer);

		return script;
	}

	int Menus::LoadMenuSource(const std::string& name, const std::string& buffer)
	{
		Utils::Memory::Allocator* allocator = Utils::Memory::GetAllocator();

		int handle = Menus::ReserveSourceHandle();
		if (!Menus::IsValidSourceHandle(handle)) return 0; // No free source slot!

		Game::script_t* script = Menus::LoadMenuScript(name, buffer);

		if (!script)
		{
			Game::sourceFiles[handle] = nullptr; // Free reserved slot
			return 0;
		}

		script->next = nullptr;

		auto* source = allocator->allocate<Game::source_t>();
		if (!source)
		{
			Game::FreeMemory(script);
			return 0;
		}

		strncpy_s(source->filename, 64, "string", 64);
		source->scriptstack = script;
		source->tokens = nullptr;
		source->defines = nullptr;
		source->indentstack = nullptr;
		source->skip = 0;
		source->definehash = static_cast<Game::define_t * *>(allocator->allocate(4096));

		Game::sourceFiles[handle] = source;

		return handle;
	}

	bool Menus::IsValidSourceHandle(int handle)
	{
		return (handle > 0 && handle < MAX_SOURCEFILES && Game::sourceFiles[handle]);
	}

	int Menus::KeywordHash(char* key)
	{
		int hash = 0;

		if (*key)
		{
			int sub = 3523 - reinterpret_cast<DWORD>(key);
			do
			{
				char _chr = *key;
				hash += reinterpret_cast<DWORD>(&(key++)[sub]) * tolower(_chr);
			} while (*key);
		}

		return (static_cast<uint16_t>(hash) + static_cast<uint16_t>(hash >> 8)) & 0x7F;
	}

	Game::menuDef_t* Menus::ParseMenu(int handle)
	{
		Utils::Memory::Allocator* allocator = Utils::Memory::GetAllocator();

		auto* menu = allocator->allocate<Game::menuDef_t>();
		if (!menu) return nullptr;

		menu->items = allocator->allocateArray<Game::itemDef_s*>(512);
		if (!menu->items)
		{
			allocator->free(menu);
			return nullptr;
		}

		Game::pc_token_t token;
		if (!Game::PC_ReadTokenHandle(handle, &token) || token.string[0] != '{')
		{
			allocator->free(menu->items);
			allocator->free(menu);
			return nullptr;
		}

		while (true)
		{
			ZeroMemory(&token, sizeof(token));

			if (!Game::PC_ReadTokenHandle(handle, &token))
			{
				Game::PC_SourceError(handle, "end of file inside menu\n");
				break; // Fail
			}

			if (*token.string == '}')
			{
				break; // Success
			}

			int idx = Menus::KeywordHash(token.string);

			Game::keywordHash_t* key = Game::menuParseKeywordHash[idx];

			if (!key)
			{
				Game::PC_SourceError(handle, "unknown menu keyword %s", token.string);
				continue;
			}

			if (!key->func(menu, handle))
			{
				Game::PC_SourceError(handle, "couldn't parse menu keyword %s", token.string);
				break; // Fail
			}
		}

		Menus::OverrideMenu(menu);
		Menus::RemoveMenu(menu->window.name);
		Menus::MenuList[menu->window.name] = menu;

		return menu;
	}

	Game::MenuList* Menus::LoadCustomMenuList(const std::string& menu, Utils::Memory::Allocator* allocator)
	{
		std::vector<std::pair<bool, Game::menuDef_t*>> menus;
		FileSystem::File menuFile(menu);

		if (!menuFile.exists()) return nullptr;

		Game::pc_token_t token;
		int handle = Menus::LoadMenuSource(menu, menuFile.getBuffer());

		if (Menus::IsValidSourceHandle(handle))
		{
			while (true)
			{
				ZeroMemory(&token, sizeof(token));

				if (!Game::PC_ReadTokenHandle(handle, &token) || token.string[0] == '}')
				{
					break;
				}

				if (!_stricmp(token.string, "loadmenu"))
				{
					Game::PC_ReadTokenHandle(handle, &token);

					Utils::Merge(&menus, Menus::LoadMenu(Utils::String::VA("ui_mp\\%s.menu", token.string)));
				}

				if (!_stricmp(token.string, "menudef"))
				{
					Game::menuDef_t* menudef = Menus::ParseMenu(handle);
					if (menudef) menus.push_back({ true, menudef }); // Custom menu
				}
			}

			Menus::FreeMenuSource(handle);
		}

		if (menus.empty()) return nullptr;

		// Allocate new menu list
		Game::MenuList* list = allocator->allocate<Game::MenuList>();
		if (!list) return nullptr;

		list->menus = allocator->allocateArray<Game::menuDef_t*>(menus.size());
		if (!list->menus)
		{
			allocator->free(list);
			return nullptr;
		}

		list->name = allocator->duplicateString(menu);
		list->menuCount = menus.size();

		// Copy new menus
		for (unsigned int i = 0; i < menus.size(); ++i)
		{
			list->menus[i] = menus[i].second;
		}

		return list;
	}

	std::vector<std::pair<bool, Game::menuDef_t*>> Menus::LoadMenu(const std::string& menu)
	{
		std::vector<std::pair<bool, Game::menuDef_t*>> menus;
		FileSystem::File menuFile(menu);

		if (menuFile.exists())
		{
			Game::pc_token_t token;
			int handle = Menus::LoadMenuSource(menu, menuFile.getBuffer());

			if (Menus::IsValidSourceHandle(handle))
			{
				while (true)
				{
					ZeroMemory(&token, sizeof(token));

					if (!Game::PC_ReadTokenHandle(handle, &token) || token.string[0] == '}')
					{
						break;
					}

					if (!_stricmp(token.string, "loadmenu"))
					{
						Game::PC_ReadTokenHandle(handle, &token);

						Utils::Merge(&menus, Menus::LoadMenu(Utils::String::VA("ui_mp\\%s.menu", token.string)));
					}

					if (!_stricmp(token.string, "menudef"))
					{
						Game::menuDef_t* menudef = Menus::ParseMenu(handle);
						if (menudef) menus.push_back({ true, menudef }); // Custom menu
					}
				}

				Menus::FreeMenuSource(handle);
			}
		}

		return menus;
	}

	std::vector<std::pair<bool, Game::menuDef_t*>> Menus::LoadMenu(Game::menuDef_t* menudef)
	{
		std::vector<std::pair<bool, Game::menuDef_t*>> menus = Menus::LoadMenu(Utils::String::VA("ui_mp\\%s.menu", menudef->window.name));

		if (menus.empty())
		{
			menus.push_back({ false, menudef }); // Native menu
		}

		return menus;
	}

	Game::MenuList* Menus::LoadScriptMenu(const char* menu)
	{
		Utils::Memory::Allocator* allocator = Utils::Memory::GetAllocator();

		std::vector<std::pair<bool, Game::menuDef_t*>> menus = Menus::LoadMenu(menu);
		if (menus.empty()) return nullptr;

		// Allocate new menu list
		auto* newList = allocator->allocate<Game::MenuList>();
		if (!newList) return nullptr;

		newList->menus = allocator->allocateArray<Game::menuDef_t*>(menus.size());
		if (!newList->menus)
		{
			allocator->free(newList);
			return nullptr;
		}

		newList->name = allocator->duplicateString(menu);
		newList->menuCount = static_cast<int>(menus.size());

		// Copy new menus
		for (unsigned int i = 0; i < menus.size(); ++i)
		{
			newList->menus[i] = menus[i].second;
		}

		Menus::RemoveMenuList(newList->name);
		Menus::MenuListList[newList->name] = newList;

		return newList;
	}

	void Menus::SafeMergeMenus(std::vector<std::pair<bool, Game::menuDef_t*>>* menus, std::vector<std::pair<bool, Game::menuDef_t*>> newMenus)
	{
		// Check if we overwrote a menu
		for (auto i = menus->begin(); i != menus->end();)
		{
			// Try to find the native menu
			bool found = !i->first; // Only if custom menu, try to find it

			// If there is none, try to find a custom menu
			if (!found)
			{
				for (auto& entry : Menus::MenuList)
				{
					if (i->second == entry.second)
					{
						found = true;
						break;
					}
				}
			}

			// Remove the menu if it has been deallocated (not found)
			if (!found)
			{
				i = menus->erase(i);
				continue;
			}

			bool increment = true;

			// Remove the menu if it has been loaded twice
			for (auto& newMenu : newMenus)
			{
				if (i->second->window.name == std::string(newMenu.second->window.name))
				{
					Menus::RemoveMenu(i->second);

					i = menus->erase(i);
					increment = false;
					break;
				}
			}

			if (increment) ++i;
		}

		Utils::Merge(menus, newMenus);
	}

	Game::MenuList* Menus::LoadMenuList(Game::MenuList* menuList)
	{
		Utils::Memory::Allocator* allocator = Utils::Memory::GetAllocator();

		std::vector<std::pair<bool, Game::menuDef_t*>> menus;

		for (int i = 0; i < menuList->menuCount; ++i)
		{
			if (!menuList->menus[i]) continue;
			Menus::SafeMergeMenus(&menus, Menus::LoadMenu(menuList->menus[i]));
		}

		// Load custom menus
		if (menuList->name == "ui_mp/code.txt"s) // Should be menus, but code is loaded ingame
		{
			for (auto menu : Menus::CustomMenus)
			{
				bool hasMenu = false;
				for (auto& loadedMenu : menus)
				{
					if (loadedMenu.second->window.name == menu)
					{
						hasMenu = true;
						break;
					}
				}

				if (!hasMenu) Menus::SafeMergeMenus(&menus, Menus::LoadMenu(menu));
			}
		}

		// Allocate new menu list
		Game::MenuList* newList = allocator->allocate<Game::MenuList>();
		if (!newList) return menuList;

		size_t size = menus.size();
		newList->menus = allocator->allocateArray<Game::menuDef_t*>(size);
		if (!newList->menus)
		{
			allocator->free(newList);
			return menuList;
		}

		newList->name = allocator->duplicateString(menuList->name);
		newList->menuCount = size;

		// Copy new menus
		for (unsigned int i = 0; i < menus.size(); ++i)
		{
			newList->menus[i] = menus[i].second;
		}

		Menus::RemoveMenuList(newList->name);
		Menus::MenuListList[newList->name] = newList;

		return newList;
	}

	void Menus::FreeMenuSource(int handle)
	{
		Utils::Memory::Allocator* allocator = Utils::Memory::GetAllocator();

		if (!Menus::IsValidSourceHandle(handle)) return;

		Game::source_t* source = Game::sourceFiles[handle];

		while (source->scriptstack)
		{
			Game::script_t* script = source->scriptstack;
			source->scriptstack = source->scriptstack->next;
			Game::FreeMemory(script);
		}

		while (source->tokens)
		{
			Game::token_t* token = source->tokens;
			source->tokens = source->tokens->next;
			Game::FreeMemory(token);
		}

		while (source->defines)
		{
			Game::define_t* define = source->defines;
			source->defines = source->defines->next;
			Game::FreeMemory(define);
		}

		while (source->indentstack)
		{
			Game::indent_t* indent = source->indentstack;
			source->indentstack = source->indentstack->next;
			allocator->free(indent);
		}

		if (source->definehash) allocator->free(source->definehash);

		allocator->free(source);

		Game::sourceFiles[handle] = nullptr;
	}

	void Menus::FreeMenu(Game::menuDef_t* menudef)
	{
		Utils::Memory::Allocator* allocator = Utils::Memory::GetAllocator();

		// Do i need to free expressions and strings?
		// Or does the game take care of it?
		// Seems like it does...

		if (menudef->items)
		{
			// Seems like this is obsolete as well,
			// as the game handles the memory

			//for (int i = 0; i < menudef->itemCount; ++i)
			//{
			//	Game::Menu_FreeItemMemory(menudef->items[i]);
			//}

			allocator->free(menudef->items);
		}

		allocator->free(menudef);
	}

	void Menus::FreeMenuList(Game::MenuList* menuList)
	{
		if (!menuList) return;
		Utils::Memory::Allocator* allocator = Utils::Memory::GetAllocator();

		// Keep our compiler happy
		Game::MenuList list = { menuList->name, menuList->menuCount, menuList->menus };

		if (list.name)
		{
			allocator->free(list.name);
		}

		if (list.menus)
		{
			allocator->free(list.menus);
		}

		allocator->free(menuList);
	}

	void Menus::RemoveMenu(const std::string& menu)
	{
		auto i = Menus::MenuList.find(menu);
		if (i != Menus::MenuList.end())
		{
			if (i->second) Menus::FreeMenu(i->second);
			i = Menus::MenuList.erase(i);
		}
	}

	void Menus::RemoveMenu(Game::menuDef_t* menudef)
	{
		for (auto i = Menus::MenuList.begin(); i != Menus::MenuList.end();)
		{
			if (i->second == menudef)
			{
				Menus::FreeMenu(menudef);
				i = Menus::MenuList.erase(i);
			}
			else
			{
				++i;
			}
		}
	}

	void Menus::RemoveMenuList(const std::string& menuList)
	{
		auto i = Menus::MenuListList.find(menuList);
		if (i != Menus::MenuListList.end())
		{
			if (i->second)
			{
				for (auto j = 0; j < i->second->menuCount; ++j)
				{
					Menus::RemoveMenu(i->second->menus[j]);
				}

				Menus::FreeMenuList(i->second);
			}

			i = Menus::MenuListList.erase(i);
		}
	}

	// This is actually a really important function
	// It checks if we have already loaded the menu we passed and replaces its instances in memory
	// Due to deallocating the old menu, the game might crash on not being able to handle its old instance
	// So we need to override it in our menu lists and the game's ui context
	// EDIT: We might also remove the old instances inside RemoveMenu
	// EDIT2: Removing old instances without having a menu to replace them with might leave a nullptr
	// EDIT3: Wouldn't it be better to check if the new menu we're trying to load has already been loaded and not was not deallocated and return that one instead of loading a new one?
	void Menus::OverrideMenu(Game::menuDef_t* menu)
	{
		if (!menu || !menu->window.name) return;
		std::string name = menu->window.name;

		// Find the old menu
		auto i = Menus::MenuList.find(name);
		if (i != Menus::MenuList.end())
		{
			// We have found it, *yay*
			Game::menuDef_t* oldMenu = i->second;

			// Replace every old instance with our new one in the ui context
			for (int j = 0; j < Game::uiContext->menuCount; ++j)
			{
				if (Game::uiContext->Menus[j] == oldMenu)
				{
					Game::uiContext->Menus[j] = menu;
				}
			}

			// Replace every old instance with our new one in our menu lists
			for (auto j = Menus::MenuListList.begin(); j != Menus::MenuListList.end(); ++j)
			{
				Game::MenuList* list = j->second;

				if (list && list->menus)
				{
					for (int k = 0; k < list->menuCount; ++k)
					{
						if (list->menus[k] == oldMenu)
						{
							list->menus[k] = menu;
						}
					}
				}
			}
		}
	}

	void Menus::RemoveMenuList(Game::MenuList* menuList)
	{
		if (!menuList || !menuList->name) return;
		Menus::RemoveMenuList(menuList->name);
	}

	void Menus::FreeEverything()
	{
		for (auto i = Menus::MenuListList.begin(); i != Menus::MenuListList.end(); ++i)
		{
			Menus::FreeMenuList(i->second);
		}

		Menus::MenuListList.clear();

		for (auto i = Menus::MenuList.begin(); i != Menus::MenuList.end(); ++i)
		{
			Menus::FreeMenu(i->second);
		}

		Menus::MenuList.clear();
	}

	Game::XAssetHeader Menus::MenuFindHook(Game::XAssetType /*type*/, const std::string& filename)
	{
		return { Game::Menus_FindByName(Game::uiContext, filename.data()) };
	}

	Game::XAssetHeader Menus::MenuListFindHook(Game::XAssetType type, const std::string& filename)
	{
		Game::XAssetHeader header = { nullptr };

		// Free the last menulist and ui context, as we have to rebuild it with the new menus
		if (Menus::MenuListList.find(filename) != Menus::MenuListList.end())
		{
			Game::MenuList* list = Menus::MenuListList[filename];

			for (int i = 0; list && list->menus && i < list->menuCount; ++i)
			{
				Menus::RemoveMenuFromContext(Game::uiContext, list->menus[i]);
			}

			Menus::RemoveMenuList(filename);
		}

		if (Utils::String::EndsWith(filename, ".menu"))
		{
			if (FileSystem::File(filename).exists())
			{
				header.menuList = Menus::LoadScriptMenu(filename.data());
				if (header.menuList) return header;
			}
		}

		Game::MenuList* menuList = Game::DB_FindXAssetHeader(type, filename.data()).menuList;
		header.menuList = menuList;

		if (menuList && reinterpret_cast<DWORD>(menuList) != 0xDDDDDDDD)
		{
			// Parse scriptmenus!
			if ((menuList->menuCount > 0 && menuList->menus[0] && menuList->menus[0]->window.name == "default_menu"s))
			{
				if (FileSystem::File(filename).exists())
				{
					header.menuList = Menus::LoadScriptMenu(filename.data());

					// Reset, if we didn't find scriptmenus
					if (!header.menuList)
					{
						header.menuList = menuList;
					}
				}
			}
			else
			{
				header.menuList = Menus::LoadMenuList(menuList);
			}
		}
		else
		{
			header.menuList = nullptr;
		}

		return header;
	}

	bool Menus::IsMenuVisible(Game::UiContext* dc, Game::menuDef_t* menu)
	{
		if (menu && menu->window.name)
		{
			if (menu->window.name == "connect"s) // Check if we're supposed to draw the loadscreen
			{
				Game::menuDef_t* originalConnect = AssetHandler::FindOriginalAsset(Game::XAssetType::ASSET_TYPE_MENU, "connect").menu;

				if (originalConnect == menu) // Check if we draw the original loadscreen
				{
					if (Menus::MenuList.find("connect") != Menus::MenuList.end()) // Check if we have a custom loadscreen, to prevent drawing the original one on top
					{
						return false;
					}
				}
			}
		}

		return Game::Menu_IsVisible(dc, menu);
	}

	void Menus::RemoveMenuFromContext(Game::UiContext* dc, Game::menuDef_t* menu)
	{
		// Search menu in context
		int i = 0;
		for (; i < dc->menuCount; ++i)
		{
			if (dc->Menus[i] == menu)
			{
				break;
			}
		}

		// Remove from stack
		if (i < dc->menuCount)
		{
			for (; i < dc->menuCount - 1; ++i)
			{
				dc->Menus[i] = dc->Menus[i + 1];
			}

			// Clear last menu
			dc->Menus[--dc->menuCount] = nullptr;
		}
	}

	void Menus::Add(const std::string& menu)
	{
		Menus::CustomMenus.push_back(menu);
	}

	void Menus::RegisterCustomMenusHook()
	{
		Utils::Hook::Call<void()>(0x401700)(); // call original load functions

#ifdef _DEBUG
		for (int i = 0; i < Game::uiContext->menuCount; i++)
		{
			OutputDebugStringA(Utils::String::VA("%s\n", Game::uiContext->Menus[i]->window.name));
		}
#endif
	}

	Menus::Menus()
	{
		if (Dedicated::IsEnabled()) return;

		// Ensure everything is zero'ed
		Menus::FreeEverything();

		// Intercept asset finding
		AssetHandler::OnFind(Game::XAssetType::ASSET_TYPE_MENU, Menus::MenuFindHook);
		AssetHandler::OnFind(Game::XAssetType::ASSET_TYPE_MENULIST, Menus::MenuListFindHook);

		// Don't open connect menu
		//Utils::Hook::Nop(0x428E48, 5);

		// register custom menufiles if they exist
		Utils::Hook(0x4A58C3, Menus::RegisterCustomMenusHook, HOOK_CALL).install()->quick();

		// Use the connect menu open call to update server motds
		Utils::Hook(0x428E48, []()
		{
			if (!Party::GetMotd().empty() && Party::Target() == *Game::connectedHost)
			{
				Dvar::Var("didyouknow").set(Party::GetMotd());
			}
		}, HOOK_CALL).install()->quick();

			// Intercept menu painting
			Utils::Hook(0x4FFBDF, Menus::IsMenuVisible, HOOK_CALL).install()->quick();

			// disable the 2 new tokens in ItemParse_rect
			Utils::Hook::Set<BYTE>(0x640693, 0xEB);

			// don't load ASSET_TYPE_MENU assets for every menu (might cause patch menus to fail)
			Utils::Hook::Nop(0x453406, 5);

			//make Com_Error and similar go back to main_text instead of menu_xboxlive.
			Utils::Hook::SetString(0x6FC790, "main_text");

			Command::Add("openmenu", [](Command::Params* params)
			{
				if (params->size() != 2)
				{
					Logger::Print("USAGE: openmenu <menu name>\n");
					return;
				}

				// Not quite sure if we want to do this if we're not ingame, but it's only needed for ingame menus.
				if (Dvar::Var("cl_ingame").get<bool>())
				{
					Game::Key_SetCatcher(0, 16);
				}

				Game::Menus_OpenByName(Game::uiContext, params->get(1));
			});

			Command::Add("reloadmenus", [](Command::Params*)
			{
				// Close all menus
				Game::Menus_CloseAll(Game::uiContext);

				// Free custom menus
				Menus::FreeEverything();

				// Only disconnect if in-game, context is updated automatically!
				if (Game::CL_IsCgameInitialized())
				{
					Game::Cbuf_AddText(0, "disconnect\n");
				}
				else
				{
					// Reinitialize ui context
					Utils::Hook::Call<void()>(0x401700)();

					// Reopen main menu
					Game::Menus_OpenByName(Game::uiContext, "main_text");
				}
			});

			Command::Add("mp_QuickMessage", [](Command::Params*)
			{
				Command::Execute("openmenu quickmessage");
			});

			// Define custom menus here
			Menus::Add("ui_mp/changelog.menu");
			Menus::Add("ui_mp/theater_menu.menu");
			Menus::Add("ui_mp/pc_options_multi.menu");
			Menus::Add("ui_mp/pc_options_game.menu");
			Menus::Add("ui_mp/pc_options_gamepad.menu");
			Menus::Add("ui_mp/stats_reset.menu");
			Menus::Add("ui_mp/stats_unlock.menu");
			Menus::Add("ui_mp/security_increase_popmenu.menu");
			Menus::Add("ui_mp/mod_download_popmenu.menu");
			Menus::Add("ui_mp/popup_friends.menu");
			Menus::Add("ui_mp/menu_first_launch.menu");
			Menus::Add("ui_mp/startup_messages.menu");
			Menus::Add("ui_mp/iw4x_credits.menu");
			Menus::Add("ui_mp/resetclass.menu");
			Menus::Add("ui_mp/popup_customtitle.menu");
			Menus::Add("ui_mp/popup_customclan.menu");
	}

	Menus::~Menus()
	{
		Menus::FreeEverything();
	}
}
