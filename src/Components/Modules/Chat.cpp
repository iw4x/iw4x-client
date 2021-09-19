#include "STDInclude.hpp"

namespace Components
{
	bool Chat::SendChat;

	const char* Chat::EvaluateSay(char* text, Game::gentity_t* player)
	{
		SendChat = true;

		if (text[1] == '/')
		{
			SendChat = false;
			text[1] = text[0];
			++text;
		}

		TextRenderer::StripMaterialTextIcons(text, text, strlen(text) + 1);

		Game::Scr_AddEntity(player);
		Game::Scr_AddString(text + 1);
		Game::Scr_NotifyLevel(Game::SL_GetString("say", 0), 2);

		return text;
	}

	__declspec(naked) void Chat::PreSayStub()
	{
		__asm
		{
			mov eax, [esp + 100h + 10h]

			push eax
			pushad

			push[esp + 100h + 28h]
			push eax
			call Chat::EvaluateSay
			add esp, 8h

			mov[esp + 20h], eax
			popad
			pop eax

			mov[esp + 100h + 10h], eax

			jmp PlayerName::CleanStrStub
		}
	}

	__declspec(naked) void Chat::PostSayStub()
	{
		__asm
		{
			// eax is used by the callee
			push eax

			xor eax, eax
			mov al, Chat::SendChat

			test al, al
			jnz return

			// Don't send the chat
			pop eax
			retn

			return:
			pop eax

			// Jump to the target
			push 5DF620h
			retn
		}
	}

	Chat::Chat()
	{
		// Intercept chat sending
		Utils::Hook(0x4D000B, PreSayStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x4D00D4, PostSayStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x4D0110, PostSayStub, HOOK_CALL).install()->quick();
	}
}
