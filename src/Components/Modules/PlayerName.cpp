#include <STDInclude.hpp>
#include "ClanTags.hpp"
#include "PlayerName.hpp"

namespace Components
{
	Dvar::Var PlayerName::sv_allowColoredNames;

	void PlayerName::UserInfoCopy(char* buffer, const char* name, const size_t size)
	{
		if (!sv_allowColoredNames.get<bool>())
		{
			char nameBuffer[64] = {0};
			TextRenderer::StripColors(name, nameBuffer, sizeof(nameBuffer));
			TextRenderer::StripAllTextIcons(nameBuffer, buffer, size);
		}
		else
		{
			TextRenderer::StripAllTextIcons(name, buffer, size);
		}

		std::string readablePlayerName(buffer);
		Utils::String::Trim(readablePlayerName);

		if (readablePlayerName.size() < 3)
		{
			strncpy(buffer, "Unknown Soldier", size);
		}
	}

	__declspec(naked) void PlayerName::ClientCleanName()
	{
		__asm
		{
			mov eax, [esp + 4h] // length

			push eax

			push ecx // name
			push edx // buffer

			call UserInfoCopy

			add esp, 0Ch
			retn
		}
	}

	int PlayerName::GetClientName(int localClientNum, int index, char* buf, int size)
	{
		const auto result = Game::CL_GetClientName(localClientNum, index, buf, size);

		// Prepend clanName to username & remove the colors
		strncpy_s(buf, size, TextRenderer::StripColors(ClanTags::GetClanTagWithName(index, buf)).data(), size);

		return result;
	}

	char* PlayerName::CleanStrStub(char* string)
	{
		TextRenderer::StripColors(string, string, strlen(string) + 1);
		return string;
	}

	bool PlayerName::CopyClientNameCheck(char* dest, const char* source, int size)
	{
		Utils::Hook::Call<void(char*, const char*, int)>(0x4D6F80)(dest, source, size); // I_strncpyz

		auto i = 0;
		while (i < size - 1 && dest[i] != '\0')
		{
			if (dest[i] > 125 || dest[i] < 32 || dest[i] == '%')
			{
				return false; // Illegal string
			}

			++i;
		}

		return true;
	}

	__declspec(naked) void PlayerName::SV_UserinfoChangedStub()
	{
		__asm
		{
			call CopyClientNameCheck
			test al, al

			jnz returnSafe

			pushad

			push 1 // tellThem
			push INVALID_NAME_MSG // reason
			push edi // drop
			mov eax, 0x4D1600 // SV_DropClient
			call eax
			add esp, 0xC

			popad

		returnSafe:
			push 0x401988
			retn
		}
	}

	PlayerName::PlayerName()
	{
		sv_allowColoredNames = Dvar::Register<bool>("sv_allowColoredNames", true, Game::DVAR_NONE, "Allow colored names on the server");

		// Disable SV_UpdateUserinfo_f, to block changing the name ingame
		Utils::Hook::Set<BYTE>(0x6258D0, 0xC3);

		// Allow colored names ingame. Hook placed in ClientUserinfoChanged
		Utils::Hook(0x5D8B40, ClientCleanName, HOOK_JUMP).install()->quick();

		// Though, don't apply that to overhead names.
		Utils::Hook(0x581932, GetClientName, HOOK_CALL).install()->quick();

		// Patch I_CleanStr
		Utils::Hook(0x4AD470, CleanStrStub, HOOK_JUMP).install()->quick();

		// Detect invalid characters including '%' to prevent format string vulnerabilities.
		// Kicks the player as soon as possible
		Utils::Hook(0x401983, SV_UserinfoChangedStub, HOOK_JUMP).install()->quick();
	}
}
