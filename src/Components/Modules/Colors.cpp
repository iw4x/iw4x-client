#include "STDInclude.hpp"

// -- Additional colors --
//
// Colors are resolved using ColorIndex().
// It resolves the colorTable entry using the ASCII value.
// If we want to add colors, we have to use correct ASCII chars.
// As the last value is 0x39 (9), we have to go on with 0x3A (:)
// So the next chars would be:
// 0x3A (:), 0x3B (;), 0x3C (<), 0x3D (=), ...
//
// The problem though is that I_CleanString doesn't know we added colors, so we have to adapt that as well!

namespace Components
{
	Dvar::Var Colors::NewColors;
	std::vector<DWORD> Colors::ColorTable;

	void Colors::Strip(const char* in, char* out, int max)
	{
		if (!in || !out) return;

		max--;
		int current = 0;
		while (*in != 0 && current < max)
		{
			char index = *(in + 1);
			if (*in == '^' && (Colors::ColorIndex(index) != 7 || index == '7')) // Add 1 new color for now
			{
				in++;
			}
			else
			{
				*out = *in;
				out++;
				current++;
			}
			in++;
		}
		*out = '\0';
	}

	std::string Colors::Strip(std::string in)
	{
		char buffer[1000] = { 0 }; // Should be more than enough
		Colors::Strip(in.data(), buffer, sizeof(buffer));
		return std::string(buffer);
	}

	void __declspec(naked) Colors::ClientUserinfoChanged()
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

	char* Colors::GetClientName(int localClientNum, int index, char *buf, size_t size)
	{
		Game::CL_GetClientName(localClientNum, index, buf, size);

		// Remove the colors
		strncpy(buf, Colors::Strip(buf).data(), size);

		return buf;
	}

	void Colors::PatchColorLimit(char limit)
	{
		Utils::Hook::Set<char>(0x535629, limit); // DrawText2d
		Utils::Hook::Set<char>(0x4C1BE4, limit); // No idea :P
		Utils::Hook::Set<char>(0x4863DD, limit); // No idea :P
		Utils::Hook::Set<char>(0x486429, limit); // No idea :P
		Utils::Hook::Set<char>(0x49A5A8, limit); // No idea :P
		Utils::Hook::Set<char>(0x505721, limit); // R_TextWidth
		Utils::Hook::Set<char>(0x505801, limit); // No idea :P
		Utils::Hook::Set<char>(0x50597F, limit); // No idea :P
		Utils::Hook::Set<char>(0x5815DB, limit); // No idea :P
		Utils::Hook::Set<char>(0x592ED0, limit); // No idea :P
		Utils::Hook::Set<char>(0x5A2E2E, limit); // No idea :P

		Utils::Hook::Set<char>(0x5A2733, limit - '0'); // No idea :P
	}

	char Colors::Add(uint8_t r, uint8_t g, uint8_t b)
	{
		char index = '0' + static_cast<char>(Colors::ColorTable.size());
		Colors::ColorTable.push_back(RGB(r, g, b));
		Colors::PatchColorLimit(index);
		return index;
	}

	unsigned int Colors::ColorIndex(unsigned char index)
	{
		unsigned int result = index - '0';
		if (result >= Colors::ColorTable.size() || result < 0) result = 7;
		return result;
	}

	void Colors::LookupColor(DWORD* color, char index)
	{
		if (index == '8') // Color 8
		{
			*color = *reinterpret_cast<DWORD*>(0x66E5F70);
		}
		else if (index == '9') // Color 9
		{
			*color = *reinterpret_cast<DWORD*>(0x66E5F74);
		}
		else
		{
			int clrIndex = Colors::ColorIndex(index);

			// Use native colors
			if (clrIndex <= 7 && !Colors::NewColors.Get<bool>())
			{
				*color = reinterpret_cast<DWORD*>(0x78DC70)[index - 48];
			}
			else
			{
				*color = Colors::ColorTable[clrIndex];
			}
		}
	}

	char* Colors::CleanStrStub(char* string)
	{
		Colors::Strip(string, string, strlen(string));
		return string;
	}

	void __declspec(naked) Colors::LookupColorStub()
	{
		__asm
		{
			push ebx
			push [esp + 8h] // Index
			push esi        // Color ref
			call Colors::LookupColor
			add esp, 8h
			pop ebx
			retn
		}
	}

	Colors::Colors()
	{
		// Allow colored names ingame
		Utils::Hook(0x5D8B40, Colors::ClientUserinfoChanged, HOOK_JUMP).Install()->Quick();

		// Though, don't apply that to overhead names.
		Utils::Hook(0x581932, Colors::GetClientName, HOOK_CALL).Install()->Quick();

		// Patch RB_LookupColor
		Utils::Hook(0x534CD0, Colors::LookupColorStub, HOOK_JUMP).Install()->Quick();

		// Patch ColorIndex
		Utils::Hook(0x417770, Colors::ColorIndex, HOOK_JUMP).Install()->Quick();

		// Patch I_CleanStr
		Utils::Hook(0x4AD470, Colors::CleanStrStub, HOOK_JUMP).Install()->Quick();

		// Register dvar
		Colors::NewColors = Dvar::Register<bool>("cg_newColors", true, Game::dvar_flag::DVAR_FLAG_SAVED, "Use Warfare² color code style.");

		// Add our colors
		Colors::Add(0, 0, 0);       // 0  - Black
		Colors::Add(255, 49, 49);   // 1  - Red
		Colors::Add(134, 192, 0);   // 2  - Green
		Colors::Add(255, 173, 34);  // 3  - Yellow
		Colors::Add(0, 135, 193);   // 4  - Blue
		Colors::Add(32, 197, 255);  // 5  - Light Blue
		Colors::Add(151, 80, 221);  // 6  - Pink

		Colors::Add(255, 255, 255); // 7  - White

		Colors::Add(0, 0, 0);       // 8  - Team color (axis?)
		Colors::Add(0, 0, 0);       // 9  - Team color (allies?)

		// Custom colors
		Colors::Add(211, 84, 0);    // 10 - Orange (:)
		Colors::Add(0, 255, 200);   // 11 - Turqoise (;) - using that color in infostrings (e.g. your name) fails, ';' is an illegal character!
	}

	Colors::~Colors()
	{
		Colors::ColorTable.clear();
	}
}
