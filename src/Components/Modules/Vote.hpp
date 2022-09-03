#pragma once

namespace Components
{
	class Vote : public Component
	{
	public:
		Vote();

	private:
		using CommandHandler = std::function<bool(const Game::gentity_s* ent, const Command::ServerParams* params)>;
		static std::unordered_map<std::string, CommandHandler> VoteCommands;

		static constexpr auto* CallVoteDesc = "%c \"GAME_VOTECOMMANDSARE\x15 map_restart, map_rotate, map <mapname>, g_gametype <typename>, typemap <typename> <mapname>, "
			"kick <player>, tempBanUser <player>\"";

		static void DisplayVote(const Game::gentity_s* ent);
		static bool IsInvalidVoteString(const std::string& input);

		static bool HandleMapRestart(const Game::gentity_s* ent, const Command::ServerParams* params);
		static bool HandleMapRotate(const Game::gentity_s* ent, const Command::ServerParams* params);
		static bool HandleTypemap(const Game::gentity_s* ent, const Command::ServerParams* params);
		static bool HandleMap(const Game::gentity_s* ent, const Command::ServerParams* params);
		static bool HandleGametype(const Game::gentity_s* ent, const Command::ServerParams* params);
		static bool HandleKick(const Game::gentity_s* ent, const Command::ServerParams* params);

		static void Scr_VoteCalled(Game::gentity_s* self, const char* command, const char* param1, const char* param2);
		static void Scr_PlayerVote(Game::gentity_s* self, const char* option);

		static void Cmd_CallVote_f(Game::gentity_s* ent, const Command::ServerParams* params);
		static void Cmd_Vote_f(Game::gentity_s* ent, const Command::ServerParams* params);
	};
}
