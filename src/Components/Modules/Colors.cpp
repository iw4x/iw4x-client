#include "STDInclude.hpp"

namespace Components
{
	char Colors::LastColorIndex;
	Dvar::Var Colors::NewColors;
	Dvar::Var Colors::ColorBlind;
	Game::dvar_t* Colors::ColorAllyColorBlind;
	Game::dvar_t* Colors::ColorEnemyColorBlind;

	std::vector<DWORD> Colors::ColorTable;

	DWORD Colors::HsvToRgb(Colors::HsvColor hsv)
	{
		DWORD rgb;
		unsigned char region, p, q, t;
		unsigned int h, s, v, remainder;

		if (hsv.s == 0)
		{
			rgb = RGB(hsv.v, hsv.v, hsv.v);
			return rgb;
		}

		// converting to 16 bit to prevent overflow
		h = hsv.h;
		s = hsv.s;
		v = hsv.v;

		region = static_cast<uint8_t>(h / 43);
		remainder = (h - (region * 43)) * 6;

		p = static_cast<uint8_t>((v * (255 - s)) >> 8);
		q = static_cast<uint8_t>((v * (255 - ((s * remainder) >> 8))) >> 8);
		t = static_cast<uint8_t>((v * (255 - ((s * (255 - remainder)) >> 8))) >> 8);

		switch (region)
		{
		case 0:
			rgb = RGB(v, t, p);
			break;
		case 1:
			rgb = RGB(q, v, p);
			break;
		case 2:
			rgb = RGB(p, v, t);
			break;
		case 3:
			rgb = RGB(p, q, v);
			break;
		case 4:
			rgb = RGB(t, p, v);
			break;
		default:
			rgb = RGB(v, p, q);
			break;
		}

		return rgb;
	}

	void Colors::Strip(const char* in, char* out, int max)
	{
		if (!in || !out) return;

		max--;
		int current = 0;
		while (*in != 0 && current < max)
		{
			char index = *(in + 1);
			if (*in == '^' && (Colors::ColorIndex(index) != 7 || index == '7'))
			{
				++in;
			}
			else
			{
				*out = *in;
				++out;
				++current;
			}

			++in;
		}
		*out = '\0';
	}

	std::string Colors::Strip(const std::string& in)
	{
		char buffer[1000] = { 0 }; // Should be more than enough
		Colors::Strip(in.data(), buffer, sizeof(buffer));
		return std::string(buffer);
	}

	void Colors::UserInfoCopy(char* buffer, const char* name, size_t size)
	{
		Utils::Memory::Allocator allocator;

		if (!Dvar::Var("sv_allowColoredNames").get<bool>())
		{
			Colors::Strip(name, buffer, size);
		}
		else
		{
			strncpy_s(buffer, size, name, _TRUNCATE);
		}
	}

	__declspec(naked) void Colors::ClientUserinfoChanged()
	{
		__asm
		{
			mov eax, [esp + 4h] // length
			//sub eax, 1
			push eax

			push ecx // name
			push edx // buffer

			call Colors::UserInfoCopy

			add esp, 0Ch
			retn
		}
	}

	char* Colors::GetClientName(int localClientNum, int index, char *buf, size_t size)
	{
		Game::CL_GetClientName(localClientNum, index, buf, size);

		// Append clantag to username & remove the colors
		strncpy_s(buf, size, Colors::Strip(ClanTags::GetUserClantag(index, buf)).data(), size);

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

		LastColorIndex = limit;
	}

	char Colors::Add(uint8_t r, uint8_t g, uint8_t b)
	{
		char index = '0' + static_cast<char>(Colors::ColorTable.size());
		Colors::ColorTable.push_back(RGB(r, g, b));
		Colors::PatchColorLimit(index);
		return index;
	}

	unsigned int Colors::ColorIndex(char index)
	{
		char result = index - '0';
		if (static_cast<unsigned int>(result) >= Colors::ColorTable.size() || result < 0) result = 7;
		return result;
	}

	void Colors::LookupColor(DWORD* color, char index)
	{
		*color = RGB(255, 255, 255);

		if (index == '8') // Color 8
		{
			*color = *reinterpret_cast<DWORD*>(0x66E5F70);
		}
		else if (index == '9') // Color 9
		{
			*color = *reinterpret_cast<DWORD*>(0x66E5F74);
		}
		else if (index == ':')
		{
			*color = Colors::HsvToRgb({ static_cast<uint8_t>((Game::Sys_Milliseconds() / 200) % 256), 255,255 });
		}
		else if (index == ';')
		{
			float fltColor[4];
			Game::Dvar_GetUnpackedColorByName("sv_customTextColor", fltColor);
			*color = RGB(fltColor[0] * 255, fltColor[1] * 255, fltColor[2] * 255);
		}
		else
		{
			int clrIndex = Colors::ColorIndex(index);

			// Use native colors
			if (clrIndex <= 7 && !Colors::NewColors.get<bool>())
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
		Colors::Strip(string, string, strlen(string) + 1);
		return string;
	}

	__declspec(naked) void Colors::LookupColorStub()
	{
		__asm
		{
			pushad
			push [esp + 24h] // Index
			push esi        // Color ref
			call Colors::LookupColor
			add esp, 8h
			popad
			retn
		}
	}

	// Patches team overhead normally
	bool Colors::Dvar_GetUnpackedColorByName(const char* name, float* expandedColor)
	{
		if (Colors::ColorBlind.get<bool>())
		{
			const auto str = std::string(name);
			if (str == "g_TeamColor_EnemyTeam")
			{
				// Dvar_GetUnpackedColor
				auto* colorblindEnemy = Colors::ColorEnemyColorBlind->current.color;
				expandedColor[0] = static_cast<float>(colorblindEnemy[0]) / 255.0f;
				expandedColor[1] = static_cast<float>(colorblindEnemy[1]) / 255.0f;
				expandedColor[2] = static_cast<float>(colorblindEnemy[2]) / 255.0f;
				expandedColor[3] = static_cast<float>(colorblindEnemy[3]) / 255.0f;
				return false;
			}
			else if (str == "g_TeamColor_MyTeam")
			{
				// Dvar_GetUnpackedColor
				auto* colorblindAlly = Colors::ColorAllyColorBlind->current.color;
				expandedColor[0] = static_cast<float>(colorblindAlly[0]) / 255.0f;
				expandedColor[1] = static_cast<float>(colorblindAlly[1]) / 255.0f;
				expandedColor[2] = static_cast<float>(colorblindAlly[2]) / 255.0f;
				expandedColor[3] = static_cast<float>(colorblindAlly[3]) / 255.0f;
				return false;
			}
		}

		return true;
	}

	__declspec(naked) void Colors::GetUnpackedColorByNameStub()
	{
		__asm
		{
			push [esp + 8h]
			push [esp + 8h]
			call Colors::Dvar_GetUnpackedColorByName
			add esp, 8h

			test al, al
			jnz continue

			retn

		continue:
			push edi
			mov edi, [esp + 8h]
			push 406535h
			retn
		}
	}

	Colors::Colors()
	{
		// Add a colorblind mode for team colors
		Colors::ColorBlind = Dvar::Register<bool>("r_colorBlindTeams", false, Game::dvar_flag::DVAR_FLAG_SAVED, "Use color-blindness-friendly colors for ingame team names");
		// A dark red
		Colors::ColorEnemyColorBlind = Game::Dvar_RegisterColor("g_ColorBlind_EnemyTeam", 0.659f, 0.088f, 0.145f, 1, Game::dvar_flag::DVAR_FLAG_SAVED, "Enemy team color for colorblind mode");
		// A bright yellow
		Colors::ColorAllyColorBlind = Game::Dvar_RegisterColor("g_ColorBlind_MyTeam", 1, 0.859f, 0.125f, 1, Game::dvar_flag::DVAR_FLAG_SAVED, "Ally team color for colorblind mode");
		Utils::Hook(0x406530, Colors::GetUnpackedColorByNameStub, HOOK_JUMP).install()->quick();

		// Disable SV_UpdateUserinfo_f, to block changing the name ingame
		Utils::Hook::Set<BYTE>(0x6258D0, 0xC3);

		// Allow colored names ingame
		Utils::Hook(0x5D8B40, Colors::ClientUserinfoChanged, HOOK_JUMP).install()->quick();

		// Though, don't apply that to overhead names.
		Utils::Hook(0x581932, Colors::GetClientName, HOOK_CALL).install()->quick();

		// Patch RB_LookupColor
		Utils::Hook(0x534CD0, Colors::LookupColorStub, HOOK_JUMP).install()->quick();

		// Patch ColorIndex
		Utils::Hook(0x417770, Colors::ColorIndex, HOOK_JUMP).install()->quick();

		// Patch I_CleanStr
		Utils::Hook(0x4AD470, Colors::CleanStrStub, HOOK_JUMP).install()->quick();

		// Register dvar
		Colors::NewColors = Dvar::Register<bool>("cg_newColors", true, Game::dvar_flag::DVAR_FLAG_SAVED, "Use Warfare 2 color code style.");
		Game::Dvar_RegisterColor("sv_customTextColor", 1, 0.7f, 0, 1, Game::dvar_flag::DVAR_FLAG_REPLICATED, "Color for the extended color code.");
		Dvar::Register<bool>("sv_allowColoredNames", true, Game::dvar_flag::DVAR_FLAG_NONE, "Allow colored names on the server");

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
		Colors::Add(0, 0, 0);       // 10 - Rainbow (:)
		Colors::Add(0, 0, 0);       // 11 - Server color (;) - using that color in infostrings (e.g. your name) fails, ';' is an illegal character!
	}

	Colors::~Colors()
	{
		Colors::ColorTable.clear();
	}
}
