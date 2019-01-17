#include "STDInclude.hpp"

namespace Components
{
	std::unordered_map<std::string, Game::menuDef_t*> Menus::DiskMenuList;
    std::unordered_map<std::string, Game::MenuList*> Menus::DiskMenuListList;
    std::unordered_map<std::string, std::pair<int, Game::menuDef_t*>> Menus::UiContextMenus;

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

		Game::script_t *script = Menus::LoadMenuScript(name, buffer);

		if (!script)
		{
			Game::sourceFiles[handle] = nullptr; // Free reserved slot
			return 0;
		}

		script->next = nullptr;

		Game::source_t *source = allocator->allocate<Game::source_t>();
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
		source->definehash = static_cast<Game::define_t**>(allocator->allocate(4096));

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

		Game::menuDef_t* menu = allocator->allocate<Game::menuDef_t>();
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

		return menu;
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

    // Can be used to load scriptmenus or menulists
	Game::MenuList* Menus::LoadMenuList(const std::string& name)
	{
		Utils::Memory::Allocator* allocator = Utils::Memory::GetAllocator();

		std::vector<std::pair<bool, Game::menuDef_t*>> menus = Menus::LoadMenu(name);
		if (menus.empty()) return nullptr;

		// Allocate new menu list
		Game::MenuList* newList = allocator->allocate<Game::MenuList>();
		if (!newList) return nullptr;

		newList->menus = allocator->allocateArray<Game::menuDef_t*>(menus.size());
		if (!newList->menus)
		{
			allocator->free(newList);
			return nullptr;
		}

		newList->name = allocator->duplicateString(name);
		newList->menuCount = menus.size();

		// Copy new menus
		for (unsigned int i = 0; i < menus.size(); ++i)
		{
			newList->menus[i] = menus[i].second;
		}

		return newList;
	}

	void Menus::FreeMenuSource(int handle)
	{
		Utils::Memory::Allocator* allocator = Utils::Memory::GetAllocator();

		if (!Menus::IsValidSourceHandle(handle)) return;

		Game::source_t *source = Game::sourceFiles[handle];

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

	void Menus::FreeDiskMenu(Game::menuDef_t* menudef)
	{
		Utils::Memory::Allocator* allocator = Utils::Memory::GetAllocator();

		// Do i need to free expressions and strings?
		// Or does the game take care of it?
		// Seems like it does...
		if (menudef->items)
		{
			allocator->free(menudef->items);
		}

		allocator->free(menudef);
	}

	void Menus::FreeDiskMenuList(Game::MenuList* menuList)
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

	void Menus::FreeEverything()
	{
		for (auto i = Menus::DiskMenuListList.begin(); i != Menus::DiskMenuListList.end(); ++i)
		{
			Menus::FreeDiskMenuList(i->second);
		}

		Menus::DiskMenuListList.clear();

		for (auto i = Menus::DiskMenuList.begin(); i != Menus::DiskMenuList.end(); ++i)
		{
			Menus::FreeDiskMenu(i->second);
		}

		Menus::DiskMenuList.clear();

        Menus::UiContextMenus.clear();
	}

	Game::XAssetHeader Menus::MenuFindHook(Game::XAssetType /*type*/, const std::string& filename)
    {
		return { Game::Menus_FindByName(Game::uiContext, filename.data()) };
	}

	Game::XAssetHeader Menus::MenuListFindHook(Game::XAssetType /*type*/, const std::string& filename)
	{
		Game::XAssetHeader header = { nullptr };

        // if menulist or scriptmenu exists on the disk, then load it
		if (FileSystem::File(filename).exists())
		{
			header.menuList = Menus::LoadMenuList(filename.data());
			if (header.menuList) return header;
		}

        // we don't need to modify any base assets here

		return header;
	}

	bool Menus::IsMenuVisible(Game::UiContext *dc, Game::menuDef_t *menu)
	{
		if (menu && menu->window.name)
		{
			if (menu->window.name == "connect"s) // Check if we're supposed to draw the loadscreen
			{
				Game::menuDef_t* originalConnect = AssetHandler::FindOriginalAsset(Game::XAssetType::ASSET_TYPE_MENU, "connect").menu;

				if (originalConnect == menu) // Check if we draw the original loadscreen
				{
					if (Menus::DiskMenuList.find("connect") != Menus::DiskMenuList.end()) // Check if we have a custom loadscreen, to prevent drawing the original one on top
					{
						return false;
					}
				}
			}
		}

		return Game::Menu_IsVisible(dc, menu);
	}

    void Menus::AddMenuToContext(Game::UiContext* ctx, int priority, Game::menuDef_t* menu)
    {
        if(ctx->menuCount < MAX_MENUS_IN_CONTEXT)
        {
            ctx->Menus[ctx->menuCount++] = menu;
        }

        Menus::UiContextMenus[menu->window.name] = { priority, menu };
    }

    std::pair<int, Game::menuDef_t*> Menus::FindMenuInContext(Game::UiContext* /*ctx*/, const std::string& name)
    {
        auto entry = Menus::UiContextMenus.find(name);
        if (entry == Menus::UiContextMenus.end()) return { 0, nullptr };
        return entry->second;
    }

    // overwrite entry in uiContext, update it in our list, and free it if it was a disk menu
    void Menus::ReplaceMenuInContext(Game::UiContext* ctx, int priority, Game::menuDef_t* menu)
    {
        for (int i = 0; i < ctx->menuCount; i++)
        {
            Game::menuDef_t* cur = ctx->Menus[i];
            if (!_stricmp(cur->window.name, menu->window.name))
            {
                // check if it was a disk menu and free it if it was
                if (Menus::DiskMenuList.find(cur->window.name) != Menus::DiskMenuList.end())
                {
                    Menus::FreeDiskMenu(cur);
                }

                // replace entry in context
                ctx->Menus[i] = menu;
                break;
            }
        }

        // update our list
        Menus::UiContextMenus[menu->window.name] = { priority, menu };
    }  

    // enforce priority on what menus get loaded
    // builtin < iw4x < mod
    void Menus::AddMenuListToContext(Game::UiContext* ctx, Game::MenuList* list, int close)
    {
        int insertPriority = Menus::MenuContextPriority::PRIORITY_BUILTIN;

        if (!strncmp(list->name, "ui_mp/iw4x.txt", 13)) insertPriority = Menus::MenuContextPriority::PRIORITY_IW4X;
        if (!strncmp(list->name, "ui_mp/mod.txt", 12)) insertPriority = Menus::MenuContextPriority::PRIORITY_MOD;

        for (int i = 0; i < list->menuCount; i++)
        {
            Game::menuDef_t* cur = list->menus[i];

            // check if menu already exists in context and replace if priority is higher
            std::pair<int, Game::menuDef_t*> ctxEntry = Menus::FindMenuInContext(ctx, cur->window.name);
            if (ctxEntry.second) // if menu ptr is null then it wasnt found
            {
                if (insertPriority >= ctxEntry.first) // compare priorities to see if we should replace
                {
                    Menus::ReplaceMenuInContext(ctx, insertPriority, cur);
                }
            }
            else // otherwise just insert
            {
                Menus::AddMenuToContext(ctx, insertPriority, cur);
            }

            if (close)
            {
                Game::Menus_CloseRequest(ctx, cur);
            }
        }
    }

    void Menus::RegisterMenuLists()
    {
        Utils::Hook::Call<void()>(0x401700)(); // reset ui context

        // we can't call DB_FindXAssetHeader here because it blocks the rest of loading waiting on those 2 menulsits
        // TODO: Figure out a better way to trigger the custom menulist loading because if you skip the intro the 
        // custom menus won't have loaded until a few seconds later. All overridden menus are already
        // loaded so it isn't a black screen but it wont show the first time intro, credits, etc.
        // as soon as this loads those start to work again
        // if we just trigger this here it blocks the intro from showing because of the FindXAssetHeader calls
        // that are waiting for zones to finish loading
        Scheduler::OnReady([]()
        {
            // attempt to load iw4x menus            
            Game::XAssetHeader header = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_MENULIST, "ui_mp/iw4x.txt");
            if (header.data && !(header.menuList->menuCount == 1 && !_stricmp("default_menu", header.menuList->menus[0]->window.name)))
            {
                Menus::AddMenuListToContext(Game::uiContext, header.menuList, 1);
            }

            // attempt to load mod menus
            header = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_MENULIST, "ui_mp/mod.txt");
            if (header.data && !(header.menuList->menuCount == 1 && !_stricmp("default_menu", header.menuList->menus[0]->window.name)))
            {
                Menus::AddMenuListToContext(Game::uiContext, header.menuList, 1);
            }
        }, true);
    }

    void Menus::ResetContextHook(int a1)
    {
        // reset our lists
        Menus::FreeEverything();

        // continue with initialization
        Utils::Hook::Call<void(int)>(0x4A57D0)(a1);
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
         Utils::Hook(0x4A58C3, Menus::RegisterMenuLists, HOOK_CALL).install()->quick();

        // take control of menus in uiContext
        Utils::Hook(0x4533C0, Menus::AddMenuListToContext, HOOK_JUMP).install()->quick();

        // reset our list on UiContext reset
        Utils::Hook(0x4B5422, Menus::ResetContextHook, HOOK_CALL).install()->quick();

        // grab custom lists as they are loaded otherwise DB takes up to 20 seconds to load intro
        /*
        AssetHandler::OnLoad([](Game::XAssetType type, Game::XAssetHeader asset, std::string name, bool*)
        {
            if (type != Game::XAssetType::ASSET_TYPE_MENULIST) return;

            if (name == "ui_mp/iw4x.txt" || name == "ui_mp/mod.txt")
            {
                Menus::AddMenuListToContext(Game::uiContext, asset.menuList, 1);
            }
        });

        */

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
			if (params->length() != 2)
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

			// Free custom menus and reset uiContext list
			Menus::FreeEverything();

			// Only disconnect if in-game, context is updated automatically!
			if (Game::CL_IsCgameInitialized())
			{
				Game::Cbuf_AddText(0, "disconnect\n");
			}
			else
			{
                Menus::RegisterMenuLists(); // register custom menus

				// Reopen main menu
				Game::Menus_OpenByName(Game::uiContext, "main_text");
			}
		});

#ifndef DISABLE_ANTICHEAT
		Scheduler::OnFrameAsync(AntiCheat::QuickCodeScanner2);
#endif

		Command::Add("mp_QuickMessage", [](Command::Params*)
		{
			Command::Execute("openmenu quickmessage");
		});
	}

	Menus::~Menus()
	{
		Menus::FreeEverything();
	}
}
