#include <STDInclude.hpp>

#include "Events.hpp"
#include "UIFeeder.hpp"

namespace Components
{
	UIFeeder::Container UIFeeder::Current;
	std::unordered_map<float, UIFeeder::Callbacks> UIFeeder::Feeders;

	Dvar::Var UIFeeder::UIMapLong;
	Dvar::Var UIFeeder::UIMapName;
	Dvar::Var UIFeeder::UIMapDesc;

	void UIFeeder::Add(float feeder, GetItemCount_t itemCountCb, GetItemText_t itemTextCb, Select_t selectCb)
	{
		Feeders[feeder] = { itemCountCb, itemTextCb, selectCb };
	}

	const char* UIFeeder::GetItemText()
	{
		if (Feeders.contains(Current.feeder))
		{
			return Feeders[Current.feeder].getItemText(Current.index, Current.column);
		}

		return nullptr;
	}

	unsigned int UIFeeder::GetItemCount()
	{
		if (Feeders.contains(Current.feeder))
		{
			return Feeders[Current.feeder].getItemCount();
		}

		return 0;
	}

	bool UIFeeder::SetItemSelection()
	{
		if (Feeders.contains(Current.feeder))
		{
			Feeders[Current.feeder].select(Current.index);
			return true;
		}

		return false;
	}

	bool UIFeeder::CheckFeeder()
	{
		if (Current.feeder == 15.0f) return false;
		return Feeders.contains(Current.feeder);
	}

	__declspec(naked) void UIFeeder::SetItemSelectionStub()
	{
		__asm
		{
			mov eax, [esp + 08h]
			mov Current.feeder, eax

			mov eax, [esp + 0Ch]
			mov Current.index, eax

			call SetItemSelection

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
			mov Current.feeder, eax

			mov eax, [esp + 10h]
			mov Current.index, eax

			mov eax, [esp + 14h]
			mov Current.column, eax

			call GetItemText

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
			mov Current.feeder, eax

			call GetItemCount

			test eax, eax
			jz continue

			retn

		continue:
			push ecx
			fld ds:739FD0h

			mov eax, 41A0D7h
			jmp eax
		}
	}

	__declspec(naked) void UIFeeder::HandleKeyStub()
	{
		static int nextClickTime = 0;

		__asm
		{
			mov ebx, ebp
			mov eax, [ebp + 12Ch]
			mov Current.feeder, eax

			push ebx
			call CheckFeeder
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
			mov Current.index, edx

			call SetItemSelection

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
			mov Current.feeder, eax

			call CheckFeeder

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
			mov Current.feeder, eax

			call CheckFeeder

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
			mov Current.feeder, eax

			call CheckFeeder

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
			auto* menu = Game::uiContext->menuStack[Game::uiContext->openMenuCount - 1];

			if (menu && menu->items)
			{
				for (int i = 0; i < menu->itemCount; ++i)
				{
					auto* item = menu->items[i];
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
		return Maps::GetCustomMaps().size();
	}

	const char* UIFeeder::GetMapText(unsigned int index, int /*column*/)
	{
		const auto& maps = Maps::GetCustomMaps();
		if (index < maps.size())
		{
			return maps.at(index).data();
		}

#ifdef _DEBUG
		if (IsDebuggerPresent())
		{
			__debugbreak();
		}
#endif

		return "";
	}

	void UIFeeder::SelectMap(unsigned int index)
	{
		const auto& maps = Maps::GetCustomMaps();

		if (index < maps.size())
		{
			std::string mapName = maps[index];

			std::string longName = mapName;
			std::string description = "(Missing arena file!)";

			const auto& arenaPath = Maps::GetArenaPath(mapName);

			if (Utils::IO::FileExists(arenaPath))
			{
				const auto& arena = Maps::ParseCustomMapArena(Utils::IO::ReadFile(arenaPath));

				if (arena.contains("longname"))
				{
					longName = arena.at("longname");
				}

				if (arena.contains("map"))
				{
					mapName = arena.at("map");
				}

				if (arena.contains("description"))
				{
					description = arena.at("description");
				}
			}

			UIMapName.set(Localization::Get(mapName.data()));
			UIMapLong.set(Localization::Get(longName.data()));
			UIMapDesc.set(Localization::Get(description.data()));
		}
	}

	void UIFeeder::ApplyMap([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
	{
		const auto* mapname = Dvar::Var("ui_map_name").get<const char*>();

		if (*mapname)
		{
			Game::Dvar_SetString(*Game::ui_mapname, mapname);
			Utils::Hook::Call<void(const char*)>(0x503B50)(mapname); // Party_SetDisplayMapName
		}
	}

	void UIFeeder::ApplyInitialMap([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
	{
		Maps::ScanCustomMaps();
		Select(60.0f, 0); // Will select nothing if there's no map
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
			call CheckSelection

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

		Events::OnDvarInit([]
		{
			UIMapLong = Dvar::Register<const char*>("ui_map_long", "", Game::DVAR_NONE, "");
			UIMapName = Dvar::Register<const char*>("ui_map_name", "", Game::DVAR_NONE, "");
			UIMapDesc = Dvar::Register<const char*>("ui_map_desc", "", Game::DVAR_NONE, "");
		});

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
