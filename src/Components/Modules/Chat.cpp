#include "STDInclude.hpp"

namespace Components
{
	Game::dvar_t** Chat::cg_chatHeight = reinterpret_cast<Game::dvar_t**>(0x7ED398);
	Dvar::Var Chat::cg_chatWidth;
	Game::dvar_t** Chat::cg_chatTime = reinterpret_cast<Game::dvar_t**>(0x9F5DE8);

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

	void Chat::CheckChatLineEnd(const char*& inputBuffer, char*& lineBuffer, float& len, const int chatHeight, const float chatWidth, char*& lastSpacePos, char*& lastFontIconPos, const int lastColor)
    {
		if (len > chatWidth)
		{
			if (lastSpacePos && lastSpacePos > lastFontIconPos)
			{
				inputBuffer += lastSpacePos - lineBuffer + 1;
				lineBuffer = lastSpacePos;
			}
			else if (lastFontIconPos)
			{
				inputBuffer += lastFontIconPos - lineBuffer;
				lineBuffer = lastFontIconPos;
			}
			*lineBuffer = 0;
			len = 0.0f;
			Game::cgsArray[0].teamChatMsgTimes[Game::cgsArray[0].teamChatPos % chatHeight] = Game::cgArray[0].time;

			Game::cgsArray[0].teamChatPos++;
			lineBuffer = Game::cgsArray[0].teamChatMsgs[Game::cgsArray[0].teamChatPos % chatHeight];
			lineBuffer[0] = '^';
			lineBuffer[1] = CharForColorIndex(lastColor);
			lineBuffer += 2;
			lastSpacePos = nullptr;
			lastFontIconPos = nullptr;
		}
    }

	void Chat::CG_AddToTeamChat(const char* text)
	{
		// Text can only be 150 characters maximum. This is bigger than the teamChatMsgs buffers with 160 characters
		// Therefore it is not needed to check for buffer lengths

		const auto chatHeight = (*cg_chatHeight)->current.integer;
		const auto chatWidth = static_cast<float>(cg_chatWidth.get<int>());
		const auto chatTime = (*cg_chatTime)->current.integer;
		if (chatHeight < 0 || static_cast<unsigned>(chatHeight) > std::extent_v<decltype(Game::cgs_t::teamChatMsgs)> || chatWidth <= 0 || chatTime <= 0)
		{
			Game::cgsArray[0].teamLastChatPos = 0;
			Game::cgsArray[0].teamChatPos = 0;
			return;
		}

		TextRenderer::FontIconInfo fontIconInfo{};
		auto len = 0.0f;
		auto lastColor = static_cast<int>(TEXT_COLOR_DEFAULT);
		char* lastSpace = nullptr;
		char* lastFontIcon = nullptr;
		char* p = Game::cgsArray[0].teamChatMsgs[Game::cgsArray[0].teamChatPos % chatHeight];
		p[0] = '\0';

		while (*text)
		{
			CheckChatLineEnd(text, p, len, chatHeight, chatWidth, lastSpace, lastFontIcon, lastColor);

			const char* fontIconEndPos = &text[1];
			if(text[0] == TextRenderer::FONT_ICON_SEPARATOR_CHARACTER && TextRenderer::IsFontIcon(fontIconEndPos, fontIconInfo))
			{
				// The game calculates width on a per character base. Since the width of a font icon is calculated based on the height of the font
				// which is roughly double as much as the average width of a character without an additional multiplier the calculated len of the font icon
				// would be less than it most likely would be rendered. Therefore apply a guessed 2.0f multiplier at this location which makes
				// the calculated width of a font icon roughly comparable to the width of an average character of the font.
				const auto normalizedFontIconWidth = TextRenderer::GetNormalizedFontIconWidth(fontIconInfo);
				const auto fontIconWidth = normalizedFontIconWidth * FONT_ICON_CHAT_WIDTH_CALCULATION_MULTIPLIER;
				len += fontIconWidth;

				lastFontIcon = p;
				for(; text < fontIconEndPos; text++)
				{
					p[0] = text[0];
					p++;
				}

				CheckChatLineEnd(text, p, len, chatHeight, chatWidth, lastSpace, lastFontIcon, lastColor);
			}
			else if (text[0] == '^' && text[1] != 0 && text[1] >= TextRenderer::COLOR_FIRST_CHAR && text[1] <= TextRenderer::COLOR_LAST_CHAR)
			{
				p[0] = '^';
				p[1] = text[1];
				lastColor = ColorIndexForChar(text[1]);
				p += 2;
				text += 2;
			}
			else
			{
				if (text[0] == ' ')
					lastSpace = p;
				*p++ = *text++;
				len += 1.0f;
			}
		}

		*p = 0;

		Game::cgsArray[0].teamChatMsgTimes[Game::cgsArray[0].teamChatPos % chatHeight] = Game::cgArray[0].time;

		Game::cgsArray[0].teamChatPos++;
		if (Game::cgsArray[0].teamChatPos - Game::cgsArray[0].teamLastChatPos > chatHeight)
			Game::cgsArray[0].teamLastChatPos = Game::cgsArray[0].teamChatPos + 1 - chatHeight;
	}

	__declspec(naked) void Chat::CG_AddToTeamChat_Stub()
	{
		__asm
		{
			pushad

			push ecx
			call CG_AddToTeamChat
			add esp, 4h

			popad
			ret
		}
	}

	Chat::Chat()
	{
		cg_chatWidth = Dvar::Register<int>("cg_chatWidth", 52, 1, INT_MAX, Game::DVAR_FLAG_SAVED, "The normalized maximum width of a chat message");

		// Intercept chat sending
		Utils::Hook(0x4D000B, PreSayStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x4D00D4, PostSayStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x4D0110, PostSayStub, HOOK_CALL).install()->quick();

		// Change logic that does word splitting with new lines for chat messages to support fonticons
		Utils::Hook(0x592E10, CG_AddToTeamChat_Stub, HOOK_JUMP).install()->quick();
	}
}
