#pragma once

namespace Components
{
  // Let an x64 client play on an x86 server and an x86 client on an x64 one.
  //
  // You would think this should just work. Both are IW4, and as far as the
  // network goes they look the same, down to the delta encoding of the
  // messages. And for the most part it does work.
  // But the x64 executable seems to have been built from a later revision of
  // the game (perhaps the one Steam ships now, though we can't be sure), and
  // somewhere between the two a handful of things got renumbered. Why anyone
  // would renumber the protocol of a shipped game is anyone's guess. What
  // makes it unpleasant for us is that none of these changes the shape of a
  // message: a client that gets one wrong doesn't fail to parse anything, it
  // parses happily and does the wrong thing, or it reads a field a few bits
  // off and everything after that turns into garbage.
  //
  // The first thing a client runs into are the server message ops, the byte
  // in front of each part of a server message. x86 has an svc_nop that x64
  // dropped, and x64 moved svc_snapshot from the end of the list to the
  // front, so not a single op has the same number in both. Then there are
  // the server commands, the text commands that the server queues for a
  // client reliably. They are named by their first letter, and x64 renamed
  // most of them. One of them, the scores, also carries an extra number per
  // player on x64. Going the other way, two client commands changed: the
  // notification that a button command ran, which x64 sends by index rather
  // than by name, and the request for the scores, which x64 shortened.
  // Finally, the hud element types are numbered without two of x86's, and
  // the default user command that the first one of a packet is a delta from
  // is built with the sprint button held on x86 but not on x64. That's what
  // we found, anyway. There may well be more that we haven't run into yet,
  // so if something behaves oddly only between the two builds, this
  // component is probably a good place to start looking.
  //
  // Two more things aren't protocol differences as such but bite all the
  // same. A pure x86 server lists iwds that an x64 client never has, and x64
  // compiled out one of the server commands IW4x sends (the one that
  // destroys a dynamic entity), which leaves us to handle it ourselves.
  //
  // So what do we do about all this? We could teach each side to speak both
  // dialects and somehow negotiate, but then every server and client out
  // there would have to know about it. Instead we always speak x86 on the
  // wire, whichever side we are on, and translate wherever the game reads or
  // writes, so that the game itself only ever sees its own x64 numbering.
  // x86 seemed the natural choice for the wire since that's what everything
  // out there already speaks, demos included, and this way the x64 build
  // fits in without anyone else having to change anything.
  //
  // The translation itself turned out to be the easy part. What made it
  // tricky is that both sides scramble their messages with the reliable
  // commands, and so the game needs each command exactly as it went over the
  // wire long after we have translated it. See the comment above Com_HashKey
  // in the source file for how we deal with that.
  //
  class Compatibility : public Component
  {
  public:
    Compatibility();

    // Write the game's server message op the way x86 numbers it. Theatre also
    // uses this when it writes the ops of a demo itself.
    //
    static void MSG_WriteServerOp(Game::msg_t* msg, int op);

  private:
    static int MSG_ReadServerOp(Game::msg_t* msg);

    static void MSG_WriteServerCommand(Game::msg_t* msg, const char* command);
    static int SV_HashServerCommand(const char* command, int length);
    static void SV_Netchan_Decode_Hk(char* client, unsigned char* data, int length);
    static void (*SV_Netchan_Decode_Original)(char* client, unsigned char* data, int length);

    static void CL_StoreServerCommand(char* dest, const char* command, int size);
    static void* CL_ClearConnection(void* connection, int value, std::size_t size);
    static int CL_HashServerCommand(const char* command, int length);
    static void CL_Netchan_Transmit_Hk(void* chan, unsigned char* data, int length);
    static void (*CL_Netchan_Transmit_Original)(void* chan, unsigned char* data, int length);
    static void CL_TokenizeServerCommand(const char* command);

    static int CL_FormatNotifyCommand(char* buffer, int size, const char* format, int index);
    static void CL_AddScoresRequest(int localClientNum, const char* command);
    static char* SV_ReadClientCommand(Game::msg_t* msg, char* buffer, int size);
    static int SV_StoreClientCommand(char* buffer, int size, const char* format, const char* command);

    static void MSG_ReadDeltaHudElems_Hk(Game::msg_t* msg, int time, const Game::hudelem_s* from, Game::hudelem_s* to, int count);
    static void (*MSG_ReadDeltaHudElems_Original)(Game::msg_t* msg, int time, const Game::hudelem_s* from, Game::hudelem_s* to, int count);
    static void MSG_WriteDeltaHudElems_Hk(void* snapInfo, Game::msg_t* msg, int time, const Game::hudelem_s* from, const Game::hudelem_s* to, int count);
    static void (*MSG_WriteDeltaHudElems_Original)(void* snapInfo, Game::msg_t* msg, int time, const Game::hudelem_s* from, const Game::hudelem_s* to, int count);

    static void MSG_SetDefaultUserCmd_Hk(Game::playerState_s* ps, Game::usercmd_s* cmd);
    static void (*MSG_SetDefaultUserCmd_Original)(Game::playerState_s* ps, Game::usercmd_s* cmd);

    static bool FS_PureServerSetLoadedIwds(const char* sums, const char* names);

    static bool CG_DestroyDynEntCommand(const Command::Params* params);
  };
}
