#include "..\STDInclude.hpp"

namespace Components
{
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

	Colors::Colors()
	{
		DWORD* color_table = (DWORD*)0x78DC70;

		// Apply NTA's W² colors :3 (slightly modified though^^)
		color_table[1] = RGB(255, 49, 49);
		color_table[2] = RGB(134, 192, 0);
		color_table[3] = RGB(255, 173, 34);
		color_table[4] = RGB(0, 135, 193);
		color_table[5] = RGB(32, 197, 255);
		color_table[6] = RGB(151, 80, 221);

		// Allow colored names ingame
		Utils::Hook(0x5D8B40, Colors::ClientUserinfoChanged, HOOK_JUMP).Install()->Quick();

		// Though, don't apply that to overhead names.
		Utils::Hook(0x581932, Colors::CL_GetClientName, HOOK_CALL).Install()->Quick();
	}
}
