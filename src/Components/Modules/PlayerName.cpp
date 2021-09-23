#include "STDInclude.hpp"

namespace Components
{
    Dvar::Var PlayerName::sv_allowColoredNames;

    void PlayerName::UserInfoCopy(char* buffer, const char* name, const size_t size)
    {
        if (!sv_allowColoredNames.get<bool>())
        {
            char nameBuffer[64] = { 0 };
            TextRenderer::StripColors(name, nameBuffer, sizeof(nameBuffer));
            TextRenderer::StripAllTextIcons(nameBuffer, buffer, size);
        }
        else
        {
            TextRenderer::StripAllTextIcons(name, buffer, size);
        }

        std::string readablePlayerName(buffer);
        readablePlayerName = Utils::String::Trim(readablePlayerName);

        if (readablePlayerName.size() < 3)
        {
            strncpy(buffer, "Unknown Soldier", size);
        }
    }

    __declspec(naked) void PlayerName::ClientUserinfoChanged()
    {
        __asm
        {
            mov eax, [esp + 4h] // length
            //sub eax, 1
            push eax

            push ecx // name
            push edx // buffer

            call UserInfoCopy

            add esp, 0Ch
            retn
        }
    }

    char* PlayerName::GetClientName(int localClientNum, int index, char* buf, size_t size)
    {
        Game::CL_GetClientName(localClientNum, index, buf, size);

        // Append clantag to username & remove the colors
        strncpy_s(buf, size, TextRenderer::StripColors(ClanTags::GetUserClantag(index, buf)).data(), size);

        return buf;
    }
	char* PlayerName::CleanStrStub(char* string)
	{
		TextRenderer::StripColors(string, string, strlen(string) + 1);
		return string;
	}

	PlayerName::PlayerName()
	{
		sv_allowColoredNames = Dvar::Register<bool>("sv_allowColoredNames", true, Game::dvar_flag::DVAR_FLAG_NONE, "Allow colored names on the server");

		// Disable SV_UpdateUserinfo_f, to block changing the name ingame
		Utils::Hook::Set<BYTE>(0x6258D0, 0xC3);

		// Allow colored names ingame
		Utils::Hook(0x5D8B40, ClientUserinfoChanged, HOOK_JUMP).install()->quick();

		// Though, don't apply that to overhead names.
		Utils::Hook(0x581932, GetClientName, HOOK_CALL).install()->quick();

		// Patch I_CleanStr
		Utils::Hook(0x4AD470, CleanStrStub, HOOK_JUMP).install()->quick();
	}
}
