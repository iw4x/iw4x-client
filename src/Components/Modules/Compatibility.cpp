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
    // Every server message is a run of ops, each a byte followed by whatever
    // that op carries, until svc_EOF. x86 starts the numbering with svc_nop,
    // which carries nothing and which the client just skips. It's presumably
    // a leftover from Quake 3 and we don't know of anything that actually
    // sends it. But who knows what some old server might do, and skipping it
    // costs us nothing, so we skip it as well. x64 doesn't have it at all.
    //
    constexpr int WIRE_SVC_NOP = 0;

    // The x86 number of each x64 op, indexed by the x64 one. Besides dropping
    // svc_nop, x64 moved svc_snapshot from the end to the very front, which is
    // why nothing lines up and why we need a table rather than an offset.
    //
    constexpr int ServerOps[]
    {
      5, // svc_snapshot
      3, // svc_serverCommand
      1, // svc_gamestate
      2, // svc_configstring
      4, // svc_matchdata
      6 // svc_EOF
    };

    static_assert(std::size(ServerOps) == Game::svc_EOF + 1);

    // A server command is a line of text that the server queues for a client
    // reliably, and its first character says what it is (e is a print, for
    // example, and b the scores). x64 renamed most of them. It looks as if
    // three of x86's commands, h through j, were moved out of the way and
    // most of the ones after them shifted down by three, which is perhaps
    // what happens when someone moves a few cases out of the middle of an
    // enum. But that's only roughly true, so rather than compute the letters
    // we spell out every pair. And the pairs have to be exact: a wrong letter
    // doesn't fail, it just makes the client do something else, like take a
    // configstring change for a print.
    //
    // Each pair is the x64 letter followed by the x86 one. We put them
    // together by comparing what each case of CG_DeployServerCommand() and
    // CL_CGameNeedsServerCommand() does in the two games. For the commands
    // every game uses we are fairly sure of the result. For the rarely used
    // ones it's about as far as we can check them without a server that sends
    // them, so some of these may well be wrong. If some odd command misbehaves
    // only between the two builds, then this table is probably the first place
    // to look.
    //
    // Two of the pairs are our own rather than the game's. G is IW4x's stats,
    // which x86 sends as M, but on x64 M is one of the game's own commands, so
    // there we take G instead (see Game::SV_SetClientStat()). And X is the
    // destruction of a dynamic entity, which x86 sends as j and which x64
    // compiled out, so we handle it ourselves under a letter that's free on
    // x64 (see CG_DestroyDynEntCommand()).
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

    // Translate the command's letter in place, to x86 if toWire is true and to
    // x64 otherwise.
    //
    // What if the other game doesn't have the command? We could send it on as
    // it is, but then the other side would run whatever its own command with
    // that letter happens to be, which seems a lot worse than not running it
    // at all. So in that case we empty the command, which both games ignore.
    //
    // Note that IW4x's own commands use the characters below the letters (20
    // and up, see ServerCommands). Those mean the same thing in both builds
    // since we wrote both, so we leave them alone.
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

    // The scores are the one command whose arguments differ. It's b and four
    // numbers followed by a record of numbers per player, and x64 has one more
    // number at the end of each record than x86 does. We are not sure what
    // that number is, maybe some newer stat that the later revision added to
    // the scoreboard. Either way we don't really have to know: towards x86 we
    // drop it and towards x64 we add a zero, which the x64 scoreboard seems
    // to be perfectly happy with.
    //
    constexpr std::size_t SCORES_HEADER_FIELDS = 5;
    constexpr std::size_t WIRE_SCORES_FIELDS = 7;
    constexpr std::size_t GAME_SCORES_FIELDS = 8;

    // Return the scores with each record resized from from to to fields,
    // leaving the header alone. A trailing partial record, if any, is dropped.
    //
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

    // Return the game's server command as it goes over the wire.
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

    // Now for the part that took the most head scratching.
    //
    // Both sides scramble their messages, and they do it with the reliable
    // commands. The client, in CL_WritePacket(), keys its user commands with
    // Com_HashKey() of the last server command it got, and
    // CL_Netchan_Transmit() then xors the text of that same command over the
    // rest of the message. The server unscrambles the message with its own
    // copy of the command, which it finds by the reliableAcknowledge that the
    // client sends (see SV_Netchan_Decode()). The other way around, the server
    // scrambles what it sends with the last command it got from the client,
    // and the client unscrambles it with its own copy of that.
    //
    // It isn't security in any real sense (perhaps it was meant to keep casual
    // packet editors at bay), but it does mean that both sides have to agree
    // on the text of these commands byte for byte. Which is a problem for us,
    // since by the time the game hashes or scrambles with a command we have
    // long translated it. If we left it at that, then the two sides would key
    // and xor with different text and the server would read noise where the
    // user commands should be.
    //
    // So what can we do? One option is to keep the commands the way they came
    // in and translate them every time the game reads one. But the game reads
    // them in quite a few places, and we would have to find every one of them
    // and hope we didn't miss any. The other is to keep them translated and to
    // put the wire form back just for the moment the game hashes or
    // scrambles, which is only a handful of places. The latter felt a lot
    // less fragile, so that's what we do.
    //
    // On the server we rebuild the wire form of the command and put it in
    // place while the server hashes and unscrambles, and then restore the
    // game's (see SV_HashServerCommand() and SV_Netchan_Decode_Hk()). This is
    // the very same translation we apply when we send the command, so the
    // result should be exactly what the client got. On the client we can get
    // away with less, since the translation only ever changes the first letter
    // of what the client stores: we remember the letter that each slot came in
    // with (serverCommandLetters) and swap it back in while the client hashes
    // and scrambles (see CL_HashServerCommand() and CL_Netchan_Transmit_Hk()).
    // You might wonder about the scores, the one command we change beyond its
    // letter. But we only resize those for the cgame (see
    // CL_TokenizeServerCommand()), so the client's own copy is still the wire
    // form apart from the letter. Finally, for the client commands that the
    // server scrambles with, we keep the last one as it came in, before we
    // translate it, and have the server store that copy instead (see
    // lastClientCommand).
    //
    // It's a bit of a dance, admittedly, and anyone who changes how a command
    // is translated will want to keep all three of these in mind.
    //
    const auto Com_HashKey(reinterpret_cast<int(*)(const char*, int)>(0x14027EB00));

    // The client connection's serverCommands, and the sequence of the last one
    // the client got.
    //
    constexpr std::uintptr_t CLIENT_SERVER_COMMANDS = 0x140C19768;
    constexpr std::size_t SERVER_COMMAND_SIZE = 0x400;
    constexpr std::size_t MAX_RELIABLE_COMMANDS = 128;

    const auto* serverCommandSequence(reinterpret_cast<const int*>(0x140C19760));

    // The letter that each slot of serverCommands came in with.
    //
    std::array<char, MAX_RELIABLE_COMMANDS> serverCommandLetters {};

    // Return the slot of a command in serverCommands from its address.
    //
    std::size_t ServerCommandSlot(const char* command)
    {
      const auto slot((reinterpret_cast<std::uintptr_t>(command) - CLIENT_SERVER_COMMANDS) / SERVER_COMMAND_SIZE);

      assert(slot < MAX_RELIABLE_COMMANDS);

      return slot;
    }

    // client_t::reliableCommands with the size of its entries, followed by
    // client_t::reliableAcknowledge.
    //
    constexpr std::size_t CLIENT_RELIABLE_COMMANDS = 0xA70;
    constexpr std::size_t CLIENT_RELIABLE_COMMAND_SIZE = 0x408;
    constexpr std::size_t CLIENT_RELIABLE_ACKNOWLEDGE = 0x20E74;

    // When a client runs one of the button commands, it tells the server, so
    // that the scripts waiting on that command get notified. x86 sends this as
    // nt followed by the command's name, while x64 sends n followed by the
    // command's index into this table (which starts with an empty entry), and
    // the server then looks the name up there. Presumably someone decided the
    // names were a waste of bandwidth.
    //
    // We always send the x86 form and turn it into x64's on the server. But
    // what about a command that isn't in the table, which x86 is free to name?
    // That one becomes the empty entry. It's probably fine, seeing that no x64
    // script can wait on a command outside the table anyway, though we
    // haven't tried every mod out there.
    //
    const auto notifyCommands(std::span(reinterpret_cast<const char* const*>(0x14041EC90), 78));

    // The x86 request for the scores, which x64 shortened to s (perhaps for
    // the same bandwidth reasons).
    //
    constexpr auto SCORES_REQUEST("score");

    // The last client command as it came in, before SV_ReadClientCommand()
    // translated it, which is what the server has to scramble with (see
    // Com_HashKey above).
    //
    std::array<char, SERVER_COMMAND_SIZE> lastClientCommand {};

    const auto MSG_ReadString(reinterpret_cast<char*(*)(Game::msg_t*, char*, int)>(0x140200A80));

    // Note that x64 has a separate copy of Cmd_TokenizeString() for the server
    // commands it passes to the cgame, and that's the one we need here.
    //
    const auto CL_Cmd_TokenizeString(reinterpret_cast<void(*)(const char*)>(0x1401E7AD0));

    // The player state carries the hud elements that the scripts create, each
    // with a type that says how to draw it. x86 has two types that x64
    // doesn't, one that shows the map name and one the game type name, and
    // since they sit in the middle of the list, every type past them is two
    // higher on x86 than on x64.
    //
    // We don't know of a script that actually uses either of the two. But
    // what if one does? Plain text is about the closest x64 can draw, so
    // that's what we turn them into. This makes the translation lossy towards
    // x64, which is fine since it never has to go back: an x64 server has no
    // such types to send in the first place.
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

  // Read an op as x86 numbers it and return the x64 number for it, skipping
  // any svc_nop along the way.
  //
  int Compatibility::MSG_ReadServerOp(Game::msg_t* msg)
  {
    int op;

    do
    {
      op = Game::MSG_ReadByte(msg);
    }
    while (op == WIRE_SVC_NOP && !msg->overflowed);

    // What if x64 doesn't have the op (which most likely means the message is
    // garbage)? Then this gives the one past svc_EOF, which both
    // CL_ParseServerMessage() and CL_ParseGamestate() take for a broken
    // message: they mark it overflowed and drop the rest. That's also what we
    // would want, since we couldn't make sense of the rest anyway.
    //
    const auto* i(std::ranges::find(ServerOps, op));
    return static_cast<int>(i - std::ranges::begin(ServerOps));
  }

  void Compatibility::MSG_WriteServerOp(Game::msg_t* msg, const int op)
  {
    assert(op >= 0 && op < static_cast<int>(std::size(ServerOps)));

    Game::MSG_WriteByte(msg, ServerOps[op]);
  }

  // Send a reliable command to a client in its wire form.
  //
  void Compatibility::MSG_WriteServerCommand(Game::msg_t* msg, const char* command)
  {
    Game::MSG_WriteString(msg, WireServerCommand(command).c_str());
  }

  // Hash the command that the client acknowledged in its wire form, the same
  // as the client does (see Com_HashKey above).
  //
  int Compatibility::SV_HashServerCommand(const char* command, const int length)
  {
    return Com_HashKey(WireServerCommand(command).c_str(), length);
  }

  // Unscramble the client's message with the wire form of the command it
  // acknowledged (see Com_HashKey above). We put that form in the client's
  // slot just for the duration of the call and then restore the game's.
  //
  void Compatibility::SV_Netchan_Decode_Hk(char* client, unsigned char* data, const int length)
  {
    const auto slot(*reinterpret_cast<const int*>(client + CLIENT_RELIABLE_ACKNOWLEDGE) & (MAX_RELIABLE_COMMANDS - 1));
    auto* command(client + CLIENT_RELIABLE_COMMANDS + slot * CLIENT_RELIABLE_COMMAND_SIZE);

    // The server unscrambles with the command's text up to its first zero and
    // then starts over from the beginning (that's the cmovz in its loop). The
    // client stores its copy padded with zeros, so we clear the slot before
    // putting the wire form in. Otherwise the tail of a longer game form could
    // end up in the key.
    //
    // @@ A command that x86 doesn't have goes out empty, but here its text
    //    still follows the emptied letter and the unscrambling picks it up
    //    once it starts over, while the client only has zeros. So the two
    //    would disagree. It's probably rare (it takes a server command that
    //    x86 has no letter for), but we should copy the wire form only up to
    //    its first zero.
    //
    std::array<char, SERVER_COMMAND_SIZE> game;
    std::memcpy(game.data(), command, game.size());

    const auto wire(WireServerCommand(std::string_view(command, strnlen(command, game.size()))));

    std::memset(command, 0, game.size());
    std::memcpy(command, wire.data(), std::min(wire.size(), game.size() - 1));

    SV_Netchan_Decode_Original(client, data, length);

    std::memcpy(command, game.data(), game.size());
  }

  // Store a server command that the client got, remembering the letter it
  // came in with before we translate it for the game.
  //
  void Compatibility::CL_StoreServerCommand(char* dest, const char* command, const int size)
  {
    Game::I_strncpyz(dest, command, size);

    serverCommandLetters[ServerCommandSlot(dest)] = *dest;
    TranslateServerCommand(dest, false);
  }

  // CL_Disconnect() clears the client connection, server commands and all,
  // with this memset, so we forget the letters along with them.
  //
  void* Compatibility::CL_ClearConnection(void* connection, const int value, const std::size_t size)
  {
    serverCommandLetters.fill('\0');
    return std::memset(connection, value, size);
  }

  // Hash the last server command in its wire form, which is the stored text
  // with the letter it came in with.
  //
  int Compatibility::CL_HashServerCommand(const char* command, const int length)
  {
    std::array<char, SERVER_COMMAND_SIZE> wire;
    std::memcpy(wire.data(), command, wire.size());

    wire.front() = serverCommandLetters[ServerCommandSlot(command)];
    wire.back() = '\0';

    return Com_HashKey(wire.data(), length);
  }

  // Scramble the message with the last server command in its wire form. For
  // that we swap the letter it came in with back in, just for the duration
  // of the call.
  //
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
    // The client keeps the scores the way they came in, since it hashes and
    // scrambles with them. It's only the cgame that wants them at its own
    // width, so this is where we resize the records.
    //
    if (*command == 'b')
    {
      static std::string scores;
      scores = ResizeScores(command, WIRE_SCORES_FIELDS, GAME_SCORES_FIELDS);
      command = scores.c_str();
    }

    CL_Cmd_TokenizeString(command);
  }

  // Send the button command notification the x86 way, by name (see
  // notifyCommands).
  //
  int Compatibility::CL_FormatNotifyCommand(char* buffer, const int size, [[maybe_unused]] const char* format, const int index)
  {
    assert(index > 0 && index < static_cast<int>(notifyCommands.size()));

    return std::snprintf(buffer, size, "nt %s", notifyCommands[index]);
  }

  // Ask for the scores the x86 way.
  //
  void Compatibility::CL_AddScoresRequest(const int localClientNum, [[maybe_unused]] const char* command)
  {
    Game::CL_AddReliableCommand(localClientNum, SCORES_REQUEST);
  }

  // Load the iwds as if the server weren't pure.
  //
  // For a pure server, CL_ParseGamestate() (presumably with
  // CL_SystemInfoChanged() inlined into it) has the game load only the iwds
  // that the server lists. An x86 server lists its own iw4x iwds, which an
  // x64 client doesn't have since it comes with its own, and loading only
  // those would unload all of ours. So we load everything, the same as for a
  // server that isn't pure.
  //
  // Strictly speaking we only need this for an x86 server, and doing it for
  // an x64 one too is a bit much. Perhaps we should tell the two
  // apart at some point, but for now it seems harmless (see also QuickPatch,
  // which keeps the game from dropping us over the iwds it can't find).
  //
  bool Compatibility::FS_PureServerSetLoadedIwds([[maybe_unused]] const char* sums, [[maybe_unused]] const char* names)
  {
    return Utils::Hook::Call<bool(const char*, const char*)>(0x1401FD4A0)("", ""); // FS_PureServerSetLoadedIwds
  }

  // Read a client command and translate it for the game, keeping the command
  // as it came in for the scrambling (see lastClientCommand).
  //
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

    // A name that isn't in the table becomes the empty entry, 0.
    //
    std::snprintf(buffer, size, "n %i", static_cast<int>(index));
    return buffer;
  }

  // The server stores the last client command for the scrambling with this
  // sprintf(), so store the one that came in rather than our translation.
  //
  int Compatibility::SV_StoreClientCommand(char* buffer, const int size, [[maybe_unused]] const char* format, [[maybe_unused]] const char* command)
  {
    return std::snprintf(buffer, size, "%s", lastClientCommand.data());
  }

  // Read the hud elements of the player state, translating their types.
  //
  void Compatibility::MSG_ReadDeltaHudElems_Hk(Game::msg_t* msg, const int time, const Game::hudelem_s* from, Game::hudelem_s* to, const int count)
  {
    std::array<Game::hudelem_s, MAX_HUDELEMS> wire;
    WireHudElems(wire, from, count);

    // The message is a delta from the elements as they went over the wire, so
    // the game has to see the x86 types of the old elements to read it right.
    // Note also that it copies whatever the delta leaves alone from the old
    // elements, wire types included, which is why afterwards we translate the
    // types of all of them back, whether they changed or not.
    //
    MSG_ReadDeltaHudElems_Original(msg, time, wire.data(), to, count);

    for (auto i(0); i < count; ++i)
    {
      to[i].type = GameHudElemType(to[i].type);
    }
  }

  // Write the hud elements as a delta between their wire forms.
  //
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

    // Both sides send the first user command of a packet as a delta from this
    // default, which the client builds in CL_WritePacket() and the server in
    // SV_UserMove(). x86 holds the sprint button in it for a sprinting player
    // and x64 doesn't. That looks harmless at first. But since the delta only
    // carries what differs from the default, a sprinting x86 client presumably
    // sends no sprint button at all, which an x64 server would then take as
    // released (and the other way around for an x64 client on an x86 server).
    // So we hold it as well, and the two sides agree whichever of them is x64.
    //
    if ((ps->otherFlags & Game::POF_PLAYER) && (ps->pm_flags & Game::PMF_SPRINTING))
    {
      cmd->buttons |= Game::CMD_BUTTON_SPRINT;
    }
  }

  // Destroy a dynamic entity, which x64 compiled out along with its command.
  // We parse the arguments the way x86 parses those of j.
  //
  bool Compatibility::CG_DestroyDynEntCommand(const Command::Params* params)
  {
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

    // This check is ours and the one place where we deliberately part ways
    // with x86, which indexes the dynamic entity lists with whatever the
    // server sends. Maybe that never goes wrong in practice, but the check
    // costs us nothing and a bad server would otherwise crash the client.
    //
    if (drawType < Game::DYNENT_DRAW_MODEL || drawType >= Game::DYNENT_DRAW_COUNT ||
        dynEntId >= Game::cm->dynEntCount[drawType])
    {
      return true;
    }

    // x64 only has the one local client, and the command doesn't name one
    // anyway.
    //
    Game::DynEntCl_DestroyEvent(0, dynEntId, static_cast<Game::DynEntityDrawType>(drawType), hitPos, hitDir);
    return true;
  }

  Compatibility::Compatibility()
  {
    // The server message ops as the client reads them, in
    // CL_ParseServerMessage() at the first address and in CL_ParseGamestate()
    // at the other two.
    //
    for (const auto address : {0x140100D55, 0x1400FFD6F, 0x1401000F8})
    {
      Utils::Hook(address, MSG_ReadServerOp, HOOK_CALL).install()->quick();
    }

    // And as the server writes them, as well as the client when it records a
    // demo (Theatre writes the rest of a demo's ops itself).
    //
    for (const auto address :
         {
           0x14023731C, 0x14023736C, 0x14023757B, // SV_SendClientGameState
           0x14023FAA8, 0x140240E98,
           0x14023FEA6, 0x14023FF49, 0x14023FF8E,
           0x1402404AA, 0x140240524, 0x1402405C8, 0x140240616,
           0x140241167, // SV_WriteSnapshotToClient
           0x1402381ED, // Match data.
           0x1400FCF8B, 0x1400FD18E, 0x1400FD1BD // CL_Record_f
         })
    {
      Utils::Hook(address, MSG_WriteServerOp, HOOK_CALL).install()->quick();
    }

    // SV_SendClientGameState() ends with a tail call for its svc_EOF.
    //
    Utils::Hook(0x1402375DC, MSG_WriteServerOp, HOOK_JUMP).install()->quick();

    // The server commands on the server. It writes a client's reliable
    // commands, and later it keys the client's user commands with the one
    // the client acknowledged and unscrambles the client's message with it.
    //
    for (const auto address : {0x14023FAD0, 0x140240EC0, 0x14023FF62, 0x1402404D6, 0x1402405E5})
    {
      Utils::Hook(address, MSG_WriteServerCommand, HOOK_CALL).install()->quick();
    }

    Utils::Hook(0x140237B5F, SV_HashServerCommand, HOOK_CALL).install()->quick();
    SV_Netchan_Decode_Original = Utils::Hook::Detour(0x14023D300, 5, &SV_Netchan_Decode_Hk);

    // And on the client. It stores them and forgets them along with the
    // connection, keys its user commands with the last one and scrambles its
    // message with it, and finally tokenizes one for the cgame.
    //
    Utils::Hook(0x140100E26, CL_StoreServerCommand, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400F8FA8, CL_ClearConnection, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400F7853, CL_HashServerCommand, HOOK_CALL).install()->quick();
    CL_Netchan_Transmit_Original = Utils::Hook::Detour(0x1400FF6E0, 5, &CL_Netchan_Transmit_Hk);
    Utils::Hook(0x1400F3DA2, CL_TokenizeServerCommand, HOOK_CALL).install()->quick();

    // The client commands, as the client sends a notification and asks for
    // the scores (from three places, the last one a tail call), and as the
    // server reads one and stores it.
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

    // The default user command (see MSG_SetDefaultUserCmd_Hk()).
    //
    AssertOffset(Game::playerState_s, pm_flags, 0xC);
    AssertOffset(Game::playerState_s, otherFlags, 0x10);
    MSG_SetDefaultUserCmd_Original = Utils::Hook::Detour(0x140200C40, 5, &MSG_SetDefaultUserCmd_Hk);

    // The iwds of a pure server (see FS_PureServerSetLoadedIwds()).
    //
    Utils::Hook(0x1401001D7, FS_PureServerSetLoadedIwds, HOOK_CALL).install()->quick();

    // And the dynamic entities that x64 no longer destroys itself (see
    // CG_DestroyDynEntCommand()).
    //
    ServerCommands::OnCommand(DESTROY_DYNENT_COMMAND, CG_DestroyDynEntCommand);
  }
}
