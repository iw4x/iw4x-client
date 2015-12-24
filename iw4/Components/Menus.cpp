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

	Game::script_t* Menus::LoadMenuScript(std::string buffer)
	{
		Game::script_t* script = Game::Script_Alloc(sizeof(Game::script_t) + 1 + buffer.length());

		strcpy_s(script->filename, sizeof(script->filename), "script_t");
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

	int Menus::LoadMenuSource(std::string buffer)
	{
		int handle = Menus::ReserveSourceHandle();
		if (!handle) return 0; // No free source slot!

		Game::source_t *source = nullptr;
		Game::script_t *script = Menus::LoadMenuScript(buffer);

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

	Game::menuDef_t* Menus::LoadMenu(Game::menuDef_t* menudef)
	{
		FileSystem::File menuFile(Utils::VA("ui_mp\\%s.menu", menudef->window.name));

		if (menuFile.Exists())
		{
			int handle = Menus::LoadMenuSource(menuFile.GetBuffer());

			// TODO: Parse menu!

			Menus::FreeMenuSource(handle);
		}

		return menudef;
	}

	Game::MenuList* Menus::LoadMenuList(Game::MenuList* menuList)
	{
		bool NewMenuLoaded = false;

		Game::MenuList* newList = (Game::MenuList*)calloc(1, sizeof(Game::MenuList));
		newList->name = _strdup(menuList->name);
		newList->menus = (Game::menuDef_t **)calloc(menuList->menuCount, sizeof(Game::menuDef_t *));
		newList->menuCount = menuList->menuCount;

		Menus::MenuListList.push_back(newList);

		for (int i = 0; i < newList->menuCount; i++)
		{
			newList->menus[i] = Menus::LoadMenu(menuList->menus[i]);
		}

		return newList;
	}

	void Menus::FreeMenuScript(Game::script_t* script)
	{
		Game::FreeMemory(script);
	}

	void Menus::FreeMenuSource(int handle)
	{
		if (!Game::sourceFiles[handle]) return;

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
		//
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
			Menus::FreeEverything();
			// TODO: Refresh ui context?
		});
	}

	Menus::~Menus()
	{
		Menus::FreeEverything();
	}
}
