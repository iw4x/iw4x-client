#include <STDInclude.hpp>

namespace Game
{
	Com_ServerPacketEvent_t Com_ServerPacketEvent = Com_ServerPacketEvent_t(0x458160);
	Com_ClientPacketEvent_t Com_ClientPacketEvent = Com_ClientPacketEvent_t(0x49F0B0);
	Com_AddStartupCommands_t Com_AddStartupCommands = Com_AddStartupCommands_t(0x60C3D0);
	Com_EventLoop_t Com_EventLoop = Com_EventLoop_t(0x43D140);

	Com_Error_t Com_Error = Com_Error_t(0x4B22D0);
	Com_Printf_t Com_Printf = Com_Printf_t(0x402500);
	Com_DPrintf_t Com_DPrintf = Com_DPrintf_t(0x413490);
	Com_PrintError_t Com_PrintError = Com_PrintError_t(0x4F8C70);
	Com_PrintWarning_t Com_PrintWarning = Com_PrintWarning_t(0x4E0200);
	Com_PrintMessage_t Com_PrintMessage = Com_PrintMessage_t(0x4AA830);
	Com_sprintf_t Com_sprintf = Com_sprintf_t(0x413DE0);
	Com_EndParseSession_t Com_EndParseSession = Com_EndParseSession_t(0x4B80B0);
	Com_BeginParseSession_t Com_BeginParseSession = Com_BeginParseSession_t(0x4AAB80);
	Com_ParseOnLine_t Com_ParseOnLine = Com_ParseOnLine_t(0x4C0350);
	Com_SkipRestOfLine_t Com_SkipRestOfLine = Com_SkipRestOfLine_t(0x4B8300);
	Com_SetSpaceDelimited_t Com_SetSpaceDelimited = Com_SetSpaceDelimited_t(0x4FC710);
	Com_Parse_t Com_Parse = Com_Parse_t(0x474D60);
	Com_MatchToken_t Com_MatchToken = Com_MatchToken_t(0x447130);
	Com_SetSlowMotion_t Com_SetSlowMotion = Com_SetSlowMotion_t(0x446E20);
	Com_Quitf_t Com_Quit_f = Com_Quitf_t(0x4D4000);
	Com_OpenLogFile_t Com_OpenLogFile = Com_OpenLogFile_t(0x60A8D0);
	Com_UpdateSlowMotion_t Com_UpdateSlowMotion = Com_UpdateSlowMotion_t(0x60B2D0);
	Com_Compress_t Com_Compress = Com_Compress_t(0x498220);

	int* com_frameTime = reinterpret_cast<int*>(0x1AD8F3C);

	int* com_fixedConsolePosition = reinterpret_cast<int*>(0x1AD8EC8);

	int* com_errorPrintsCount = reinterpret_cast<int*>(0x1AD7910);

	int* errorcode = reinterpret_cast<int*>(0x1AD7EB4);

	char* Com_GetParseThreadInfo()
	{
		if (Sys_IsMainThread())
		{
			return reinterpret_cast<char*>(0x6466628);
		}
		if (Sys_IsRenderThread())
		{
			return reinterpret_cast<char*>(0x646AC34);
		}
		if (Sys_IsServerThread())
		{
			return reinterpret_cast<char*>(0x646F240);
		}
		if (Sys_IsDatabaseThread())
		{
			return reinterpret_cast<char*>(0x647384C);
		}

		return nullptr;
	}

	void Com_SetParseNegativeNumbers(int parse)
	{
		char* g_parse = Com_GetParseThreadInfo();

		if (g_parse)
		{
			g_parse[1056 * *(reinterpret_cast<DWORD*>(g_parse) + 4224) + 1032] = parse != 0;
		}
	}

	const char* Com_LoadInfoString_FastFile(const char* fileName, const char* fileDesc, const char* ident, char* loadBuffer)
	{
		static DWORD Com_LoadInfoString_FastFile_t = 0x609B60;

		const char* result{};

		__asm
		{
			pushad

			mov edi, fileName
			mov ebx, loadBuffer
			push ident
			push fileDesc
			call Com_LoadInfoString_FastFile_t
			add esp, 0x8
			mov result, eax

			popad
		}

		return result;
	}
}
