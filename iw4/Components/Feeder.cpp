#include "..\STDInclude.hpp"

namespace Components
{
	Feeder::Container Feeder::Current;
	std::map<float, Feeder::Callbacks> Feeder::Feeders;

	void Feeder::Add(float feeder, Feeder::GetItemCount_t itemCountCb, Feeder::GetItemText_t itemTextCb, Feeder::Select_t selectCb)
	{
		Feeder::Feeders[feeder] = { itemCountCb, itemTextCb, selectCb };
	}

	const char* Feeder::GetItemText()
	{
		if (Feeder::Feeders.find(Feeder::Current.Num) != Feeder::Feeders.end())
		{
			return Feeder::Feeders[Feeder::Current.Num].GetItemText(Feeder::Current.Index, Feeder::Current.Column);
		}

		return nullptr;
	}

	int Feeder::GetItemCount()
	{
		if (Feeder::Feeders.find(Feeder::Current.Num) != Feeder::Feeders.end())
		{
			return Feeder::Feeders[Feeder::Current.Num].GetItemCount();
		}

		return 0;
	}

	bool Feeder::SetItemSelection()
	{
		if (Feeder::Feeders.find(Feeder::Current.Num) != Feeder::Feeders.end())
		{
			Feeder::Feeders[Feeder::Current.Num].Select(Feeder::Current.Index);
			return true;
		}

		return false;
	}

	bool Feeder::CheckFeeder()
	{
		if (Feeder::Current.Num == 15.0f) return false;
		return (Feeder::Feeders.find(Feeder::Current.Num) != Feeder::Feeders.end());
	}

	void __declspec(naked) Feeder::SetItemSelectionStub()
	{
		__asm
		{
			mov eax, [esp + 08h]
			mov Feeder::Current.Num, eax

			mov eax, [esp + 0Ch]
			mov Feeder::Current.Index, eax

			call Feeder::SetItemSelection

			test eax, eax
			jz continue

			retn

		continue:
			fld ds : 739FD0h

			mov eax, 4C25D6h
			jmp eax
		}
	}

	void __declspec(naked) Feeder::GetItemTextStub()
	{
		__asm
		{
			mov eax, [esp + 0Ch]
			mov Feeder::Current.Num, eax

			mov eax, [esp + 10h]
			mov Feeder::Current.Index, eax

			mov eax, [esp + 14h]
			mov Feeder::Current.Column, eax

			call Feeder::GetItemText

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

	void __declspec(naked) Feeder::GetItemCountStub()
	{
		__asm
		{
			mov eax, [esp + 8h]
			mov Feeder::Current.Num, eax

			call Feeder::GetItemCount

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

	void __declspec(naked) Feeder::HandleKeyStub()
	{
		static int NextClickTime = 0;

		__asm
		{
			mov ebx, ebp
			mov eax, [ebp + 12Ch]
			mov Feeder::Current.Num, eax

			push ebx
			call Feeder::CheckFeeder
			pop ebx

			test eax, eax
			jz continueOriginal

			// Get current milliseconds
			call Game::Com_Milliseconds

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
			mov Feeder::Current.Index, edx

			call Feeder::SetItemSelection

		returnSafe:
			retn

		continueOriginal:
			mov eax, 635570h
			jmp eax
		}
	}

	void __declspec(naked) Feeder::MouseEnterStub()
	{
		__asm
		{
			mov eax, [edi + 12Ch]
			mov Feeder::Current.Num, eax

			call Feeder::CheckFeeder

			test eax, eax
			jnz continue

			mov[edi + 130h], esi

		continue:
			mov eax, 639D75h
			jmp eax
		}
	}

	void __declspec(naked) Feeder::MouseSelectStub()
	{
		__asm
		{
			mov eax, [esp + 08h]
			mov Feeder::Current.Num, eax

			call Feeder::CheckFeeder

			test eax, eax
			jnz continue

			mov eax, 4C25D0h
			jmp eax

		continue:
			retn
		}
	}

	void __declspec(naked) Feeder::PlaySoundStub()
	{
		__asm
		{
			mov eax, [edi + 12Ch]
			mov Feeder::Current.Num, eax

			call Feeder::CheckFeeder

			test eax, eax
			jnz continue

			mov eax, 685E10h
			jmp eax

		continue:
			retn
		}
	}

	Feeder::Feeder()
	{
		// Get feeder item count
		Utils::Hook(0x41A0D0, Feeder::GetItemCountStub, HOOK_JUMP).Install()->Quick();

		// Get feeder item text
		Utils::Hook(0x4CE9E0, Feeder::GetItemTextStub, HOOK_JUMP).Install()->Quick();

		// Select feeder item
		Utils::Hook(0x4C25D0, Feeder::SetItemSelectionStub, HOOK_JUMP).Install()->Quick();

		// Mouse enter check
		Utils::Hook(0x639D6E, Feeder::MouseEnterStub, HOOK_JUMP).Install()->Quick();

		// Handle key event
		Utils::Hook(0x63C5BC, Feeder::HandleKeyStub, HOOK_CALL).Install()->Quick();

		// Mouse select check
		Utils::Hook(0x639D31, Feeder::MouseSelectStub, HOOK_CALL).Install()->Quick();

		// Play mouse over sound check
		Utils::Hook(0x639D66, Feeder::PlaySoundStub, HOOK_CALL).Install()->Quick();

		// some thing overwriting feeder 2's data
		Utils::Hook::Set<BYTE>(0x4A06A9, 0xEB);
	}

	Feeder::~Feeder()
	{
		Feeder::Feeders.clear();
	}
}
