#include "Vote.hpp"

#include "ClientCommand.hpp"
#include "MapRotation.hpp"
#include "Events.hpp"
#include "Arena.hpp"

using namespace Utils::String;

namespace Components
{
  namespace
  {
    // The server id that the client keeps from the server's configstrings,
    // and which it checks the vote's time against.
    //
    const int* ServerId = reinterpret_cast<const int*>(0x140C9B95C);
  }

  Dvar::Var Vote::SV_VotesRequired;

  const Game::dvar_t* Vote::g_oldVoting;
  const Game::dvar_t* Vote::g_voteAbstainWeight;

  Vote::LevelVote Vote::Level;
  Vote::ClientVote Vote::Cgs;

  std::unordered_map<std::string, Vote::CommandHandler> Vote::VoteCommands =
  {
    {"map_restart", HandleMapRestart},
    {"map_rotate", HandleMapRotate},
    {"typemap", HandleTypemap},
    {"map", HandleMap},
    {"g_gametype", HandleGametype},
    {"kick", HandleKick},
    {"tempBanUser", HandleKick}
  };

  void Vote::DisplayVote(const Game::gentity_s* ent)
  {
    Game::SV_GameSendServerCommand(-1, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_CALLEDAVOTE\x15%s\"", 0x65, ent->client->sess.cs.name));
    Level.voteNo = 0;
    Level.voteYes = 1;
    Level.voteTime = Game::level->time + 30000;

    for (auto i = 0; i < Game::level->maxclients; ++i)
    {
      Game::level->clients[i].ps.eFlags &= ~Game::EF_VOTED;
    }

    ent->client->ps.eFlags |= Game::EF_VOTED;
    Game::SV_SetConfigstring(Game::CS_VOTE_TIME, VA("%i %i", Level.voteTime, *Game::sv_serverId_value));
    Game::SV_SetConfigstring(Game::CS_VOTE_STRING, Level.voteDisplayString);
    Game::SV_SetConfigstring(Game::CS_VOTE_YES, VA("%i", Level.voteYes));
    Game::SV_SetConfigstring(Game::CS_VOTE_NO, VA("%i", Level.voteNo));
  }

  int Vote::VotesRequired()
  {
    auto votesRequired = Vote::SV_VotesRequired.get<int>();

    if (votesRequired > 0)
    {
      return votesRequired;
    }
    return Game::level->numConnectedClients / 2 + 1;
  }

  // x64 compiles out SEH_ReadVoteReference(), presumably along with the
  // rest of the vote that the later revision no longer had any use for. So
  // this is x86's. As there, we look the text up by its load name in the
  // arenas, which the arena component keeps for the map list.
  //
  const char* Vote::SEH_ReadVoteReference(const char* text, unsigned int* consumed)
  {
    Utils::Hook::Call<void()>(0x140269D20)(); // UI_UpdateArenas

    for (const Game::newMapArena_t& entry : arena::maps())
    {
      *consumed = static_cast<unsigned int>(std::strlen(entry.mapName));
      if (Game::I_strnicmp(text, entry.mapName, static_cast<int>(*consumed)) == 0)
      {
        return Game::UI_SafeTranslateString(entry.uiName);
      }
    }

    return nullptr;
  }

  void Vote::GetVoteText(const char* voteString)
  {
    char source[256]{};
    std::size_t out = 0;

    for (std::size_t in = 0; in < sizeof(source); ++in)
    {
      unsigned int consumed = 0;
      if (const auto* expansion = SEH_ReadVoteReference(&voteString[in], &consumed))
      {
        for (const auto* c = expansion; *c != '\0'; ++c)
        {
          if (out < sizeof(source) - 1)
          {
            source[out++] = *c;
          }
        }

        in += consumed;
      }

      const auto ch = voteString[in];
      if (out < sizeof(source) - 1)
      {
        source[out++] = ch;
      }

      if (ch == '\0')
      {
        break;
      }
    }

    source[sizeof(source) - 1] = '\0';

    const auto* localized = SEH_LocalizeTextMessage(source, "vote string", Game::LOCMSG_SAFE);
    Game::I_strncpyz(Cgs.voteString, localized, sizeof(Cgs.voteString));
  }

  const char* Vote::SEH_LocalizeTextMessage(const char* text, const char* messageType, Game::msgLocErrType_t errType)
  {
    return Utils::Hook::Call<const char*(const char*, const char*, Game::msgLocErrType_t)>(0x14024D0B0)(text, messageType, errType);
  }

  bool Vote::IsInvalidVoteString(const std::string& input)
  {
    static const char* separators[] = { "\n", "\r", ";" };

    return std::ranges::any_of(separators,
      [&](const std::string& str) { return input.find(str) != std::string::npos; });
  }

  bool Vote::HandleMapRestart([[maybe_unused]] const Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
  {
    sprintf_s(Level.voteString, "fast_restart");
    sprintf_s(Level.voteDisplayString, "GAME_VOTE_MAPRESTART");
    return true;
  }

  bool Vote::HandleMapRotate([[maybe_unused]] const Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
  {
    sprintf_s(Level.voteString, "%s", params->get(1));
    sprintf_s(Level.voteDisplayString, "GAME_VOTE_NEXTMAP");
    return true;
  }

  bool Vote::HandleTypemap([[maybe_unused]] const Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
  {
    char arg2[0x100]{};
    char arg3[0x100]{};

    strncpy_s(arg2, params->get(2), _TRUNCATE);
    strncpy_s(arg3, params->get(3), _TRUNCATE);

    // This prevents abuse
    if (!MapRotation::Contains("map", arg3))
    {
      Game::SV_GameSendServerCommand(static_cast<int>(ent - Game::g_entities), Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_NOTONROTATION\"", 0x65));
      return false;
    }

    if (!Game::Scr_IsValidGameType(arg2))
    {
      Game::SV_GameSendServerCommand(static_cast<int>(ent - Game::g_entities), Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_INVALIDGAMETYPE\"", 0x65));
      return false;
    }

    if (!std::strcmp(arg2, (*Game::g_gametype)->current.string))
    {
      arg2[0] = '\0';
    }

    const auto* mapname = Game::Dvar_RegisterString("mapname", "", Game::DVAR_ROM | Game::DVAR_SERVERINFO, "Current map name");
    if (!std::strcmp(arg3, mapname->current.string))
    {
      arg3[0] = '\0';
    }

    if (!arg2[0] && !arg3[0])
    {
      Game::SV_GameSendServerCommand(static_cast<int>(ent - Game::g_entities), Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_TYPEMAP_NOCHANGE\"", 0x65));
      return false;
    }

    if (arg3[0])
    {
      if (arg2[0])
      {
        sprintf_s(Level.voteString, "g_gametype %s; map %s", arg2, arg3);
      }
      else
      {
        sprintf_s(Level.voteString, "map %s", arg3);
      }

      if (arg2[0])
      {
        sprintf_s(Level.voteDisplayString, "GAME_VOTE_GAMETYPE\x14%s\x15 - \x14GAME_VOTE_MAP\x15%s", Game::Scr_GetGameTypeNameForScript(arg2), arg3);
      }
      else
      {
        sprintf_s(Level.voteDisplayString, "GAME_VOTE_MAP\x15%s", arg3);
      }
    }
    else
    {
      sprintf_s(Level.voteString, "g_gametype %s; map_restart", arg2);
      sprintf_s(Level.voteDisplayString, "GAME_VOTE_GAMETYPE\x14%s", Game::Scr_GetGameTypeNameForScript(arg2));
    }

    return true;
  }

  bool Vote::HandleMap([[maybe_unused]] const Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
  {
    // This prevents abuse
    if (!MapRotation::Contains("map", params->get(2)))
    {
      Game::SV_GameSendServerCommand(static_cast<int>(ent - Game::g_entities), Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_NOTONROTATION\"", 0x65));
      return false;
    }

    sprintf_s(Level.voteString, "%s %s", params->get(1), params->get(2));
    sprintf_s(Level.voteDisplayString, "GAME_VOTE_MAP\x15%s", params->get(2));
    return true;
  }

  bool Vote::HandleGametype([[maybe_unused]] const Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
  {
    if (!Game::Scr_IsValidGameType(params->get(2)))
    {
      Game::SV_GameSendServerCommand(static_cast<int>(ent - Game::g_entities), Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_INVALIDGAMETYPE\"", 0x65));
      return false;
    }

    sprintf_s(Level.voteString, "%s %s; map_restart", params->get(2), params->get(3));
    sprintf_s(Level.voteDisplayString, "GAME_VOTE_GAMETYPE\x14%s", Game::Scr_GetGameTypeNameForScript(params->get(2)));
    return true;
  }

  bool Vote::HandleKick(const Game::gentity_s* ent, const Command::ServerParams* params)
  {
    char cleanName[0x40]{};

    auto kicknum = Game::level->maxclients;
    for (auto i = 0; i < Game::level->maxclients; ++i)
    {
      if (Game::level->clients[i].sess.connected == Game::CON_CONNECTED)
      {
        strncpy_s(cleanName, Game::level->clients[i].sess.cs.name, _TRUNCATE);
        Game::I_CleanStr(cleanName);
        if (Utils::String::Compare(cleanName, params->get(2)))
        {
          kicknum = i;
        }
      }
    }

    if (kicknum == Game::level->maxclients)
    {
      Game::SV_GameSendServerCommand(static_cast<int>(ent - Game::g_entities), Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_CLIENTNOTONSERVER\"", 0x65));
      return false;
    }

    sprintf_s(Level.voteString, "%s \"%d\"", "tempBanClient", kicknum); // kick and tempBanClient do the same thing
    sprintf_s(Level.voteDisplayString, "GAME_VOTE_KICK\x15(%i)%s", kicknum, Game::level->clients[kicknum].sess.cs.name);
    return true;
  }

  void Vote::Scr_VoteCalled(Game::gentity_s* self, const char* command, const char* param1, const char* param2)
  {
    Game::Scr_AddString(param2);
    Game::Scr_AddString(param1);
    Game::Scr_AddString(command);
    Game::Scr_Notify(self, Game::scr_const->call_vote, 3);
  }

  void Vote::Scr_PlayerVote(Game::gentity_s* self, const char* option)
  {
    Game::Scr_AddString(option);
    Game::Scr_Notify(self, Game::scr_const->vote, 1);
  }

  void Vote::Cmd_CallVote_f(Game::gentity_s* ent, const Command::ServerParams* params)
  {
    if (!(*Game::g_allowVote)->current.enabled)
    {
      Game::SV_GameSendServerCommand(static_cast<int>(ent - Game::g_entities), Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_VOTINGNOTENABLED\"", 0x65));
      return;
    }

    if (Game::level->numConnectedClients < VotesRequired())
    {
      Game::SV_GameSendServerCommand(static_cast<int>(ent - Game::g_entities), Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_VOTINGNOTENOUGHPLAYERS\"", 0x65));
      return;
    }

    if (g_oldVoting->current.enabled)
    {
      if (Level.voteTime)
      {
        Game::SV_GameSendServerCommand(static_cast<int>(ent - Game::g_entities), Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_VOTEALREADYINPROGRESS\"", 0x65));
        return;
      }

      if (ent->client->sess.voteCount >= MAX_VOTE_COUNT)
      {
        Game::SV_GameSendServerCommand(static_cast<int>(ent - Game::g_entities), Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_MAXVOTESCALLED\"", 0x65));
        return;
      }

      if (ent->client->sess.cs.team == Game::TEAM_SPECTATOR)
      {
        Game::SV_GameSendServerCommand(static_cast<int>(ent - Game::g_entities), Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_NOSPECTATORCALLVOTE\"", 0x65));
        return;
      }
    }

    if (IsInvalidVoteString(params->get(1)) ||
      IsInvalidVoteString(params->get(2)) ||
      IsInvalidVoteString(params->get(3)))
    {
      Game::SV_GameSendServerCommand(static_cast<int>(ent - Game::g_entities), Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_INVALIDVOTESTRING\"", 0x65));
      return;
    }

    if (!g_oldVoting->current.enabled)
    {
      Scr_VoteCalled(ent, params->get(1), params->get(2), params->get(3));
      return;
    }

    const auto itr = VoteCommands.find(params->get(1));
    if (itr == VoteCommands.end())
    {
      Game::SV_GameSendServerCommand(static_cast<int>(ent - Game::g_entities), Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_INVALIDVOTESTRING\"", 0x65));
      Game::SV_GameSendServerCommand(static_cast<int>(ent - Game::g_entities), Game::SV_CMD_CAN_IGNORE, VA(CallVoteDesc, 0x65));
      return;
    }

    if (Level.voteExecuteTime)
    {
      Level.voteExecuteTime = 0;
      Game::Cbuf_AddText(0, VA("%s\n", Level.voteString));
    }

    const auto shouldDisplay = itr->second(ent, params);
    if (shouldDisplay)
    {
      DisplayVote(ent);
      ++ent->client->sess.voteCount;
    }
  }

  void Vote::Cmd_Vote_f(Game::gentity_s* ent, const Command::ServerParams* params)
  {
    char arg1[0x100]{};

    if (g_oldVoting->current.enabled)
    {
      if (!Level.voteTime)
      {
        Game::SV_GameSendServerCommand(static_cast<int>(ent - Game::g_entities), Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_NOVOTEINPROGRESS\"", 0x65));
        return;
      }

      if ((ent->client->ps.eFlags & Game::EF_VOTED) != 0)
      {
        Game::SV_GameSendServerCommand(static_cast<int>(ent - Game::g_entities), Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_VOTEALREADYCAST\"", 0x65));
        return;
      }

      if (ent->client->sess.cs.team == Game::TEAM_SPECTATOR)
      {
        Game::SV_GameSendServerCommand(static_cast<int>(ent - Game::g_entities), Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_NOSPECTATORVOTE\"", 0x65));
        return;
      }

      Game::SV_GameSendServerCommand(static_cast<int>(ent - Game::g_entities), Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_VOTECAST\"", 0x65));
      ent->client->ps.eFlags |= Game::EF_VOTED;
    }

    strncpy_s(arg1, params->get(1), _TRUNCATE);
    if (arg1[0] == 'y' || arg1[0] == 'Y' || arg1[0] == '1')
    {
      if (g_oldVoting->current.enabled)
      {
        Game::SV_SetConfigstring(Game::CS_VOTE_YES, VA("%i", ++Level.voteYes));
      }
      else
      {
        Scr_PlayerVote(ent, "yes");
      }
    }
    else if (g_oldVoting->current.enabled)
    {
      Game::SV_SetConfigstring(Game::CS_VOTE_NO, VA("%i", ++Level.voteNo));
    }
    else
    {
      Scr_PlayerVote(ent, "no");
    }
  }

  // x64 drops CheckVote() from G_RunFrame(), so we bring back x86's and run
  // it where x86 does, right after the call that precedes it. Running it
  // anywhere else in the frame would probably work just as well, but this
  // way the timing is the same as on x86.
  //
  void Vote::G_RunFrame_Hk()
  {
    Utils::Hook::Call<void()>(0x1401AA180)();

    if (g_oldVoting->current.enabled)
    {
      CheckVote();
    }
  }

  void Vote::CheckVote()
  {
    if (Level.voteExecuteTime && Level.voteExecuteTime < Game::level->time)
    {
      Level.voteExecuteTime = 0;
      Game::Cbuf_AddText(0, VA("%s\n", Level.voteString));
    }

    if (!Level.voteTime)
    {
      return;
    }

    auto passed (false);

    if (Game::level->time - Level.voteTime >= 0)
    {
      // Note that x86 rounds the weighted abstains with fistp, which rounds to
      // nearest even, after adding just under one half. Probably someone's
      // idea of rounding up that the compiler took literally. Odd, but we
      // want the same count as an x86 server, so we do the same.
      //
      const auto abstain (static_cast<float>(Level.numVotingClients - Level.voteNo - Level.voteYes) * g_voteAbstainWeight->current.value);
      const auto weighted (static_cast<int>(std::nearbyint(static_cast<double>(abstain) + 0.4999999990686774)));

      passed = Level.voteYes > weighted + Level.voteNo;
    }
    else
    {
      const auto half (Level.numVotingClients / 2 + 1);

      if (Level.voteYes >= half)
      {
        passed = true;
      }
      else if (Level.voteNo <= Level.numVotingClients - half)
      {
        return;
      }
    }

    if (passed)
    {
      Game::SV_GameSendServerCommand(-1, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_VOTEPASSED\"", 0x65));
      Level.voteExecuteTime = Game::level->time + 3000;
    }
    else
    {
      Game::SV_GameSendServerCommand(-1, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_VOTEFAILED\"", 0x65));
    }

    Level.voteTime = 0;
    Game::SV_SetConfigstring(Game::CS_VOTE_TIME, "");
  }

  // x86 counts the clients that may vote in CalculateRanks()'s loop over
  // the clients, which x64 keeps but without the count. So we count them
  // ourselves once it has sorted the clients. We could have hooked the loop
  // itself, but the sort is a single call, which seemed a lot simpler.
  //
  void Vote::CalculateRanks_qsort_Hk(void* base, std::size_t count, std::size_t size, int (*compare)(const void*, const void*))
  {
    Utils::Hook::Call<void(void*, std::size_t, std::size_t, int (*)(const void*, const void*))>(0x140338FC0)(base, count, size, compare); // qsort

    Level.numVotingClients = 0;

    for (auto i = 0; i < Game::level->maxclients; ++i)
    {
      const auto& sess (Game::level->clients[i].sess);

      if (sess.connected == Game::CON_CONNECTED &&
          sess.cs.team != Game::TEAM_SPECTATOR)
      {
        ++Level.numVotingClients;
      }
    }
  }

  // G_InitGame() clears level with this memset, and on x86 that clears the
  // vote fields along with it, which we keep separately.
  //
  void* Vote::G_InitGame_memset_Hk(void* dest, int value, std::size_t size)
  {
    std::memset(&Level, 0, sizeof(Level));
    return std::memset(dest, value, size);
  }

  // x64 compiles out the client's side of the vote along with its fields in
  // cgs. So we bring back CG_ParseVote() and CG_DrawVote() from x86, along
  // with the vote's parts of CG_ConfigStringModified() and CG_MapRestart().
  // It's a fair bit of code to carry, but votes are one of those things
  // people would definitely miss.
  //
  void Vote::CG_ParseVoteTime(const char* string)
  {
    Cgs.voteTime = 0;

    int voteTime, serverId;
    if (std::sscanf(string, "%d %d", &voteTime, &serverId) == 2 && serverId == *ServerId)
    {
      Cgs.voteTime = voteTime;
    }
  }

  void Vote::CG_ParseVote()
  {
    CG_ParseVoteTime(Game::CL_GetConfigString(Game::CS_VOTE_TIME));
    Cgs.voteYes = std::atoi(Game::CL_GetConfigString(Game::CS_VOTE_YES));
    Cgs.voteNo = std::atoi(Game::CL_GetConfigString(Game::CS_VOTE_NO));

    const auto* localized = SEH_LocalizeTextMessage(Game::CL_GetConfigString(Game::CS_VOTE_STRING), "vote string", Game::LOCMSG_SAFE);
    Game::I_strncpyz(Cgs.voteString, localized, sizeof(Cgs.voteString));
  }

  void Vote::CG_Init_Hk(const int localClientNum)
  {
    CG_ParseVote();

    Utils::Hook::Call<void(int)>(0x1400CCB50)(localClientNum);
  }

  void Vote::CG_ConfigStringModified_Hk(const int localClientNum)
  {
    Utils::Hook::Call<void(int)>(0x1400E5E10)(localClientNum); // CG_ConfigStringModified

    const Command::ClientParams params;
    const auto num (std::atoi(params.get(1)));
    if (num < 0 || num >= Game::MAX_CONFIGSTRINGS)
    {
      return;
    }

    const auto* string (Game::CL_GetConfigString(num));

    switch (num)
    {
      case Game::CS_VOTE_TIME:
        CG_ParseVoteTime(string);
        break;
      case Game::CS_VOTE_YES:
        Cgs.voteYes = std::atoi(string);
        break;
      case Game::CS_VOTE_NO:
        Cgs.voteNo = std::atoi(string);
        break;
      case Game::CS_VOTE_STRING:
        GetVoteText(string);
        break;
    }
  }

  void Vote::CG_MapRestart_Hk(const int localClientNum, const int restart)
  {
    Utils::Hook::Call<void(int, int)>(0x1400E7730)(localClientNum, restart); // CG_MapRestart

    Cgs.voteTime = 0;
  }

  void Vote::CG_DrawVote(const int localClientNum)
  {
    if (!Cgs.voteTime)
    {
      return;
    }

    auto* placement (Game::ScrPlace_GetActivePlacement(localClientNum));

    const auto size (Game::cgsArray[localClientNum].viewHeight > 768 ? 10.0f : 16.0f);
    const auto scale (static_cast<float>(size / 48.0));
    auto* font (Game::UI_GetFontHandle(placement, 0, scale));

    const auto x ((*Game::cg_hudVotePosition)->current.vector[0]);
    const auto y ((*Game::cg_hudVotePosition)->current.vector[1] + size);

    auto key ([localClientNum] (const char* command, char (&key)[256])
    {
      char binding[256];

      // UI_GetKeyBindingLocalizedString()
      //
      if (Utils::Hook::Call<int(int, const char*, char*, int, bool)>(0x14025CF80)(localClientNum, command, binding, sizeof(binding), false))
      {
        Game::I_strncpyz(key, binding, sizeof(key));
      }
      else
      {
        Game::I_strncpyz(key, command, sizeof(key));
      }
    });

    char yesKey[256];
    char noKey[256];
    key("vote yes", yesKey);
    key("vote no", noKey);

    auto seconds ((Cgs.voteTime - Game::cgArray[localClientNum].time) / 1000);
    if (seconds < 0)
    {
      seconds = 0;
    }

    constexpr float color[4] = {1.0f, 1.0f, 0.0f, 1.0f};

    const auto* line (VA("%s(%i):%s", Game::UI_SafeTranslateString("CGAME_VOTE"), seconds, Cgs.voteString));
    Game::UI_DrawText(placement, line, std::numeric_limits<int>::max(), font, x, y, 1, 1, scale, color, 3);

    if (Game::cgArray[localClientNum].predictedPlayerState.eFlags & Game::EF_VOTED)
    {
      line = VA("%s:%i, %s:%i", Game::UI_SafeTranslateString("CGAME_YES"), Cgs.voteYes, Game::UI_SafeTranslateString("CGAME_NO"), Cgs.voteNo);
    }
    else
    {
      line = VA("%s(%s):%i, %s(%s):%i", Game::UI_SafeTranslateString("CGAME_YES"), yesKey, Cgs.voteYes, Game::UI_SafeTranslateString("CGAME_NO"), noKey, Cgs.voteNo);
    }

    Game::UI_DrawText(placement, line, std::numeric_limits<int>::max(), font, x, y + size, 1, 1, scale, color, 3);
  }

  Vote::Vote()
  {
    AssertSize(Game::gclient_s, 0x3678);
    AssertOffset(Game::gclient_s, sess.connected, 0x3148);
    AssertOffset(Game::gclient_s, sess.cs.team, 0x31DC);
    AssertOffset(Game::gclient_s, sess.cs.name, 0x321C);
    AssertOffset(Game::cg_s, predictedPlayerState.eFlags, 0xB0);
    AssertOffset(Game::cgs_t, viewHeight, 0xC);
    AssertOffset(Game::scr_const_t, call_vote, 0x1D6);
    AssertOffset(Game::scr_const_t, vote, 0x1EC);
    AssertOffset(Game::uiInfo_s, playerIndex, 0x2CD0);
    AssertOffset(Game::sharedUiInfo_t, playerCount, 0xB0);
    AssertOffset(Game::sharedUiInfo_t, playerNames, 0xB4);
    AssertOffset(Game::sharedUiInfo_t, gameTypes, 0x584);
    AssertOffset(Game::sharedUiInfo_t, mapCount, 0x1E10);

    // Replicate g_allowVote
    Utils::Hook::Set<std::uint32_t>(0x14019DF57, Game::DVAR_INTERNAL | Game::DVAR_CODINFO);

    Events::OnDvarInit([]{
      Vote::SV_VotesRequired = Game::Dvar_RegisterInt("sv_votesRequired", 0, 0, 18, Game::DVAR_NONE, "Set the amount of votes required for a vote to pass.\n0 = (players / 2) + 1");

      // x64 compiles out these two along with the vote, so we register them
      // the way x86 does.
      //
      g_oldVoting = Game::Dvar_RegisterBool("g_oldVoting", true, Game::DVAR_ARCHIVE | Game::DVAR_INTERNAL, "Use old voting method");
      g_voteAbstainWeight = Game::Dvar_RegisterFloat("g_voteAbstainWeight", 0.5f, 0.0f, 1.0f, Game::DVAR_ARCHIVE | Game::DVAR_INTERNAL, "How much an abstained vote counts as a 'no' vote");
    });

    Utils::Hook(0x14019CFC8, G_InitGame_memset_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x14019CACF, CalculateRanks_qsort_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x14019EFB9, G_RunFrame_Hk, HOOK_CALL).install()->quick();

    Utils::Hook(0x1400D6D82, CG_Init_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400E65B7, CG_ConfigStringModified_Hk, HOOK_CALL).install()->quick(); // CG_ServerCommand
    Utils::Hook(0x1400E6779, CG_MapRestart_Hk, HOOK_CALL).install()->quick(); // CG_ServerCommand
    Utils::Hook(0x1400E6F0A, CG_MapRestart_Hk, HOOK_CALL).install()->quick(); // CG_ServerCommand

    ClientCommand::Add("callvote", Cmd_CallVote_f);
    ClientCommand::Add("vote", Cmd_Vote_f);

    UIScript::Add("voteKick", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
    {
      if (info->playerIndex >= 0 && info->playerIndex < Game::sharedUiInfo->playerCount)
      {
        Game::Cbuf_AddText(0, VA("callvote kick \"%s\"\n", Game::sharedUiInfo->playerNames[info->playerIndex]));
      }
    });

    UIScript::Add("voteTempBan", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
    {
      if (info->playerIndex >= 0 && info->playerIndex < Game::sharedUiInfo->playerCount)
      {
        Game::Cbuf_AddText(0, VA("callvote tempBanUser \"%s\"\n", Game::sharedUiInfo->playerNames[info->playerIndex]));
      }
    });

    // Note that ui_currentMap is a position in the map list as the menu shows
    // it, which the game sorts by display name. So we have to go through the
    // sorted list rather than index the arena file's order with it.
    //
    UIScript::Add("voteTypeMap", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
    {
      if (const Game::newMapArena_t* map = arena::sorted_map((*Game::ui_currentMap)->current.integer))
      {
        Game::Cbuf_AddText(0, VA("callvote typemap %s %s\n", Game::sharedUiInfo->gameTypes[(*Game::ui_netGameType)->current.integer].gameType,
          map->mapName));
      }
    });

    UIScript::Add("voteMap", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
    {
      if (const Game::newMapArena_t* map = arena::sorted_map((*Game::ui_currentMap)->current.integer))
      {
        Game::Cbuf_AddText(0, VA("callvote map %s\n", map->mapName));
      }
    });

    UIScript::Add("voteGame", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
    {
      Game::Cbuf_AddText(0, VA("callvote g_gametype %s\n", Game::sharedUiInfo->gameTypes[(*Game::ui_netGameType)->current.integer].gameType));
    });
  }
}
