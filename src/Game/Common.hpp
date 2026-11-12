#pragma once

namespace Game
{
  typedef void(*Com_ServerPacketEvent_t)();
  extern Com_ServerPacketEvent_t Com_ServerPacketEvent;

  typedef void(*Com_AddStartupCommands_t)();
  extern Com_AddStartupCommands_t Com_AddStartupCommands;

  typedef void(*Com_EventLoop_t)();
  extern Com_EventLoop_t Com_EventLoop;

  typedef void(*Com_Error_t)(errorParm_t type, const char* message, ...);
  extern Com_Error_t Com_Error;

  // Message of the error Com_Error() is handling.
  //
  extern char* com_errorMessage;

  // Print functions.
  //
  // Note that x64 has no console print path, so these are our own versions
  // of the x86 functions. See Com_PrintMessage() for details.
  //
  void Com_Printf(int channel, const char* fmt, ...);
  void Com_DPrintf(int channel, const char* fmt, ...);
  void Com_PrintError(int channel, const char* fmt, ...);
  void Com_PrintWarning(int channel, const char* fmt, ...);
  void Com_PrintMessage(int channel, const char* msg, int error);

  // Open logFileName for Com_PrintMessage().
  //
  void Com_OpenLogFile();

  extern const char* logFileName;
  extern fileHandle_t logfile;
  extern bool opening_qconsole;
  extern bool com_consoleLogOpenFailed;
  extern int com_errorPrintsCount;

  typedef int(*Com_sprintf_t)(char* dest, int size, const char* fmt, ...);
  extern Com_sprintf_t Com_sprintf;

  typedef void(*Com_EndParseSession_t)();
  extern Com_EndParseSession_t Com_EndParseSession;

  typedef void(*Com_BeginParseSession_t)(const char* filename);
  extern Com_BeginParseSession_t Com_BeginParseSession;

  typedef char*(*Com_ParseOnLine_t)(const char** data_p);
  extern Com_ParseOnLine_t Com_ParseOnLine;

  typedef void(*Com_SkipRestOfLine_t)(const char** data);
  extern Com_SkipRestOfLine_t Com_SkipRestOfLine;

  typedef void(*Com_SetSpaceDelimited_t)(int);
  extern Com_SetSpaceDelimited_t Com_SetSpaceDelimited;

  typedef char* (*Com_Parse_t)(const char** data_p);
  extern Com_Parse_t Com_Parse;

  typedef void(*Com_SetSlowMotion_t)(float start, float end, int duration);
  extern Com_SetSlowMotion_t Com_SetSlowMotion;

  typedef void(*Com_Quitf_t)();
  extern Com_Quitf_t Com_Quit_f;

  typedef float(*Com_UpdateSlowMotion_t)(float frameTime);
  extern Com_UpdateSlowMotion_t Com_UpdateSlowMotion;

  typedef char* (*Com_LoadInfoString_t)(const char* fileName, const char* fileDesc, const char* ident, char* loadBuffer);
  extern Com_LoadInfoString_t Com_LoadInfoString;

  typedef void (*Com_SyncThreads_t)();
  extern Com_SyncThreads_t Com_SyncThreads;

  extern int* com_frameTime;

  extern int* com_fixedConsolePosition;

  extern int* errorcode;

  extern char* Com_GetParseThreadInfo();
  extern void Com_SetParseNegativeNumbers(int parse);

  extern char* Com_GetCommandLine();
}

#define Com_InitThreadData()                                                             \
{                                                                                        \
  static Game::ProfileStack profile_stack{};                                           \
  static Game::va_info_t va_info{};                                                    \
  static jmp_buf g_com_error{};                                                        \
  static Game::TraceThreadInfo g_trace_thread_info{};                                  \
  static void* g_thread_values[Game::THREAD_VALUE_COUNT]{};                            \
  *(Game::Sys::GetTls<void*>(Game::Sys::TLS_OFFSET::THREAD_VALUES)) = g_thread_values; \
  Game::Sys_SetValue(Game::THREAD_VALUE_PROF_STACK, &profile_stack);                   \
  Game::Sys_SetValue(Game::THREAD_VALUE_VA, &va_info);                                 \
  Game::Sys_SetValue(Game::THREAD_VALUE_COM_ERROR, &g_com_error);                      \
  Game::Sys_SetValue(Game::THREAD_VALUE_TRACE, &g_trace_thread_info);                  \
}
