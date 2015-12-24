#include "..\STDInclude.hpp"

namespace Components
{
	std::vector<Game::menuDef_t*> Menus::MenuList;
	std::vector<Game::MenuList*> Menus::MenuListList;

	int Menus::ReserveSourceHandle()
	{
		// Check if a free slot is available
		int i = 1;
		for (; i < MAX_SOURCEFILES; i++)
		{
			if (!Game::sourceFiles[i])
				break;
		}

		if (i >= MAX_SOURCEFILES)
			return 0;

		// Reserve it, if yes
		Game::sourceFiles[i] = (Game::source_t*)1;

		return i;
	}

	Game::script_t* Menus::LoadMenuScript(const char* name, std::string buffer)
	{
		Game::script_t* script = Game::Script_Alloc(sizeof(Game::script_t) + 1 + buffer.length());

		strcpy_s(script->filename, sizeof(script->filename), name);
		script->buffer = (char*)(script + 1);

		*((char*)(script + 1) + buffer.length()) = '\0';

		script->script_p = script->buffer;
		script->lastscript_p = script->buffer;
		script->length = buffer.length();
		script->end_p = &script->buffer[buffer.length()];
		script->line = 1;
		script->lastline = 1;
		script->tokenavailable = 0;

		Game::Script_SetupTokens(script, (void*)0x797F80);
		script->punctuations = (Game::punctuation_t*)0x797F80;

		strcpy(script->buffer, buffer.data());

		script->length = Game::Script_CleanString(script->buffer);

		return script;
	}

	int Menus::LoadMenuSource(const char* name, std::string buffer)
	{
		int handle = Menus::ReserveSourceHandle();
		if (!Menus::IsValidSourceHandle(handle)) return 0; // No free source slot!

		Game::source_t *source = nullptr;
		Game::script_t *script = Menus::LoadMenuScript(name, buffer);

		if (!script)
		{
			Game::sourceFiles[handle] = nullptr; // Free reserved slot
			return 0;
		}

		script->next = NULL;

		source = (Game::source_t *)calloc(1, sizeof(Game::source_t));

		strncpy(source->filename, "string", 64);
		source->scriptstack = script;
		source->tokens = NULL;
		source->defines = NULL;
		source->indentstack = NULL;
		source->skip = 0;
		source->definehash = (Game::define_t**)calloc(1, 4096);

		Game::sourceFiles[handle] = source;

		return handle;
	}

	bool Menus::IsValidSourceHandle(int handle)
	{
		return (handle > 0 && handle < MAX_SOURCEFILES && Game::sourceFiles[handle]);
	}

	int Menus::KeywordHash(char* key)
	{
		// patch this function on-the-fly, as it's some ugly C.
		Utils::Hook::Set<DWORD>(0x63FE9E, 3523);
		Utils::Hook::Set<DWORD>(0x63FECB, 0x7F);

		int var = 0x63FE90;
		__asm
		{
			mov eax, key
			call var
			mov var, eax
		}

		Utils::Hook::Set<DWORD>(0x63FE9E, 531);
		Utils::Hook::Set<DWORD>(0x63FECB, 0x1FF);

		return var;
	}

	Game::menuDef_t* Menus::ParseMenu(int handle)
	{
		Game::menuDef_t* menu = (Game::menuDef_t*)calloc(1, 2048); // FIXME: tentative size
		menu->items = (Game::itemDef_t**)calloc(512, sizeof(Game::itemDef_t*));
		Menus::MenuList.push_back(menu);

		Game::pc_token_t token;
		Game::keywordHash_t *key;

		if (!Menus::ReadToken(handle, &token) || token.string[0] != '{')
		{
			return menu;
		}

		while (true) 
		{
			ZeroMemory(&token, sizeof(token));

			if (!Menus::ReadToken(handle, &token)) 
			{
				Game::PC_SourceError(handle, "end of file inside menu\n");
				break; // Fail
			}

			if (*token.string == '}') 
			{
				break; // Success
			}

			int idx = Menus::KeywordHash(token.string);

			key = Game::menuParseKeywordHash[idx];

			if (!key) 
			{
				Game::PC_SourceError(handle, "unknown menu keyword %s", token.string);
				continue;
			}

			if (!key->func((Game::itemDef_t*)menu, handle)) 
			{
				Game::PC_SourceError(handle, "couldn't parse menu keyword %s", token.string);
				break; // Fail
			}
		}

		return menu;
	}

	int Menus::ReadToken(int handle, Game::pc_token_t *pc_token)
	{
		Game::token_t token;
		int ret;

		if (!Menus::IsValidSourceHandle(handle)) return 0;

		ret = Game::PC_ReadToken(Game::sourceFiles[handle], &token);
		strcpy(pc_token->string, token.string);
		pc_token->type = token.type;
		pc_token->subtype = token.subtype;
		pc_token->intvalue = token.intvalue;
		pc_token->floatvalue = (float)token.floatvalue;

		if (pc_token->type == TT_STRING)
		{
			// StripDoubleQuotes
			char *string = pc_token->string;
			if (*string == '\"')
			{
				strcpy(string, string + 1);
			}
			if (string[strlen(string) - 1] == '\"')
			{
				string[strlen(string) - 1] = '\0';
			}
		}

		return ret;
	}

	std::vector<Game::menuDef_t*> Menus::LoadMenu(Game::menuDef_t* menudef)
	{
		std::vector<Game::menuDef_t*> menus;
		FileSystem::File menuFile(Utils::VA("ui_mp\\%s.menu", menudef->window.name));

		if (menuFile.Exists())
		{
			Game::pc_token_t token;
			int handle = Menus::LoadMenuSource(menudef->window.name, menuFile.GetBuffer());
			if (!Menus::IsValidSourceHandle(handle))
			{
				menus.push_back(menudef);
				return menus;
			}

			while (true)
			{
				ZeroMemory(&token, sizeof(token));

				if (!Menus::ReadToken(handle, &token) || token.string[0] == '}')
				{
					break;
				}

				if (!_stricmp(token.string, "loadmenu"))
				{
					Menus::ReadToken(handle, &token);

					// Ugly, but does the job ;)
					Game::menuDef_t _temp;
					_temp.window.name = token.string;

					std::vector<Game::menuDef_t*> newMenus = Menus::LoadMenu(&_temp);

					for (auto newMenu : newMenus)
					{
						menus.push_back(newMenu);
					}
				}

				if (!_stricmp(token.string, "menudef"))
				{
					menus.push_back(Menus::ParseMenu(handle));
				}
			}

			Menus::FreeMenuSource(handle);
		}

		if (!menus.size())
		{
			menus.push_back(menudef);
		}

		return menus;
	}

	Game::MenuList* Menus::LoadMenuList(Game::MenuList* menuList)
	{
		std::vector<Game::menuDef_t*> menus;

		for (int i = 0; i < menuList->menuCount; i++)
		{
			std::vector<Game::menuDef_t*> newMenus = Menus::LoadMenu(menuList->menus[i]);

			for (auto newMenu : newMenus)
			{
				menus.push_back(newMenu);
			}
		}

		// Allocate new menu list
		Game::MenuList* newList = (Game::MenuList*)calloc(1, sizeof(Game::MenuList));
		newList->name = _strdup(menuList->name);
		newList->menus = (Game::menuDef_t **)calloc(menus.size(), sizeof(Game::menuDef_t *));
		newList->menuCount = menus.size();

		// Copy new menus
		memcpy(newList->menus, menus.data(), menus.size() * sizeof(Game::menuDef_t *));

		Menus::MenuListList.push_back(newList);

		return newList;
	}

	void Menus::FreeMenuScript(Game::script_t* script)
	{
		Game::FreeMemory(script);
	}

	void Menus::FreeMenuSource(int handle)
	{
		if (!Menus::IsValidSourceHandle(handle)) return;

		Game::script_t *script;
// 		Game::token_t *token;
// 		Game::define_t *define;
		Game::indent_t *indent;
		Game::source_t *source = Game::sourceFiles[handle];

		while (source->scriptstack)
		{
			script = source->scriptstack;
			source->scriptstack = source->scriptstack->next;
			Menus::FreeMenuScript(script);
		}

		while (source->indentstack)
		{
			indent = source->indentstack;
			source->indentstack = source->indentstack->next;
			free(indent);
		}

		if (source->definehash) free(source->definehash);

		free(source);

		Game::sourceFiles[handle] = nullptr;
	}

	void Menus::FreeMenu(Game::menuDef_t* menudef)
	{
		if (menudef->items) free(menudef->items);
		free(menudef);
	}

	void Menus::FreeMenuList(Game::MenuList* menuList)
	{
		if (menuList)
		{
			if (menuList->name)
			{
				free((void*)menuList->name);
			}

			if (menuList->menus)
			{
				free(menuList->menus);
			}

			free(menuList);
		}
	}

	void Menus::FreeEverything()
	{
		for (auto menu : Menus::MenuList)
		{
			Menus::FreeMenu(menu);
		}

		Menus::MenuList.clear();

		for (auto menuList : Menus::MenuListList)
		{
			Menus::FreeMenuList(menuList);
		}

		Menus::MenuListList.clear();
	}

	Game::XAssetHeader Menus::MenuFileLoad(Game::XAssetType type, const char* filename)
	{
		Game::XAssetHeader header = { 0 };

		// Check if we already loaded it
		for (auto menuList : Menus::MenuListList)
		{
			if (!strcmp(menuList->name, filename))
			{
				header.menuList = menuList;
				return header;
			}
		}

		Game::MenuList* menuList = Game::DB_FindXAssetHeader(type, filename).menuList;
		header.menuList = menuList;
		
		if (menuList)
		{
			header.menuList = Menus::LoadMenuList(menuList);
		}

		return header;
	}

	Menus::Menus()
	{
		AssetHandler::On(Game::XAssetType::ASSET_TYPE_MENUFILE, Menus::MenuFileLoad);

		// disable the 2 new tokens in ItemParse_rect
		Utils::Hook::Set<BYTE>(0x640693, 0xEB);

		// don't load ASSET_TYPE_MENU assets for every menu (might cause patch menus to fail)
		Utils::Hook::Nop(0x453406, 5);

		Command::Add("openmenu", [] (Command::Params params)
		{
			if (params.Length() != 2)
			{
				Logger::Print("USAGE: openmenu <menu name>\n");
				return;
			}

			Game::Menus_OpenByName(0x62E2858, params[1]);
		});

		Command::Add("reloadmenus", [] (Command::Params params)
		{
			if (Game::CL_IsCgameInitialized())
			{
				Logger::Print("Realoading menus in-game is not allowed!\n");
				return;
			}

			// Close all menus
			Game::Menus_CloseAll(0x62E2858);

			// Free custom menus
			Menus::FreeEverything();
			
			// Reinitialize ui context
			((void(*)())0x401700)();

			// Reopen main menu
			Game::Menus_OpenByName(0x62E2858, "main_text");
		});
	}

	Menus::~Menus()
	{
		Menus::FreeEverything();
	}
}
