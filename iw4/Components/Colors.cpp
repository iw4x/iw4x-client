#include "..\STDInclude.hpp"

namespace Components
{
	Dvar::Var Colors::NewColors;

	void Colors::Strip(const char* in, char* out, int max)
	{
		max--;
		int current = 0;
		while (*in != 0 && current < max)
		{
			if (!Q_IsColorString(in))
			{
				*out = *in;
				out++;
				current++;
			}
			else
			{
				*in++;
			}
			*in++;
		}
		*out = '\0';
	}

	void __declspec(naked) Colors::ClientUserinfoChanged(int length)
	{
		__asm
		{
			mov eax, [esp + 4h] // length
			sub eax, 1
			push eax

			push ecx // name
			push edx // buffer

			call strncpy

			add esp, 0Ch
			retn
		}
	}

	char* Colors::CL_GetClientName(int a1, int a2, char* buffer, size_t _length)
	{
		__asm
		{
			push _length
			push buffer
			push a2
			push a1
			mov eax, 4563D0h
			call eax
			add esp, 10h
		}

		// Remove the colors
		char tempBuffer[100] = { 0 };
		Colors::Strip(buffer, tempBuffer, _length);
		strncpy(buffer, tempBuffer, _length);

		return buffer;
	}

	void Colors::UpdateColorTable()
	{
		static int LastState = 2;
		static DWORD DefaultTable[8] = { 0 };
		DWORD* gColorTable = (DWORD*)0x78DC70;

		if (LastState == 2)
		{
			memcpy(DefaultTable, gColorTable, sizeof(DefaultTable));
		}

		if (Colors::NewColors.Get<bool>() && (0xF & (int)Colors::NewColors.Get<bool>()) != LastState)
		{
			// Apply NTA's W² colors :3 (slightly modified though^^)
			gColorTable[1] = RGB(255, 49, 49);
			gColorTable[2] = RGB(134, 192, 0);
			gColorTable[3] = RGB(255, 173, 34);
			gColorTable[4] = RGB(0, 135, 193);
			gColorTable[5] = RGB(32, 197, 255);
			gColorTable[6] = RGB(151, 80, 221);

			LastState = Colors::NewColors.Get<bool>();
		}
		else if (!Colors::NewColors.Get<bool>() && (0xF & (int)Colors::NewColors.Get<bool>()) != LastState)
		{
			memcpy(gColorTable, DefaultTable, sizeof(DefaultTable));

			LastState = Colors::NewColors.Get<bool>();
		}
	}

	Colors::Colors()
	{
		// Allow colored names ingame
		Utils::Hook(0x5D8B40, Colors::ClientUserinfoChanged, HOOK_JUMP).Install()->Quick();

		// Though, don't apply that to overhead names.
		Utils::Hook(0x581932, Colors::CL_GetClientName, HOOK_CALL).Install()->Quick();

		// Set frame handler
		Renderer::OnFrame(Colors::UpdateColorTable);

		// Register dvar
		Colors::NewColors = Dvar::Register<bool>("cg_newColors", true, Game::dvar_flag::DVAR_FLAG_SAVED, "Use Warfare² color code style.");
	}
}
