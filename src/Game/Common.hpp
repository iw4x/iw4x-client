#pragma once

namespace Game
{
  typedef void(*Com_ServerPacketEvent_t)();
  extern Com_ServerPacketEvent_t Com_ServerPacketEvent;

  typedef void(*Com_AddStartupCommands_t)();
  extern Com_AddStartupCommands_t Com_AddStartupCommands;

  typedef void(*Com_EventLoop_t)();
  extern Com_EventLoop_t Com_EventLoop;

  // Hand the client the packets that came in since the last call.
  //
  // Note that x64 inlines this into the end of Com_EventLoop(), so we
  // reimplement it from there.
  //
  void Com_ClientPacketEvent();

  typedef void(*Com_Error_t)(errorParm_t type, const char* message, ...);
  extern Com_Error_t Com_Error;

  typedef void(*Com_Shutdown_t)(const char* finalmsg);
  extern Com_Shutdown_t Com_Shutdown;

  // The message of the error that Com_Error() is handling.
  //
  extern char* com_errorMessage;

  // The print functions.
  //
  // Note that x64 has no console print path at all, so these are our own
  // versions of the x86 functions (see Com_PrintMessage() for details).
  //
  void Com_Printf(int channel, const char* fmt, ...);
  void Com_DPrintf(int channel, const char* fmt, ...);
  void Com_PrintError(int channel, const char* fmt, ...);
  void Com_PrintWarning(int channel, const char* fmt, ...);
  void Com_PrintMessage(int channel, const char* msg, int error);

  // A function that, if set, sees each message before Com_PrintMessage()
  // prints it. Returning true takes the message, and Com_PrintMessage() then
  // prints nothing.
  //
  extern bool (*Com_PrintMessageInterceptor)(int channel, const char* msg);

  // Open logFileName for Com_PrintMessage().
  //
  void Com_OpenLogFile();

  // A function that, if set, opens the log file in place of
  // Com_OpenLogFile().
  //
  extern void (*Com_OpenLogFileOverride)();

  extern const char* logFileName;
  extern fileHandle_t logfile;
  extern bool opening_qconsole;
  extern bool com_consoleLogOpenFailed;
  extern int com_errorPrintsCount;

  typedef int(*Com_sprintf_t)(char* dest, int size, const char* fmt, ...);
  extern Com_sprintf_t Com_sprintf;

  // Format into one of two rotating per-thread buffers. Two isn't many, so
  // it's probably best not to hold on to the result for long.
  //
  typedef const char*(*va_t)(const char* fmt, ...);
  extern va_t va;

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

  // Load the info string fileName, which must start with ident, from the
  // fastfiles into loadBuffer of 0x2000 bytes, and return what follows
  // ident.
  //
  // Note that x64 inlines this into Com_LoadInfoString(), which then also
  // validates the info string.
  //
  extern const char* Com_LoadInfoString_FastFile(const char* fileName, const char* fileDesc, const char* ident, char* loadBuffer);
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
