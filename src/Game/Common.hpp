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

	typedef void(*Com_PrintError_t)(int channel, const char* fmt, ...);
	extern Com_PrintError_t Com_PrintError;

	typedef void(*Com_PrintWarning_t)(int channel, const char* fmt, ...);
	extern Com_PrintWarning_t  Com_PrintWarning;

	typedef void(*Com_PrintMessage_t)(int channel, const char* msg, int error);
	extern Com_PrintMessage_t Com_PrintMessage;

	typedef void(*Com_EndParseSession_t)();
	extern Com_EndParseSession_t Com_EndParseSession;

	typedef void(*Com_BeginParseSession_t)(const char* filename);
	extern Com_BeginParseSession_t Com_BeginParseSession;

	typedef char* (*Com_ParseOnLine_t)(const char** data_p);
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

	extern int* com_frameTime;

	extern int* com_fixedConsolePosition;

	extern int* com_errorPrintsCount;

	extern char* Com_GetParseThreadInfo();
	extern void Com_SetParseNegativeNumbers(int parse);
}
