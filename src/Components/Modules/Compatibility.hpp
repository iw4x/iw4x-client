#pragma once

namespace Components
{
  // Note that x64 numbers the server message ops and names the server
  // commands its own way, and that a few of the commands and the hud
  // elements differ in what they carry. Both games speak the same protocol,
  // so on the wire we keep what x86 sends and translate as the game reads
  // and writes. This way an x64 client joins an x86 server and an x86
  // client joins an x64 one.
  //
  class Compatibility : public Component
  {
  public:
    Compatibility();

    // Write the game's server message op as x86 numbers it.
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

    static bool CG_DestroyDynEntCommand(const Command::Params* params);
  };
}
