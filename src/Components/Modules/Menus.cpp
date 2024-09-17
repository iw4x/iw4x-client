#include <STDInclude.hpp>
#include "Party.hpp"
#include "Events.hpp"

#define MAX_SOURCEFILES	64
#define DEFINEHASHSIZE 1024

namespace Components
{

	/// This variable dispenses us from the horror of having a text file in IWD containing the menus we want to load
	std::vector<std::string> Menus::CustomIW4xMenus;

	std::unordered_map<std::string, Game::menuDef_t*> Menus::MenusFromDisk;
	std::unordered_map<std::string, Game::MenuList*> Menus::MenuListsFromDisk;

	std::unordered_map<std::string, Game::menuDef_t*> Menus::OverridenMenus;


	Utils::Memory::Allocator Menus::Allocator;

	Game::KeywordHashEntry<Game::menuDef_t, 128, 3523>** menuParseKeywordHash;

	template <int HASH_COUNT, int HASH_SEED>
	static int KeywordHashKey(const char* keyword)
	{
		auto hash = 0;
		for (auto i = 0; keyword[i]; ++i)
		{
			hash += (i + HASH_SEED) * std::tolower(static_cast<unsigned char>(keyword[i]));
		}
		return (hash + (hash >> 8)) & (128 - 1);
	}

	template <typename T, int N, int M>
	static Game::KeywordHashEntry<T, N, M>* KeywordHashFind(Game::KeywordHashEntry<T, N, M>** table, const char* keyword)
	{
		auto hash = KeywordHashKey<N, M>(keyword);
		Game::KeywordHashEntry<T, N, M>* key = table[hash];
		if (key && !_stricmp(key->keyword, keyword))
		{
			return key;
		}
		return nullptr;
	}

	int Menus::ReserveSourceHandle()
	{
		// Check if a free slot is available
		auto i = 1;
		while (i < MAX_SOURCEFILES)
		{
			if (!Game::sourceFiles[i])
			{
				break;
			}

			++i;
		}

		if (i >= MAX_SOURCEFILES)
		{
			return 0;
		}

		// Reserve it, if yes
		Game::sourceFiles[i] = reinterpret_cast<Game::source_s*>(1);

		return i;
	}

	Game::script_s* Menus::LoadMenuScript(const std::string& name, const std::string& buffer)
	{
		auto* script = static_cast<Game::script_s*>(Game::GetClearedMemory(sizeof(Game::script_s) + 1 + buffer.length()));
		if (!script) return nullptr;

		strcpy_s(script->filename, sizeof(script->filename), name.data());
		script->buffer = reinterpret_cast<char*>(script + 1);

		*(script->buffer + buffer.length()) = '\0';

		script->script_p = script->buffer;
		script->lastscript_p = script->buffer;
		script->length = static_cast<int>(buffer.length());
		script->end_p = &script->buffer[buffer.length()];
		script->line = 1;
		script->lastline = 1;
		script->tokenavailable = 0;

		Game::PS_CreatePunctuationTable(script, Game::default_punctuations);
		script->punctuations = Game::default_punctuations;

		std::memcpy(script->buffer, buffer.data(), script->length + 1);

		script->length = Game::Com_Compress(script->buffer);

		return script;
	}

	int Menus::LoadMenuSource(const std::string& name, const std::string& buffer)
	{
		const auto handle = ReserveSourceHandle();
		if (!IsValidSourceHandle(handle)) return 0; // No free source slot!

		auto* script = LoadMenuScript(name, buffer);
		if (!script)
		{
			Game::sourceFiles[handle] = nullptr; // Free reserved slot
			return 0;
		}

		auto* source = static_cast<Game::source_s*>(Game::GetMemory(sizeof(Game::source_s)));
		std::memset(source, 0, sizeof(Game::source_s));

		script->next = nullptr;

		strncpy_s(source->filename, name.data(), _TRUNCATE);
		source->scriptstack = script;
		source->tokens = nullptr;
		source->defines = nullptr;
		source->indentstack = nullptr;
		source->skip = 0;
		source->definehash = static_cast<Game::define_s**>(Game::GetClearedMemory(DEFINEHASHSIZE * sizeof(Game::define_s*)));

		Game::sourceFiles[handle] = source;

		return handle;
	}

	bool Menus::IsValidSourceHandle(int handle)
	{
		return (handle > 0 && handle < MAX_SOURCEFILES && Game::sourceFiles[handle]);
	}

	Game::menuDef_t* Menus::ParseMenu(int handle)
	{
		auto* menu = Allocator.allocate<Game::menuDef_t>();
		if (!menu)
		{
			Components::Logger::PrintError(Game::CON_CHANNEL_UI, "No more memory to allocate menu\n");
			return nullptr;
		}

		menu->items = Allocator.allocateArray<Game::itemDef_s*>(512);
		if (!menu->items)
		{
			Components::Logger::PrintError(Game::CON_CHANNEL_UI, "No more memory to allocate menu items\n");
			Allocator.free(menu);
			return nullptr;
		}

		Game::pc_token_s token;
		if (!Game::PC_ReadTokenHandle(handle, &token) || token.string[0] != '{')
		{
			Components::Logger::PrintError(Game::CON_CHANNEL_UI, "Invalid or unexpected syntax on menu\n");
			Allocator.free(menu->items);
			Allocator.free(menu);
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

			auto* key = KeywordHashFind(menuParseKeywordHash, token.string);
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

		if (!menu->window.name)
		{
			Game::PC_SourceError(handle, "menu has no name");
			Allocator.free(menu->items);
			Allocator.free(menu);
			return nullptr;
		}

		// Shrink item size now that we're done parsing
		{
			const auto newItemArray = Allocator.allocateArray<Game::itemDef_s*>(menu->itemCount);
			std::memcpy(newItemArray, menu->items, menu->itemCount * sizeof(Game::itemDef_s*));

			Allocator.free(menu->items);

			menu->items = newItemArray;
		}

		// Reallocate Menu with our allocator because these data will get freed when LargeLocal::Reset gets called!
		{
			Components::Logger::Print("[MENUS] {:X} Reallocating menu {} ({:X})...\n",
				std::hash<std::thread::id>{}(std::this_thread::get_id()),
				menu->window.name,
				(unsigned int)(menu)
			);

			menu->window.name = Allocator.duplicateString(menu->window.name);

			for (int i = 0; i < menu->itemCount; i++)
			{
				const auto item = menu->items[i];
				const auto reallocatedItem = Allocator.allocate<Game::itemDef_s>();
				std::memcpy(reallocatedItem, item, sizeof(Game::itemDef_s));

				reallocatedItem->floatExpressions = Allocator.allocateArray<Game::ItemFloatExpression>(item->floatExpressionCount);

				if (item->floatExpressionCount)
				{
					std::memcpy(reallocatedItem->floatExpressions, item->floatExpressions, sizeof(Game::ItemFloatExpression) * item->floatExpressionCount);

					for (auto j = 0; j < item->floatExpressionCount; ++j)
					{
						const auto previousExpression = item->floatExpressions[j].expression;
						reallocatedItem->floatExpressions[j].expression = ReallocateExpressionLocally(previousExpression, false);
					}
				}

				// What about item expressions? We don't free these?
				// Apparently not, the game doesn't free them
				Game::Menu_FreeItem(item);
				menu->items[i] = reallocatedItem;
			}

			menu->visibleExp = ReallocateExpressionLocally(menu->visibleExp, true);
			menu->rectXExp = ReallocateExpressionLocally(menu->rectXExp, true);
			menu->rectYExp = ReallocateExpressionLocally(menu->rectYExp, true);
			menu->rectWExp = ReallocateExpressionLocally(menu->rectWExp, true);
			menu->rectHExp = ReallocateExpressionLocally(menu->rectHExp, true);
			menu->openSoundExp = ReallocateExpressionLocally(menu->openSoundExp, true);
			menu->closeSoundExp = ReallocateExpressionLocally(menu->closeSoundExp, true);

		}

		return menu;
	}

	std::vector<Game::menuDef_t*> Menus::LoadMenuByName_Recursive(const std::string& menu)
	{
		std::vector<Game::menuDef_t*> menus;
		FileSystem::File menuFile(menu);

		if (menuFile.exists())
		{
			Game::pc_token_s token;
			const auto handle = LoadMenuSource(menu, menuFile.getBuffer());

			if (IsValidSourceHandle(handle))
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

						const auto loadedMenu = LoadMenuByName_Recursive(Utils::String::VA("ui_mp\\%s.menu", token.string));

						for (const auto& loaded : loadedMenu)
						{
							menus.emplace_back(loaded);
						}
					}
					else if (!_stricmp(token.string, "menudef"))
					{
						auto* menuDef = ParseMenu(handle);
						if (menuDef)
						{
							menus.emplace_back(menuDef);
						}
					}
				}

				FreeMenuSource(handle);
			}
		}

		return menus;
	}

	void Menus::LoadScriptMenu(const char* menu, bool allowNewMenus)
	{
		auto menus = LoadMenuByName_Recursive(menu);

		if (menus.empty())
		{
			Components::Logger::PrintError(Game::CON_CHANNEL_UI, "Could not load menu {}\n", menu);
			return;
		}

		if (!allowNewMenus)
		{
			// We remove every menu we loaded that is not going to override something
			for (int i = 0; i < static_cast<int>(menus.size()); i++)
			{
				const auto menuName = menus[i]->window.name;
				if (Game::Menus_FindByName(Game::uiContext, menuName))
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

			Components::Logger::Print("[MENUS] {:X} Loaded menuList {} at {:X} from disk\n",
				std::hash<std::thread::id>{}(std::this_thread::get_id()),
				newList->name,
				(unsigned int)newList
			);

			MenuListsFromDisk[menuListName] = newList;
		}
	}

	void Menus::FreeScript(Game::script_s* script)
	{
		if (script->punctuationtable)
		{
			Game::FreeMemory(script->punctuationtable);
		}

		Game::FreeMemory(script);
	}

	void Menus::FreeMenuSource(int handle)
	{
		if (!IsValidSourceHandle(handle)) return;

		auto* source = Game::sourceFiles[handle];

		while (source->scriptstack)
		{
			auto* script = source->scriptstack;
			source->scriptstack = source->scriptstack->next;
			FreeScript(script);
		}

		while (source->tokens)
		{
			auto* token = source->tokens;
			source->tokens = source->tokens->next;

			Game::FreeMemory(token);
			--*Game::numtokens;
		}

		for (auto i = 0; i < DEFINEHASHSIZE; ++i)
		{
			while (source->definehash[i])
			{
				auto* define = source->definehash[i];
				source->definehash[i] = source->definehash[i]->hashnext;
				Game::PC_FreeDefine(define);
			}
		}

		while (source->indentstack)
		{
			auto* indent = source->indentstack;
			source->indentstack = source->indentstack->next;
			Game::FreeMemory(indent);
		}

		if (source->definehash)
		{
			Game::FreeMemory(source->definehash);
		}

		Game::FreeMemory(source);

		Game::sourceFiles[handle] = nullptr;
	}

	void Menus::FreeItem(Game::itemDef_s* item)
	{
		for (auto i = 0; i < item->floatExpressionCount; ++i)
		{
			FreeExpression(item->floatExpressions[i].expression);
		}

		Allocator.free(item->floatExpressions);

		item->floatExpressionCount = 0;
		Allocator.free(item);
	}

	void Menus::PrepareToUnloadMenu(Game::menuDef_t* menu)
	{
		const std::string name = menu->window.name;

		bool isRemoval = OverridenMenus[name] == nullptr || !OverridenMenus.contains(name);
		Game::menuDef_t* replacement = isRemoval ? nullptr : OverridenMenus[name];

		if (Game::uiContext)
		{
			for (size_t i = 0; i < ARRAYSIZE(Game::uiContext->menuStack); i++)
			{
				if (Game::uiContext->menuStack[i] &&
					Game::uiContext->menuStack[i]->window.name == name)
				{
					Components::Logger::Print("[MENUS] {:X} In stack - Restored menu {} ({:X} => {:X})\n",
						std::hash<std::thread::id>{}(std::this_thread::get_id()),
						name,
						(unsigned int)Game::uiContext->Menus[i],
						(unsigned int)replacement
					);

					if (isRemoval)
					{
						if (Game::uiContext->menuStack[i] == menu)
						{
							Game::uiContext->menuStack[i] = replacement;

							for (int j = i; j < Game::uiContext->openMenuCount - 1; j++)
							{
								Game::uiContext->menuStack[j] = Game::uiContext->menuStack[j + 1];
							}

							Game::uiContext->openMenuCount--;
							assert(Game::uiContext->openMenuCount >= 0);
						}
						else
						{
							// The menu I could have overriden got loaded in the meantime - no need to delete them
							// I'm simply going to remove myself
						}
					}
					else
					{
						Game::uiContext->menuStack[i] = replacement;
					}
				}
			}

			for (size_t i = 0; i < ARRAYSIZE(Game::uiContext->Menus); i++)
			{
				if (Game::uiContext->Menus[i] &&
					Game::uiContext->Menus[i]->window.name == name)
				{
					Components::Logger::Print("[MENUS] {:X} In context - Restored menu {} ({:X} => {:X})\n",
						std::hash<std::thread::id>{}(std::this_thread::get_id()),
						name,
						(unsigned int)Game::uiContext->Menus[i],
						(unsigned int)replacement
					);

					if (isRemoval)
					{
						if (Game::uiContext->Menus[i] == menu)
						{
							Game::uiContext->Menus[i] = replacement;

							for (size_t j = i; j < std::min(static_cast<unsigned int>(Game::uiContext->menuCount), ARRAYSIZE(Game::uiContext->Menus)) - 1; j++)
							{
								Game::uiContext->Menus[j] = Game::uiContext->Menus[j + 1];
							}

							Game::uiContext->menuCount--;
							assert(Game::uiContext->menuCount >= 0);
						}
						else
						{
							// The menu I could have overriden got loaded in the meantime - no need to delete them
							// I'm simply going to remove myself
						}
					}
					else
					{
						Game::uiContext->Menus[i] = replacement;
					}
				}
			}

			if (OverridenMenus.contains(name))
			{
				OverridenMenus.erase(name);
			}
		}
	}

	void Menus::AfterLoadedMenuFromDisk(Game::menuDef_t* menu)
	{
		const std::string name = menu->window.name;

		Components::Logger::Print("[MENUS] {:X} Loaded menu {} at {:X} from disk\n",
			std::hash<std::thread::id>{}(std::this_thread::get_id()),
			name,
			(unsigned int)menu
		);

		bool overrode = false;

		if (Game::uiContext)
		{
			for (size_t i = 0; i < ARRAYSIZE(Game::uiContext->menuStack); i++)
			{
				if (Game::uiContext->menuStack[i] &&
					Game::uiContext->menuStack[i]->window.name == name)
				{
					if (OverridenMenus.contains(name))
					{
						assert(OverridenMenus[name] == Game::uiContext->menuStack[i]);
					}
					else
					{
						OverridenMenus[name] = Game::uiContext->menuStack[i];
					}

					Game::uiContext->menuStack[i] = MenusFromDisk[name];

					Components::Logger::Print("[MENUS] {:X} In stack - Overrode menu {} ({:X} => {:X})\n",
						std::hash<std::thread::id>{}(std::this_thread::get_id()),
						name,
						(unsigned int)OverridenMenus[name],
						(unsigned int)MenusFromDisk[name]
					);
				}
			}

			for (size_t i = 0; i < ARRAYSIZE(Game::uiContext->Menus); i++)
			{
				if (Game::uiContext->Menus[i] &&
					Game::uiContext->Menus[i]->window.name == name)
				{
					if (OverridenMenus.contains(name))
					{
						assert(OverridenMenus[name] == Game::uiContext->Menus[i]);
					}
					else
					{
						OverridenMenus[name] = Game::uiContext->Menus[i];
					}

					Game::uiContext->Menus[i] = MenusFromDisk[name];

					Components::Logger::Print("[MENUS] {:X} In context - Overrode menu {} ({:X} => {:X})\n",
						std::hash<std::thread::id>{}(std::this_thread::get_id()),
						name,
						(unsigned int)OverridenMenus[name],
						(unsigned int)MenusFromDisk[name]
					);

					overrode = true;
				}
			}

			if (!overrode)
			{
				// A brand new menu! How fancy!
				OverridenMenus[name] = nullptr;
				Game::uiContext->Menus[Game::uiContext->menuCount] = MenusFromDisk[name];
				Game::uiContext->menuCount++;
			}
		}
	}

	void Menus::Add(const std::string& menu)
	{
		CustomIW4xMenus.push_back(menu);
	}

	Game::Statement_s* Menus::ReallocateExpressionLocally(Game::Statement_s* statement, bool andFree)
	{
		Game::Statement_s* reallocated = nullptr;

		if (statement)
		{
			reallocated = Allocator.allocate<Game::Statement_s>();
			std::memcpy(reallocated, statement, sizeof(Game::Statement_s));

			if (statement->entries)
			{
				assert(reallocated->numEntries);
				reallocated->entries = Allocator.allocateArray<Game::expressionEntry>(reallocated->numEntries);
				std::memcpy(reallocated->entries, statement->entries, sizeof(Game::expressionEntry) * reallocated->numEntries);
			}

			if (andFree)
			{
				Game::free_expression(statement);
			}
		}

		return reallocated;
	}

	void Menus::FreeMenuListOnly(Game::MenuList* menuList)
	{
		Components::Logger::Print("[MENUS] {:X} Freeing only menuList {} at {:X}\n",
			std::hash<std::thread::id>{}(std::this_thread::get_id()),
			menuList->name,
			(unsigned int)menuList
		);

		Allocator.free(menuList->name);
		Allocator.free(menuList->menus);
		Allocator.free(menuList);
	}

	void Menus::FreeMenuOnly(Game::menuDef_t* menu)
	{
		Components::Logger::Print("[MENUS] {:X} Freeing only menu {} at {:X}\n",
			std::hash<std::thread::id>{}(std::this_thread::get_id()),
			menu->window.name,
			(unsigned int)menu
		);

		if (menu->items)
		{
			for (int i = 0; i < menu->itemCount; ++i)
			{
				FreeItem(menu->items[i]);
			}

			Allocator.free(menu->items);
		}

		FreeExpression(menu->visibleExp);
		FreeExpression(menu->rectXExp);
		FreeExpression(menu->rectYExp);
		FreeExpression(menu->rectWExp);
		FreeExpression(menu->rectHExp);
		FreeExpression(menu->openSoundExp);
		FreeExpression(menu->closeSoundExp);

		Allocator.free(menu->window.name);

		Allocator.free(menu);
	}

	void Menus::FreeExpression(Game::Statement_s* statement)
	{
		if (statement)
		{
			if (statement->entries)
			{
				Allocator.free(statement->entries);
			}

			Allocator.free(statement);
		}
	}

	void Menus::UnloadMenuFromDisk(const std::string& menuName)
	{
		const auto menu = MenusFromDisk[menuName];
		PrepareToUnloadMenu(menu);
		FreeMenuOnly(menu);
		MenusFromDisk.erase(menuName);
	}

	void Menus::ReloadDiskMenus()
	{
		const auto connectionState = *reinterpret_cast<Game::connstate_t*>(0xB2C540);

		// We only allow non-menulist menus when we're ingame
		// Otherwise we load a ton of ingame-only menus that are glitched on main_text
		const bool allowStrayMenus = connectionState > Game::connstate_t::CA_DISCONNECTED
			&& Game::CL_IsCgameInitialized();

		Components::Logger::Print("[MENUS] {:X} Reloading disk menus...\n",
			std::hash<std::thread::id>{}(std::this_thread::get_id()));

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

		if (Allocator.empty())
		{
			// good
		}
		else
		{
			__debugbreak();
			Logger::Print("Warning - menu leak? Expected allocator to be empty after reload, but it's not!\n");
		}

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
	}

	Menus::Menus()
	{
		menuParseKeywordHash = reinterpret_cast<Game::KeywordHashEntry<Game::menuDef_t, 128, 3523>**>(0x63AE928);

		if (ZoneBuilder::IsEnabled())
		{
			Game::Menu_Setup(Game::uiContext);
		}

		if (Dedicated::IsEnabled()) return;

		Components::Events::OnCGameInit(ReloadDiskMenus);
		Components::Events::AfterUIInit(ReloadDiskMenus);


		// Don't open connect menu twice - it gets stuck!
		Utils::Hook::Nop(0x428E48, 5);

		// Use the connect menu open call to update server motds
	/*	Utils::Hook(0x428E48, []
			{
				if (!Party::GetMotd().empty() && Party::Target() == *Game::connectedHost)
				{
					Dvar::Var("didyouknow").set(Party::GetMotd());
				}
			}, HOOK_CALL).install()->quick();*/

			// Intercept menu painting
			//Utils::Hook(0x4FFBDF, IsMenuVisible, HOOK_CALL).install()->quick();

		// disable the 2 new tokens in ItemParse_rect (Fix by NTA. Probably because he didn't want to update the menus)
		Utils::Hook::Set<std::uint8_t>(0x640693, 0xEB);

		// make Com_Error and similar go back to main_text instead of menu_xboxlive.
		Utils::Hook::SetString(0x6FC790, "main_text");

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
		Add("ui_mp/pc_options_multi.menu");
		Add("ui_mp/popup_customclan.menu");
		Add("ui_mp/popup_customtitle.menu");
		Add("ui_mp/popup_friends.menu");
		Add("ui_mp/resetclass.menu");
		Add("ui_mp/security_increase_popmenu.menu");
		Add("ui_mp/startup_messages.menu");
		Add("ui_mp/stats_reset.menu");
		Add("ui_mp/stats_unlock.menu");
		Add("ui_mp/theater_menu.menu");
	}

	void Menus::preDestroy()
	{
		// Let Windows handle the memory leaks for you!
	}
}
