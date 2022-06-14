#pragma once

namespace Components
{
	class Chat : public Component
	{
		static constexpr auto FONT_ICON_CHAT_WIDTH_CALCULATION_MULTIPLIER = 2.0f;
	public:
		Chat();

	private:
		static Dvar::Var cg_chatWidth;
		static Dvar::Var sv_disableChat;

		// Game dvars
		static Game::dvar_t** cg_chatHeight;
		static Game::dvar_t** cg_chatTime;

		static bool SendChat;

		static std::mutex AccessMutex;
		static std::unordered_set<std::uint64_t> MuteList;

		static bool CanAddCallback; // ClientCommand & GSC thread are the same
		static std::vector<Scripting::Function> SayCallbacks;

		static const char* EvaluateSay(char* text, Game::gentity_t* player, int mode);

		static void PreSayStub();
		static void PostSayStub();

		static void CheckChatLineEnd(const char*& inputBuffer, char*& lineBuffer, float& len, int chatHeight, float chatWidth, char*& lastSpacePos, char*& lastFontIconPos, int lastColor);
		static void CG_AddToTeamChat(const char* text);
		static void CG_AddToTeamChat_Stub();

		static void MuteClient(const Game::client_t* client);
		static void UnmuteClient(const Game::client_t* client);
		static void UnmuteInternal(const std::uint64_t id, bool everyone = false);
		static void AddChatCommands();

		static int GetCallbackReturn();
		static int ChatCallback(Game::gentity_s* self, const char* codePos, const char* message, int mode);
		static void AddScriptFunctions();
	};
}
