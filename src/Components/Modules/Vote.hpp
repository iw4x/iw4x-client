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

    static Dvar::Var SV_VotesRequired;

    static const Game::dvar_t* g_oldVoting;
    static const Game::dvar_t* g_voteAbstainWeight;

    // The vote fields of x86's level_locals_t.
    //
    struct LevelVote
    {
      char voteString[1024];
      char voteDisplayString[1024];
      int voteTime;
      int voteExecuteTime;
      int voteYes;
      int voteNo;
      int numVotingClients;
    };

    static LevelVote Level;

    // The vote fields of x86's cgs_t.
    //
    struct ClientVote
    {
      int voteTime;
      int voteYes;
      int voteNo;
      char voteString[256];
    };

    static ClientVote Cgs;

    // The number of votes a single player may call. Same as the stock
    // MAX_VOTE_COUNT.
    //
    static constexpr auto MAX_VOTE_COUNT = 3;

    static constexpr auto* CallVoteDesc = "%c \"GAME_VOTECOMMANDSARE\x15 map_restart, map_rotate, map <mapname>, g_gametype <typename>, typemap <typename> <mapname>, "
      "kick <player>, tempBanUser <player>\"";

    static void DisplayVote(const Game::gentity_s* ent);
    static bool IsInvalidVoteString(const std::string& input);
    static int VotesRequired();

    static const char* SEH_ReadVoteReference(const char* text, unsigned int* consumed);
    static const char* SEH_LocalizeTextMessage(const char* text, const char* messageType, Game::msgLocErrType_t errType);
    static void GetVoteText(const char* voteString);

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

    static void CheckVote();
    static void G_RunFrame_Hk();
    static void CalculateRanks_qsort_Hk(void* base, std::size_t count, std::size_t size, int (*compare)(const void*, const void*));
    static void* G_InitGame_memset_Hk(void* dest, int value, std::size_t size);

    static void CG_ParseVoteTime(const char* string);
    static void CG_ParseVote();
    static void CG_Init_Hk(int localClientNum);
    static void CG_ConfigStringModified_Hk(int localClientNum);
    static void CG_MapRestart_Hk(int localClientNum, int restart);
    static void CG_DrawVote(int localClientNum);
    static void CG_Draw2D_Hk(int localClientNum, int x, int y);
  };
}
