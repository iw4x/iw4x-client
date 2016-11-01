#include "STDInclude.hpp"

namespace Components
{
	UIFeeder::Container UIFeeder::Current;
	std::map<float, UIFeeder::Callbacks> UIFeeder::Feeders;

	void UIFeeder::Add(float feeder, UIFeeder::GetItemCount_t itemCountCb, UIFeeder::GetItemText_t itemTextCb, UIFeeder::Select_t selectCb)
	{
		UIFeeder::Feeders[feeder] = { itemCountCb, itemTextCb, selectCb };
	}

	const char* UIFeeder::GetItemText()
	{
		if (UIFeeder::Feeders.find(UIFeeder::Current.Feeder) != UIFeeder::Feeders.end())
		{
			return UIFeeder::Feeders[UIFeeder::Current.Feeder].GetItemText(UIFeeder::Current.Index, UIFeeder::Current.Column);
		}

		return nullptr;
	}

	unsigned int UIFeeder::GetItemCount()
	{
		if (UIFeeder::Feeders.find(UIFeeder::Current.Feeder) != UIFeeder::Feeders.end())
		{
			return UIFeeder::Feeders[UIFeeder::Current.Feeder].GetItemCount();
		}

		return 0;
	}

	bool UIFeeder::SetItemSelection()
	{
		if (UIFeeder::Feeders.find(UIFeeder::Current.Feeder) != UIFeeder::Feeders.end())
		{
			UIFeeder::Feeders[UIFeeder::Current.Feeder].Select(UIFeeder::Current.Index);
			return true;
		}

		return false;
	}

	bool UIFeeder::CheckFeeder()
	{
		if (UIFeeder::Current.Feeder == 15.0f) return false;
		return (UIFeeder::Feeders.find(UIFeeder::Current.Feeder) != UIFeeder::Feeders.end());
	}

	__declspec(naked) void UIFeeder::SetItemSelectionStub()
	{
		__asm
		{
			mov eax, [esp + 08h]
			mov UIFeeder::Current.Feeder, eax

			mov eax, [esp + 0Ch]
			mov UIFeeder::Current.Index, eax

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
			mov UIFeeder::Current.Feeder, eax

			mov eax, [esp + 10h]
			mov UIFeeder::Current.Index, eax

			mov eax, [esp + 14h]
			mov UIFeeder::Current.Column, eax

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
			mov UIFeeder::Current.Feeder, eax

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
		static int NextClickTime = 0;

		__asm
		{
			mov ebx, ebp
			mov eax, [ebp + 12Ch]
			mov UIFeeder::Current.Feeder, eax

			push ebx
			call UIFeeder::CheckFeeder
			pop ebx

			test al, al
			jz continueOriginal

			// Get current milliseconds
			call Game::Sys_Milliseconds

			// Check if allowed to click
			cmp eax, NextClickTime
			jl continueOriginal

			// Set next allowed click time to current time + 300ms
			add eax, 300
			mov NextClickTime, eax

			// Get item cursor position ptr
			mov ecx, ebx
			add ecx, Game::itemDef_t::cursorPos

			// Get item listbox ptr
			mov edx, ebx
			add edx, Game::itemDef_t::typeData

			// Get listbox cursor pos
			mov edx, [edx]
			add edx, Game::listBoxDef_s::startPos
			mov edx, [edx]

			// Resolve item cursor pos pointer
			mov ebx, [ecx]

			// Check if item cursor pos equals listbox cursor pos
			cmp ebx, edx
			je returnSafe

			// Update indices if not
			mov [ecx], edx
			mov UIFeeder::Current.Index, edx

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
			mov UIFeeder::Current.Feeder, eax

			call UIFeeder::CheckFeeder

			test al, al
			jnz continue

			mov[edi + 130h], esi

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
			mov UIFeeder::Current.Feeder, eax

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
			mov UIFeeder::Current.Feeder, eax

			call UIFeeder::CheckFeeder

			test al, al
			jnz continue

			mov eax, 685E10h
			jmp eax

		continue:
			retn
		}
	}

	void UIFeeder::ApplyMapFeeder(Game::dvar_t* dvar, int num)
	{
		Dvar::Var(dvar).Set(num);

		if (num < 0 || num >= *Game::arenaCount)
		{
			num = 0;
		}

		// UI_SortArenas
		Utils::Hook::Call<void()>(0x630AE0)();

		const char* mapname = ArenaLength::NewArenas[reinterpret_cast<int*>(0x633E934)[num]].mapName;

		Dvar::Var("ui_mapname").Set(mapname);

		// Party_SetDisplayMapName
		Utils::Hook::Call<void(const char*)>(0x503B50)(mapname);
	}

	UIFeeder::UIFeeder()
	{
		// Get feeder item count
		Utils::Hook(0x41A0D0, UIFeeder::GetItemCountStub, HOOK_JUMP).Install()->Quick();

		// Get feeder item text
		Utils::Hook(0x4CE9E0, UIFeeder::GetItemTextStub, HOOK_JUMP).Install()->Quick();

		// Select feeder item
		Utils::Hook(0x4C25D0, UIFeeder::SetItemSelectionStub, HOOK_JUMP).Install()->Quick();

		// Mouse enter check
		Utils::Hook(0x639D6E, UIFeeder::MouseEnterStub, HOOK_JUMP).Install()->Quick();

		// Handle key event
		Utils::Hook(0x63C5BC, UIFeeder::HandleKeyStub, HOOK_CALL).Install()->Quick();

		// Mouse select check
		Utils::Hook(0x639D31, UIFeeder::MouseSelectStub, HOOK_CALL).Install()->Quick();

		// Play mouse over sound check
		Utils::Hook(0x639D66, UIFeeder::PlaySoundStub, HOOK_CALL).Install()->Quick();

		// some thing overwriting feeder 2's data
		Utils::Hook::Set<BYTE>(0x4A06A9, 0xEB);

		// correct feeder 4
		Utils::Hook(0x4C260E, UIFeeder::ApplyMapFeeder, HOOK_CALL).Install()->Quick();
	}

	UIFeeder::~UIFeeder()
	{
		UIFeeder::Feeders.clear();
	}
}
