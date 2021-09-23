#pragma once

namespace Components
{
	class Chat : public Component
	{
		static constexpr auto FONT_ICON_CHAT_WIDTH_CALCULATION_MULTIPLIER = 2.0f;
	public:
		Chat();

	private:
		static Game::dvar_t** cg_chatHeight;
		static Dvar::Var cg_chatWidth;
		static Game::dvar_t** cg_chatTime;

		static bool SendChat;

		static const char* EvaluateSay(char* text, Game::gentity_t* player);

		static void PreSayStub();
		static void PostSayStub();

		static void CheckChatLineEnd(const char*& inputBuffer, char*& lineBuffer, float& len, int chatHeight, float chatWidth, char*& lastSpacePos, char*& lastFontIconPos, int lastColor);
		static void CG_AddToTeamChat(const char* text);
		static void CG_AddToTeamChat_Stub();
	};
}
