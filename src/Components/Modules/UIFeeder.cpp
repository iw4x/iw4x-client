#include <STDInclude.hpp>

namespace Components
{
	UIFeeder::Container UIFeeder::Current;
	std::unordered_map<float, UIFeeder::Callbacks> UIFeeder::Feeders;

	void UIFeeder::Add(float feeder, UIFeeder::GetItemCount_t itemCountCb, UIFeeder::GetItemText_t itemTextCb, UIFeeder::Select_t selectCb)
	{
		UIFeeder::Feeders[feeder] = { itemCountCb, itemTextCb, selectCb };
	}

	const char* UIFeeder::GetItemText()
	{
		if (UIFeeder::Feeders.contains(UIFeeder::Current.feeder))
		{
			return UIFeeder::Feeders[UIFeeder::Current.feeder].getItemText(UIFeeder::Current.index, UIFeeder::Current.column);
		}

		return nullptr;
	}

	unsigned int UIFeeder::GetItemCount()
	{
		if (UIFeeder::Feeders.contains(UIFeeder::Current.feeder))
		{
			return UIFeeder::Feeders[UIFeeder::Current.feeder].getItemCount();
		}

		return 0;
	}

	bool UIFeeder::SetItemSelection()
	{
		if (UIFeeder::Feeders.find(UIFeeder::Current.feeder) != UIFeeder::Feeders.end())
		{
			UIFeeder::Feeders[UIFeeder::Current.feeder].select(UIFeeder::Current.index);
			return true;
		}

		return false;
	}

	bool UIFeeder::CheckFeeder()
	{
		if (UIFeeder::Current.feeder == 15.0f) return false;
		return (UIFeeder::Feeders.find(UIFeeder::Current.feeder) != UIFeeder::Feeders.end());
	}

	__declspec(naked) void UIFeeder::SetItemSelectionStub()
	{
		__asm
		{
			mov eax, [esp + 08h]
			mov UIFeeder::Current.feeder, eax

			mov eax, [esp + 0Ch]
			mov UIFeeder::Current.index, eax

			call UIFeeder::SetItemSelection

			test al, al
			jz continue

			retn

		continue:
			fld ds:739FD0h

			mov eax, 4C25D6h
			jmp eax
		}
	}

	__declspec(naked) void UIFeeder::GetItemTextStub()
	{
		__asm
		{
			mov eax, [esp + 0Ch]
			mov UIFeeder::Current.feeder, eax

			mov eax, [esp + 10h]
			mov UIFeeder::Current.index, eax

			mov eax, [esp + 14h]
			mov UIFeeder::Current.column, eax

			call UIFeeder::GetItemText

			test eax, eax
			jz continue

			push ebx
			mov ebx, [esp + 4 + 28h]
			mov dword ptr[ebx], 0
			pop ebx
			retn

		continue:
			push ecx
			fld ds:739FD0h

			mov eax, 4CE9E7h
			jmp eax
		}
	}

	__declspec(naked) void UIFeeder::GetItemCountStub()
	{
		__asm
		{
			mov eax, [esp + 8h]
			mov UIFeeder::Current.feeder, eax

			call UIFeeder::GetItemCount

			test eax, eax
			jz continue

			retn

		continue:
			push ecx
			fld ds:739FD0h

			mov eax, 41A0D7h
			jmp eax;
		}
	}

	__declspec(naked) void UIFeeder::HandleKeyStub()
	{
		// ReSharper disable once CppEntityNeverUsed
		static int nextClickTime = 0;

		__asm
		{
			mov ebx, ebp
			mov eax, [ebp + 12Ch]
			mov UIFeeder::Current.feeder, eax

			push ebx
			call UIFeeder::CheckFeeder
			pop ebx

			test al, al
			jz continueOriginal

			// Get current milliseconds
			call Game::Sys_Milliseconds

			// Check if allowed to click
			cmp eax, nextClickTime
			jl continueOriginal

			// Set next allowed click time to current time + 300ms
			add eax, 300
			mov nextClickTime, eax

			// Get item cursor position ptr
			mov ecx, ebx
			add ecx, Game::itemDef_s::cursorPos

			// Get item listbox ptr
			mov edx, ebx
			add edx, Game::itemDef_s::typeData

			// Get listbox cursor pos
			mov edx, [edx]
			add edx, Game::listBoxDef_s::mousePos
			mov edx, [edx]

			// Resolve item cursor pos pointer
			mov ebx, [ecx]

			// Check if item cursor pos equals listbox cursor pos
			cmp ebx, edx
			je returnSafe

			// Update indices if not
			mov [ecx], edx
			mov UIFeeder::Current.index, edx

			call UIFeeder::SetItemSelection

		returnSafe:
			retn

		continueOriginal:
			mov eax, 635570h
			jmp eax
		}
	}

	__declspec(naked) void UIFeeder::MouseEnterStub()
	{
		__asm
		{
			mov eax, [edi + 12Ch]
			mov UIFeeder::Current.feeder, eax

			call UIFeeder::CheckFeeder

			test al, al
			jnz continue

			mov [edi + 130h], esi

		continue:
			mov eax, 639D75h
			jmp eax
		}
	}

	__declspec(naked) void UIFeeder::MouseSelectStub()
	{
		__asm
		{
			mov eax, [esp + 08h]
			mov UIFeeder::Current.feeder, eax

			call UIFeeder::CheckFeeder

			test al, al
			jnz continue

			mov eax, 4C25D0h
			jmp eax

		continue:
			retn
		}
	}

	__declspec(naked) void UIFeeder::PlaySoundStub()
	{
		__asm
		{
			mov eax, [edi + 12Ch]
			mov UIFeeder::Current.feeder, eax

			call UIFeeder::CheckFeeder

			test al, al
			jnz continue

			mov eax, 685E10h
			jmp eax

		continue:
			retn
		}
	}

	void UIFeeder::Select(float feeder, unsigned int index)
	{
		if (Game::uiContext->openMenuCount > 0)
		{
			Game::menuDef_t* menu = Game::uiContext->menuStack[Game::uiContext->openMenuCount - 1];

			if (menu && menu->items)
			{
				for (int i = 0; i < menu->itemCount; ++i)
				{
					Game::itemDef_s* item = menu->items[i];
					if (item && item->type == 6 && item->special == feeder)
					{
						item->cursorPos[0] = static_cast<int>(index);
						break;
					}
				}
			}
		}
	}

	unsigned int UIFeeder::GetMapCount()
	{
		Game::UI_UpdateArenas();
		Game::UI_SortArenas();
		return *Game::arenaCount;
	}

	const char* UIFeeder::GetMapText(unsigned int index, int /*column*/)
	{
		Game::UI_UpdateArenas();
		Game::UI_SortArenas();

		if (index < static_cast<unsigned int>(*Game::arenaCount))
		{
			return Game::SEH_StringEd_GetString(ArenaLength::NewArenas[reinterpret_cast<int*>(0x633E934)[index]].uiName);
		}

		return "";
	}

	void UIFeeder::SelectMap(unsigned int index)
	{
		Game::UI_UpdateArenas();
		Game::UI_SortArenas();

		if (index < static_cast<unsigned int>(*Game::arenaCount))
		{
			index = reinterpret_cast<int*>(0x633E934)[index];
			const char* mapname = ArenaLength::NewArenas[index].mapName;
			const char* longname = ArenaLength::NewArenas[index].uiName;
			const char* description = ArenaLength::NewArenas[index].description;

			Dvar::Var("ui_map_name").set(mapname);
			Dvar::Var("ui_map_long").set(Game::SEH_StringEd_GetString(longname));
			Dvar::Var("ui_map_desc").set(Game::SEH_StringEd_GetString(description));
		}
	}

	void UIFeeder::ApplyMap([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
	{
		const auto mapname = Dvar::Var("ui_map_name").get<std::string>();

		Dvar::Var("ui_mapname").set(mapname);
		Utils::Hook::Call<void(const char*)>(0x503B50)(mapname.data()); // Party_SetDisplayMapName
	}

	void UIFeeder::ApplyInitialMap([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
	{
		const auto mapname = Dvar::Var("ui_mapname").get<std::string>();

		Game::UI_UpdateArenas();
		Game::UI_SortArenas();

		for (unsigned int i = 0; i < static_cast<unsigned int>(*Game::arenaCount); ++i)
		{
			if (ArenaLength::NewArenas[i].mapName == mapname)
			{
				for (unsigned int j = 0; j < static_cast<unsigned int>(*Game::arenaCount); ++j)
				{
					if (reinterpret_cast<unsigned int*>(0x633E934)[j] == i)
					{
						UIFeeder::SelectMap(j);
						UIFeeder::Select(60.0f, j);
						break;
					}
				}

				break;
			}
		}
	}

	int UIFeeder::CheckSelection(int feeder)
	{
		// Changelog and server info (player list) should not have the hover effect
		if (feeder == 62 || feeder == 13) return 0;
		return 1;
	}

	__declspec(naked) void UIFeeder::CheckSelectionStub()
	{
		__asm
		{
			mov ecx, 6B5240h
			call ecx // __ftol2_sse

			push eax
			call UIFeeder::CheckSelection

			test al, al
			jz returnSafe

			pop eax
			push 635A9Dh
			retn

		returnSafe:
			pop eax
			xor eax, eax
			pop ecx
			retn
		}
	}

	UIFeeder::UIFeeder()
	{
		if (Dedicated::IsEnabled()) return;

		Scheduler::Once([]
		{
			Dvar::Register<const char*>("ui_map_long", "Afghan", Game::DVAR_NONE, "");
			Dvar::Register<const char*>("ui_map_name", "mp_afghan", Game::DVAR_NONE, "");
			Dvar::Register<const char*>("ui_map_desc", "", Game::DVAR_NONE, "");
		}, Scheduler::Pipeline::MAIN);

		// Get feeder item count
		Utils::Hook(0x41A0D0, UIFeeder::GetItemCountStub, HOOK_JUMP).install()->quick();

		// Get feeder item text
		Utils::Hook(0x4CE9E0, UIFeeder::GetItemTextStub, HOOK_JUMP).install()->quick();

		// Select feeder item
		Utils::Hook(0x4C25D0, UIFeeder::SetItemSelectionStub, HOOK_JUMP).install()->quick();

		// Mouse enter check
		Utils::Hook(0x639D6E, UIFeeder::MouseEnterStub, HOOK_JUMP).install()->quick();

		// Handle key event
		Utils::Hook(0x63C5BC, UIFeeder::HandleKeyStub, HOOK_CALL).install()->quick();

		// Mouse select check
		Utils::Hook(0x639D31, UIFeeder::MouseSelectStub, HOOK_CALL).install()->quick();

		// Play mouse over sound check
		Utils::Hook(0x639D66, UIFeeder::PlaySoundStub, HOOK_CALL).install()->quick();

		// Disable blinking
		Utils::Hook(0x635A98, UIFeeder::CheckSelectionStub, HOOK_JUMP).install()->quick();

		// some thing overwriting feeder 2's data
		Utils::Hook::Set<BYTE>(0x4A06A9, 0xEB);

		// Use feeder 5 for maps, as feeder 4 selects on mouse over
		UIFeeder::Add(60.0f, UIFeeder::GetMapCount, UIFeeder::GetMapText, UIFeeder::SelectMap);
		UIScript::Add("ApplyMap", UIFeeder::ApplyMap);
		UIScript::Add("ApplyInitialMap", UIFeeder::ApplyInitialMap);

		// Fix feeder focus
		//Utils::Hook::Nop(0x63B1DD, 2); // Flag 4 check (WINDOW_VISIBLE)
		Utils::Hook::Nop(0x63B1E6, 2); // Flag 2 check (WINDOW_HASFOCUS)
		Utils::Hook::Nop(0x63B1F7, 2); // Flag 2000 check (WINDOW_LB_THUMB?)
	}

	UIFeeder::~UIFeeder()
	{
		UIFeeder::Feeders.clear();
	}
}
