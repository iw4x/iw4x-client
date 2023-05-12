#pragma once

namespace Game
{
	typedef void(*Com_ServerPacketEvent_t)();
	extern Com_ServerPacketEvent_t Com_ServerPacketEvent;

	typedef void(*Com_ClientPacketEvent_t)();
	extern Com_ClientPacketEvent_t Com_ClientPacketEvent;

	typedef void(*Com_AddStartupCommands_t)();
	extern Com_AddStartupCommands_t Com_AddStartupCommands;

	typedef void(*Com_EventLoop_t)();
	extern Com_EventLoop_t Com_EventLoop;

	typedef void(*Com_Error_t)(errorParm_t type, const char* message, ...);
	extern Com_Error_t Com_Error;

	typedef void(*Com_Printf_t)(int channel, const char* fmt, ...);
	extern Com_Printf_t Com_Printf;

	typedef void(*Com_DPrintf_t)(int channel, const char* fmt, ...);
	extern Com_DPrintf_t Com_DPrintf;

	typedef void(*Com_PrintError_t)(int channel, const char* fmt, ...);
	extern Com_PrintError_t Com_PrintError;

	typedef void(*Com_PrintWarning_t)(int channel, const char* fmt, ...);
	extern Com_PrintWarning_t  Com_PrintWarning;

	typedef void(*Com_PrintMessage_t)(int channel, const char* msg, int error);
	extern Com_PrintMessage_t Com_PrintMessage;

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

	typedef bool (*Com_MatchToken_t)(const char** data_p, const char* token, int size);
	extern Com_MatchToken_t Com_MatchToken;

	typedef void(*Com_SetSlowMotion_t)(float start, float end, int duration);
	extern Com_SetSlowMotion_t Com_SetSlowMotion;

	typedef void(*Com_Quitf_t)();
	extern Com_Quitf_t Com_Quit_f;

	typedef void(*Com_OpenLogFile_t)();
	extern Com_OpenLogFile_t Com_OpenLogFile;

	typedef void(*Com_UpdateSlowMotion_t)(int msec);
	extern Com_UpdateSlowMotion_t Com_UpdateSlowMotion;

	typedef int(*Com_Compress_t)(char* data_p);
	extern Com_Compress_t Com_Compress;

	extern int* com_frameTime;

	extern int* com_fixedConsolePosition;

	extern int* com_errorPrintsCount;

	extern int* errorcode;

	extern char* Com_GetParseThreadInfo();
	extern void Com_SetParseNegativeNumbers(int parse);

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
