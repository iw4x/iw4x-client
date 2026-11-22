#include "Compatibility.hpp"
#include "ServerCommands.hpp"

namespace Components
{
  void (*Compatibility::SV_Netchan_Decode_Original)(char* client, unsigned char* data, int length);
  void (*Compatibility::CL_Netchan_Transmit_Original)(void* chan, unsigned char* data, int length);
  void (*Compatibility::MSG_ReadDeltaHudElems_Original)(Game::msg_t* msg, int time, const Game::hudelem_s* from, Game::hudelem_s* to, int count);
  void (*Compatibility::MSG_WriteDeltaHudElems_Original)(void* snapInfo, Game::msg_t* msg, int time, const Game::hudelem_s* from, const Game::hudelem_s* to, int count);
  void (*Compatibility::MSG_SetDefaultUserCmd_Original)(Game::playerState_s* ps, Game::usercmd_s* cmd);

  namespace
  {
    // The server message ops as x86 numbers them, from svc_nop, by the
    // game's op, which x64 numbers from svc_snapshot and without a nop.
    //
    constexpr int WIRE_SVC_NOP = 0;

    constexpr int ServerOps[]
    {
      5, // svc_snapshot
      3, // svc_serverCommand
      1, // svc_gamestate
      2, // svc_configstring
      4, // svc_matchdata
      6  // svc_EOF
    };

    static_assert(std::size(ServerOps) == Game::svc_EOF + 1);

    // The letters of the server commands, the game's first and the wire's,
    // which are x86's. We take them from the switches of the two games'
    // CG_DeployServerCommand() and CL_CGameNeedsServerCommand().
    //
    // Note that IW4x sends the stats as M on x86, which x64 uses for one of
    // its own commands, so we send them as G (see Game::SV_SetClientStat()).
    //
    // Note also that x64 compiles out the command that destroys a dynamic
    // entity, j on x86, so we take it as X, which x64 has no use for (see
    // CG_DestroyDynEntCommand()).
    //
    using ServerCommandLetter = std::pair<char, char>;

    constexpr ServerCommandLetter ServerCommandLetters[]
    {
      {'a', 'a'}, {'b', 'b'}, {'c', 'c'}, {'d', 'd'}, {'e', 'e'}, {'f', 'f'},
      {'g', 'g'}, {'h', 'k'}, {'i', 'l'}, {'j', 'm'}, {'k', 'n'}, {'l', 'o'},
      {'m', 'p'}, {'n', 'q'}, {'o', 'r'}, {'p', 's'}, {'q', 't'}, {'r', 'u'},
      {'s', 'v'}, {'t', 'w'}, {'u', 'x'}, {'v', 'y'}, {'w', 'A'}, {'x', 'B'},
      {'y', 'C'}, {'z', 'D'}, {'A', 'E'}, {'B', 'F'}, {'C', 'G'}, {'D', 'H'},
      {'E', 'I'}, {'F', 'J'}, {'G', 'M'}, {'H', 'L'}, {'J', 'N'}, {'K', 'O'},
      {'M', 'Q'}, {'N', 'R'}, {'O', 'S'}, {'P', 'T'}, {'Q', 'U'}, {'R', 'V'},
      {'S', 'W'}, {'U', 'h'}, {'V', 'i'}, {'X', 'j'}
    };

    constexpr char DESTROY_DYNENT_COMMAND = 'X';

    // Translate the letter that the command starts with in place, to the
    // wire's or back. A letter that the other side has no command for
    // empties the command, which both games ignore. IW4x's own commands
    // start with a number below the letters and are the same on both sides.
    //
    void TranslateServerCommand(char* command, const bool toWire)
    {
      if (!std::isalpha(static_cast<unsigned char>(*command)))
      {
        return;
      }

      const auto from(toWire ? &ServerCommandLetter::first : &ServerCommandLetter::second);
      const auto to(toWire ? &ServerCommandLetter::second : &ServerCommandLetter::first);

      const auto* i(std::ranges::find(ServerCommandLetters, *command, from));
      *command = i != std::ranges::end(ServerCommandLetters) ? (*i).*to : '\0';
    }

    // The scores for the scoreboard, b, are the letter, four numbers, and a
    // record of numbers for each player. The record has seven numbers on x86
    // and eight on x64, which adds one after them. Resize the records from
    // one width to the other, dropping the last number or adding a zero.
    //
    constexpr std::size_t SCORES_HEADER_FIELDS = 5;
    constexpr std::size_t WIRE_SCORES_FIELDS = 7;
    constexpr std::size_t GAME_SCORES_FIELDS = 8;

    std::string ResizeScores(const std::string_view command, const std::size_t from, const std::size_t to)
    {
      std::vector<std::string_view> fields;
      for (const auto field : std::views::split(command, ' '))
      {
        if (!field.empty())
        {
          fields.emplace_back(field.begin(), field.end());
        }
      }

      std::string r;
      for (std::size_t i = 0; i < std::min(fields.size(), SCORES_HEADER_FIELDS); ++i)
      {
        r += i != 0 ? " " : "";
        r += fields[i];
      }

      for (auto i = SCORES_HEADER_FIELDS; i + from <= fields.size(); i += from)
      {
        for (std::size_t j = 0; j < to; ++j)
        {
          r += ' ';
          r += j < from ? fields[i + j] : "0";
        }
      }

      return r;
    }

    // Return the server command as it goes over the wire.
    //
    std::string WireServerCommand(const std::string_view command)
    {
      std::string r(command);

      if (!r.empty() && r[0] == 'b')
      {
        r = ResizeScores(r, GAME_SCORES_FIELDS, WIRE_SCORES_FIELDS);
      }

      TranslateServerCommand(r.data(), true);
      return r;
    }

    // The netchan scrambles the client's messages with the last server
    // command the client has, the user commands with a key that hashes it
    // and the rest of the message with its text. So both sides must use the
    // command as it went over the wire. The client keeps the wire letter of
    // each of its commands, since the translation only changes the first.
    //
    const auto Com_HashKey(reinterpret_cast<int(*)(const char*, int)>(0x14027EB00));

    constexpr std::uintptr_t CLIENT_SERVER_COMMANDS = 0x140C19768;
    constexpr std::size_t SERVER_COMMAND_SIZE = 0x400;
    constexpr std::size_t MAX_RELIABLE_COMMANDS = 128;

    const auto* serverCommandSequence(reinterpret_cast<const int*>(0x140C19760));

    std::array<char, MAX_RELIABLE_COMMANDS> serverCommandLetters {};

    std::size_t ServerCommandSlot(const char* command)
    {
      const auto slot((reinterpret_cast<std::uintptr_t>(command) - CLIENT_SERVER_COMMANDS) / SERVER_COMMAND_SIZE);

      assert(slot < MAX_RELIABLE_COMMANDS);

      return slot;
    }

    // The client's reliable commands on the server, client_t::reliableCommands,
    // and the slot that the client acknowledged last.
    //
    constexpr std::size_t CLIENT_RELIABLE_COMMANDS = 0xA70;
    constexpr std::size_t CLIENT_RELIABLE_COMMAND_SIZE = 0x408;
    constexpr std::size_t CLIENT_RELIABLE_ACKNOWLEDGE = 0x20E74;

    // The client tells the server which of the button commands it runs, for
    // the scripts that wait on them. x86 sends nt and the command and x64 n
    // and the index of the command in its table of them, whose first is the
    // empty one. On the wire we keep the x86 form. Since x86 tells of any
    // command, we take one outside the table as the empty one.
    //
    const auto notifyCommands(std::span(reinterpret_cast<const char* const*>(0x14041EC90), 78));

    // The client asks for the scores with score on x86 and s on x64.
    //
    constexpr auto SCORES_REQUEST("score");

    // The server scrambles its messages to a client with the last command it
    // took from that client, and the client unscrambles them with the command
    // as it sent it. So the server keeps the command as it came over the
    // wire.
    //
    std::array<char, SERVER_COMMAND_SIZE> lastClientCommand {};

    const auto MSG_ReadString(reinterpret_cast<char*(*)(Game::msg_t*, char*, int)>(0x140200A80));

    // Note that x64 tokenizes the server command for the cgame with another
    // copy of Cmd_TokenizeString().
    //
    const auto CL_Cmd_TokenizeString(reinterpret_cast<void(*)(const char*)>(0x1401E7AD0));

    // x86 has two more hud element types than x64, the map and the game type
    // names after the player name, so that every later type is two more on
    // the wire. x64 has no use for the two names and shows them as text.
    //
    constexpr int WIRE_HE_TYPE_MAPNAME = 4;
    constexpr int WIRE_HE_TYPE_GAMETYPE = 5;
    constexpr int WIRE_HE_TYPES = 2;

    constexpr std::size_t MAX_HUDELEMS = 31;

    Game::he_type_t GameHudElemType(const int type)
    {
      if (type == WIRE_HE_TYPE_MAPNAME || type == WIRE_HE_TYPE_GAMETYPE)
      {
        return Game::HE_TYPE_TEXT;
      }

      return static_cast<Game::he_type_t>(type > WIRE_HE_TYPE_GAMETYPE ? type - WIRE_HE_TYPES : type);
    }

    Game::he_type_t WireHudElemType(const int type)
    {
      return static_cast<Game::he_type_t>(type >= WIRE_HE_TYPE_MAPNAME ? type + WIRE_HE_TYPES : type);
    }

    void WireHudElems(std::array<Game::hudelem_s, MAX_HUDELEMS>& r, const Game::hudelem_s* elems, const int count)
    {
      assert(count >= 0 && static_cast<std::size_t>(count) <= r.size());

      std::copy_n(elems, count, r.begin());

      for (auto i(0); i < count; ++i)
      {
        r[i].type = WireHudElemType(r[i].type);
      }
    }
  }

  int Compatibility::MSG_ReadServerOp(Game::msg_t* msg)
  {
    int op;

    do
    {
      op = Game::MSG_ReadByte(msg);
    }
    while (op == WIRE_SVC_NOP && !msg->overflowed);

    // An op that x64 has none for reads as the one past svc_EOF, which the
    // game takes for a broken message.
    //
    const auto* i(std::ranges::find(ServerOps, op));
    return static_cast<int>(i - std::ranges::begin(ServerOps));
  }

  void Compatibility::MSG_WriteServerOp(Game::msg_t* msg, const int op)
  {
    assert(op >= 0 && op < static_cast<int>(std::size(ServerOps)));

    Game::MSG_WriteByte(msg, ServerOps[op]);
  }

  void Compatibility::MSG_WriteServerCommand(Game::msg_t* msg, const char* command)
  {
    Game::MSG_WriteString(msg, WireServerCommand(command).c_str());
  }

  int Compatibility::SV_HashServerCommand(const char* command, const int length)
  {
    return Com_HashKey(WireServerCommand(command).c_str(), length);
  }

  void Compatibility::SV_Netchan_Decode_Hk(char* client, unsigned char* data, const int length)
  {
    const auto slot(*reinterpret_cast<const int*>(client + CLIENT_RELIABLE_ACKNOWLEDGE) & (MAX_RELIABLE_COMMANDS - 1));
    auto* command(client + CLIENT_RELIABLE_COMMANDS + slot * CLIENT_RELIABLE_COMMAND_SIZE);

    // Note that the client keeps a command that went out empty as zeros to
    // its end, and that the scrambling starts the text over at a zero past
    // its first letter. So we clear the whole command before we put the
    // wire one in.
    //
    std::array<char, SERVER_COMMAND_SIZE> game;
    std::memcpy(game.data(), command, game.size());

    const auto wire(WireServerCommand(std::string_view(command, strnlen(command, game.size()))));

    std::memset(command, 0, game.size());
    std::memcpy(command, wire.data(), std::min(wire.size(), game.size() - 1));

    SV_Netchan_Decode_Original(client, data, length);

    std::memcpy(command, game.data(), game.size());
  }

  void Compatibility::CL_StoreServerCommand(char* dest, const char* command, const int size)
  {
    Game::I_strncpyz(dest, command, size);

    serverCommandLetters[ServerCommandSlot(dest)] = *dest;
    TranslateServerCommand(dest, false);
  }

  void* Compatibility::CL_ClearConnection(void* connection, const int value, const std::size_t size)
  {
    serverCommandLetters.fill('\0');
    return std::memset(connection, value, size);
  }

  int Compatibility::CL_HashServerCommand(const char* command, const int length)
  {
    std::array<char, SERVER_COMMAND_SIZE> wire;
    std::memcpy(wire.data(), command, wire.size());

    wire.front() = serverCommandLetters[ServerCommandSlot(command)];
    wire.back() = '\0';

    return Com_HashKey(wire.data(), length);
  }

  void Compatibility::CL_Netchan_Transmit_Hk(void* chan, unsigned char* data, const int length)
  {
    const auto slot(*serverCommandSequence & (MAX_RELIABLE_COMMANDS - 1));

    auto& letter(*reinterpret_cast<char*>(CLIENT_SERVER_COMMANDS + slot * SERVER_COMMAND_SIZE));
    const auto game(std::exchange(letter, serverCommandLetters[slot]));

    CL_Netchan_Transmit_Original(chan, data, length);

    letter = game;
  }

  void Compatibility::CL_TokenizeServerCommand(const char* command)
  {
    // The client keeps the scores as they came, since it hashes and
    // scrambles with them, and the cgame reads them at its own width.
    //
    if (*command == 'b')
    {
      static std::string scores;
      scores = ResizeScores(command, WIRE_SCORES_FIELDS, GAME_SCORES_FIELDS);
      command = scores.c_str();
    }

    CL_Cmd_TokenizeString(command);
  }

  int Compatibility::CL_FormatNotifyCommand(char* buffer, const int size, [[maybe_unused]] const char* format, const int index)
  {
    assert(index > 0 && index < static_cast<int>(notifyCommands.size()));

    return std::snprintf(buffer, size, "nt %s", notifyCommands[index]);
  }

  void Compatibility::CL_AddScoresRequest(const int localClientNum, [[maybe_unused]] const char* command)
  {
    Game::CL_AddReliableCommand(localClientNum, SCORES_REQUEST);
  }

  bool Compatibility::FS_PureServerSetLoadedIwds([[maybe_unused]] const char* sums, [[maybe_unused]] const char* names)
  {
    return Utils::Hook::Call<bool(const char*, const char*)>(0x1401FD4A0)("", ""); // FS_PureServerSetLoadedIwds
  }

  char* Compatibility::SV_ReadClientCommand(Game::msg_t* msg, char* buffer, const int size)
  {
    auto* command(MSG_ReadString(msg, buffer, size));
    Game::I_strncpyz(lastClientCommand.data(), command, static_cast<int>(lastClientCommand.size()));

    if (std::strcmp(command, SCORES_REQUEST) == 0)
    {
      Game::I_strncpyz(buffer, "s", size);
      return buffer;
    }

    if (std::strncmp(command, "nt ", 3) != 0)
    {
      return command;
    }

    const auto i(std::ranges::find_if(notifyCommands, [command](const char* name)
    {
      return name != nullptr && *name != '\0' && std::strcmp(name, command + 3) == 0;
    }));

    const auto index(i != notifyCommands.end() ? std::ranges::distance(notifyCommands.begin(), i) : 0);

    std::snprintf(buffer, size, "n %i", static_cast<int>(index));
    return buffer;
  }

  int Compatibility::SV_StoreClientCommand(char* buffer, const int size, [[maybe_unused]] const char* format, [[maybe_unused]] const char* command)
  {
    return std::snprintf(buffer, size, "%s", lastClientCommand.data());
  }

  void Compatibility::MSG_ReadDeltaHudElems_Hk(Game::msg_t* msg, const int time, const Game::hudelem_s* from, Game::hudelem_s* to, const int count)
  {
    // The message is a delta from the elements as they went over the wire,
    // and an element that it leaves alone keeps the one it is from.
    //
    std::array<Game::hudelem_s, MAX_HUDELEMS> wire;
    WireHudElems(wire, from, count);

    MSG_ReadDeltaHudElems_Original(msg, time, wire.data(), to, count);

    for (auto i(0); i < count; ++i)
    {
      to[i].type = GameHudElemType(to[i].type);
    }
  }

  void Compatibility::MSG_WriteDeltaHudElems_Hk(void* snapInfo, Game::msg_t* msg, const int time, const Game::hudelem_s* from, const Game::hudelem_s* to, const int count)
  {
    std::array<Game::hudelem_s, MAX_HUDELEMS> wireFrom;
    std::array<Game::hudelem_s, MAX_HUDELEMS> wireTo;

    WireHudElems(wireFrom, from, count);
    WireHudElems(wireTo, to, count);

    MSG_WriteDeltaHudElems_Original(snapInfo, msg, time, wireFrom.data(), wireTo.data(), count);
  }

  void Compatibility::MSG_SetDefaultUserCmd_Hk(Game::playerState_s* ps, Game::usercmd_s* cmd)
  {
    MSG_SetDefaultUserCmd_Original(ps, cmd);

    // Note that x64 doesn't hold the sprint button in the default command of
    // a sprinting player, which x86 does. Without it a sprint on one side is
    // none on the other.
    //
    if ((ps->otherFlags & Game::POF_PLAYER) && (ps->pm_flags & Game::PMF_SPRINTING))
    {
      cmd->buttons |= Game::CMD_BUTTON_SPRINT;
    }
  }

  bool Compatibility::CG_DestroyDynEntCommand(const Command::Params* params)
  {
    // Note that x86 reads the arguments of the command the way it reads
    // those of j, which we take as X.
    //
    const auto dynEntId(static_cast<unsigned __int16>(std::atol(params->get(1))));
    const auto drawType(std::atol(params->get(2)));

    const float hitPos[3]
    {
      static_cast<float>(std::atof(params->get(3))),
      static_cast<float>(std::atof(params->get(4))),
      static_cast<float>(std::atof(params->get(5)))
    };

    const float hitDir[3]
    {
      static_cast<float>(std::atof(params->get(6))),
      static_cast<float>(std::atof(params->get(7))),
      static_cast<float>(std::atof(params->get(8)))
    };

    // Note that x86 trusts the server with the dynamic entity, so that a
    // server can index past the lists. We drop such a command instead.
    //
    if (drawType < Game::DYNENT_DRAW_MODEL || drawType >= Game::DYNENT_DRAW_COUNT ||
        dynEntId >= Game::cm->dynEntCount[drawType])
    {
      return true;
    }

    // Note that the command doesn't tell us its local client, and x64 only
    // has the first.
    //
    Game::DynEntCl_DestroyEvent(0, dynEntId, static_cast<Game::DynEntityDrawType>(drawType), hitPos, hitDir);
    return true;
  }

  Compatibility::Compatibility()
  {
    // The server message ops, as the client reads them in
    // CL_ParseServerMessage() and CL_ParseGamestate().
    //
    for (const auto address : {0x140100D55, 0x1400FFD6F, 0x1401000F8})
    {
      Utils::Hook(address, MSG_ReadServerOp, HOOK_CALL).install()->quick();
    }

    // And as the server writes them, and the client into the gamestate of
    // a demo (see Components::Theatre for the configstring op).
    //
    for (const auto address :
         {
           0x14023731C, 0x14023736C, 0x14023757B, // SV_SendClientGameState
           0x14023FAA8, 0x140240E98,
           0x14023FEA6, 0x14023FF49, 0x14023FF8E,
           0x1402404AA, 0x140240524, 0x1402405C8, 0x140240616,
           0x140241167, // SV_WriteSnapshotToClient
           0x1402381ED, // match data
           0x1400FCF8B, 0x1400FD18E, 0x1400FD1BD // CL_Record_f
         })
    {
      Utils::Hook(address, MSG_WriteServerOp, HOOK_CALL).install()->quick();
    }

    Utils::Hook(0x1402375DC, MSG_WriteServerOp, HOOK_JUMP).install()->quick();

    // The server commands, as the server writes the reliable commands of
    // a client, hashes the one the client acknowledged into the key of its
    // user commands, and unscrambles the client's message with it.
    //
    for (const auto address : {0x14023FAD0, 0x140240EC0, 0x14023FF62, 0x1402404D6, 0x1402405E5})
    {
      Utils::Hook(address, MSG_WriteServerCommand, HOOK_CALL).install()->quick();
    }

    Utils::Hook(0x140237B5F, SV_HashServerCommand, HOOK_CALL).install()->quick();
    SV_Netchan_Decode_Original = Utils::Hook::Detour(0x14023D300, 5, &SV_Netchan_Decode_Hk);

    // And as the client stores them, forgets them with the connection,
    // hashes the last one into the key of its user commands, scrambles its
    // message with it, and tokenizes one for the cgame.
    //
    Utils::Hook(0x140100E26, CL_StoreServerCommand, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400F8FA8, CL_ClearConnection, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400F7853, CL_HashServerCommand, HOOK_CALL).install()->quick();
    CL_Netchan_Transmit_Original = Utils::Hook::Detour(0x1400FF6E0, 5, &CL_Netchan_Transmit_Hk);
    Utils::Hook(0x1400F3DA2, CL_TokenizeServerCommand, HOOK_CALL).install()->quick();

    // The client commands, as the client sends a notify command and asks for
    // the scores, and as the server reads one and stores it.
    //
    Utils::Hook(0x1400F877E, CL_FormatNotifyCommand, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400D069F, CL_AddScoresRequest, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400E3F9D, CL_AddScoresRequest, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400E5BE2, CL_AddScoresRequest, HOOK_JUMP).install()->quick();
    Utils::Hook(0x1402354FB, SV_ReadClientCommand, HOOK_CALL).install()->quick();
    Utils::Hook(0x140235699, SV_StoreClientCommand, HOOK_CALL).install()->quick();

    // The hud elements of the player state, both of its lists.
    //
    AssertSize(Game::hudelem_s, 0xA8);
    MSG_ReadDeltaHudElems_Original = Utils::Hook::Detour(0x1402020A0, 5, &MSG_ReadDeltaHudElems_Hk);
    MSG_WriteDeltaHudElems_Original = Utils::Hook::Detour(0x140206C90, 5, &MSG_WriteDeltaHudElems_Hk);

    // The first user command of a packet goes as a delta from a default one
    // that both sides build from the player state, the client in
    // CL_WritePacket() and the server in SV_UserMove().
    //
    AssertOffset(Game::playerState_s, pm_flags, 0xC);
    AssertOffset(Game::playerState_s, otherFlags, 0x10);
    MSG_SetDefaultUserCmd_Original = Utils::Hook::Detour(0x140200C40, 5, &MSG_SetDefaultUserCmd_Hk);

    // The iwds of a pure server, which CL_SystemInfoChanged() has the game
    // load only those of. An x86 client has the iw4x iwds that an x86 server
    // names, but we have our own, which the game would unload. So we load
    // all of ours, as we do for a server that isn't pure. Note that
    // QuickPatch already keeps the game from dropping us over them.
    //
    Utils::Hook(0x1401001D7, FS_PureServerSetLoadedIwds, HOOK_CALL).install()->quick();

    ServerCommands::OnCommand(DESTROY_DYNENT_COMMAND, CG_DestroyDynEntCommand);
  }
}
