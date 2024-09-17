#pragma once

namespace Components
{
	class Chat : public Component
	{
		static constexpr auto FONT_ICON_CHAT_WIDTH_CALCULATION_MULTIPLIER = 2.0f;
	public:
		Chat();

		static bool IsMuted(const Game::gentity_s* ent);
		static bool IsMuted(const Game::client_s* cl);

	private:
		static Dvar::Var cg_chatWidth;
		static Dvar::Var sv_disableChat;
		static Dvar::Var sv_sayName;

		static bool SendChat;

		using muteList = std::unordered_set<std::uint64_t>;
		static Utils::Concurrency::Container<muteList> MutedList;
		static const char* MutedListFile;

		static bool CanAddCallback; // ClientCommand & GSC thread are the same
		static std::vector<Scripting::Function> SayCallbacks;

		static std::unique_lock<Utils::NamedMutex> Lock();

		static const char* EvaluateSay(char* text, Game::gentity_s* player, int mode);

		static void PreSayStub();
		static void PostSayStub();

		static void CheckChatLineEnd(const char*& inputBuffer, char*& lineBuffer, float& len, int chatHeight, float chatWidth, char*& lastSpacePos, char*& lastFontIconPos, int lastColor);
		static void CG_AddToTeamChat(const char* text);
		static void CG_AddToTeamChat_Stub();

		static void MuteClient(const Game::client_s* client);
		static void UnmuteClient(const Game::client_s* client);
		static void UnmuteInternal(std::uint64_t id, bool everyone = false);
		static void SaveMutedList(const muteList& list);
		static void LoadMutedList();

		static void AddServerCommands();

		static int GetCallbackReturn();
		static int ChatCallback(Game::gentity_s* self, const char* codePos, const char* message, int mode);
		static void AddScriptFunctions();
	};
}
