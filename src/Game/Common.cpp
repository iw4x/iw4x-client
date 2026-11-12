#include "Common.hpp"

namespace Game
{
  Com_ServerPacketEvent_t Com_ServerPacketEvent = Com_ServerPacketEvent_t(0x1401F5B70);
  Com_AddStartupCommands_t Com_AddStartupCommands = Com_AddStartupCommands_t(0x1401F3200);
  Com_EventLoop_t Com_EventLoop = Com_EventLoop_t(0x1401F3BE0);

  Com_Error_t Com_Error = Com_Error_t(0x1401F3540);
  Com_sprintf_t Com_sprintf = Com_sprintf_t(0x14028A3D0);
  Com_EndParseSession_t Com_EndParseSession = Com_EndParseSession_t(0x140289260);
  Com_BeginParseSession_t Com_BeginParseSession = Com_BeginParseSession_t(0x140289170);
  Com_ParseOnLine_t Com_ParseOnLine = Com_ParseOnLine_t(0x140289BD0);
  Com_SkipRestOfLine_t Com_SkipRestOfLine = Com_SkipRestOfLine_t(0x140289FD0);
  Com_SetSpaceDelimited_t Com_SetSpaceDelimited = Com_SetSpaceDelimited_t(0x140289F50);
  Com_Parse_t Com_Parse = Com_Parse_t(0x1402894A0);
  Com_SetSlowMotion_t Com_SetSlowMotion = Com_SetSlowMotion_t(0x1401F6530);
  Com_Quitf_t Com_Quit_f = Com_Quitf_t(0x1401F5800);
  Com_UpdateSlowMotion_t Com_UpdateSlowMotion = Com_UpdateSlowMotion_t(0x1401F67B0);
  Com_LoadInfoString_t Com_LoadInfoString = Com_LoadInfoString_t(0x1401F6B50);
  Com_SyncThreads_t Com_SyncThreads = Com_SyncThreads_t(0x1401F6780);

  int* com_frameTime = reinterpret_cast<int*>(0x141BD7958);

  int* com_fixedConsolePosition = reinterpret_cast<int*>(0x141BD79A0);

  int* errorcode = reinterpret_cast<int*>(0x141BD79D0);

  char* Com_GetParseThreadInfo()
  {
    if (Sys_IsMainThread())
    {
      return reinterpret_cast<char*>(0x146705E20);
    }
    if (Sys_IsRenderThread())
    {
      return reinterpret_cast<char*>(0x14670A5B8);
    }
    if (Sys_IsServerThread())
    {
      return reinterpret_cast<char*>(0x14670ED50);
    }
    if (Sys_IsDatabaseThread())
    {
      return reinterpret_cast<char*>(0x1467134E8);
    }

    return nullptr;
  }

  void Com_SetParseNegativeNumbers(int parse)
  {
    char* g_parse = Com_GetParseThreadInfo();

    if (g_parse)
    {
      // ParseThreadInfo holds 16 parseInfo_t of 0x438 bytes followed by the
      // index of the current one. Note that negativeNumbers is at 0x408.
      //
      g_parse[0x438 * *reinterpret_cast<int*>(g_parse + 0x4380) + 0x408] = parse != 0;
    }
  }

  char* Com_GetCommandLine()
  {
    return reinterpret_cast<char*>(0x141BD7450);
  }
}
