#pragma once

namespace Components::GSC
{
	class ScriptError : public Component
	{
	public:
		ScriptError();

		static int Scr_IsInOpcodeMemory(const char* pos);
		static int Scr_GetLineNum(unsigned int bufferIndex, unsigned int sourcePos);

		static void RuntimeError(const char* codePos, unsigned int index, const char* msg, const char* dialogMessage);

	private:
		// Replacement for variables not present in currently available structs
		static int Developer_;

		static Game::scrParserGlob_t ScrParserGlob;
		static Game::scrParserPub_t ScrParserPub;

		static void AddOpcodePos(unsigned int sourcePos, int type);
		static void RemoveOpcodePos();
		static void AddThreadStartOpcodePos(unsigned int sourcePos);

		static unsigned int Scr_GetPrevSourcePos(const char* codePos, unsigned int index);
		static Game::OpcodeLookup* Scr_GetPrevSourcePosOpcodeLookup(const char* codePos);
		static void Scr_CopyFormattedLine(char* line, const char* rawLine);
		static int Scr_GetLineNumInternal(const char* buf, unsigned int sourcePos, const char** startLine, int* col, Game::SourceBufferInfo* binfo);
		static unsigned int Scr_GetSourceBuffer(const char* codePos);
		static void Scr_PrintPrevCodePos(int channel, const char* codePos, unsigned int index);
		static int Scr_GetLineInfo(const char* buf, unsigned int sourcePos, int* col, char* line, Game::SourceBufferInfo* binfo);
		static void Scr_PrintSourcePos(int channel, const char* filename, const char* buf, unsigned int sourcePos);

		static void RuntimeErrorInternal(int channel, const char* codePos, unsigned int index, const char* msg);

		static void CompileError(unsigned int sourcePos, const char* msg, ...);
		static void CompileError2(const char* codePos, const char* msg, ...);

		static void Scr_GetTextSourcePos(const char* buf, const char* codePos, char* line);

		static void Scr_InitOpcodeLookup();
		static void Scr_ShutdownOpcodeLookup();

		static void EmitThreadInternal_Stub();

		static Game::SourceBufferInfo* Scr_GetNewSourceBuffer();
		static void Scr_AddSourceBufferInternal(const char* extFilename, const char* codePos, char* sourceBuf, int len, bool doEolFixup, bool archive);
		static char* Scr_ReadFile_FastFile(const char* filename, const char* extFilename, const char* codePos, bool archive);
		static char* Scr_ReadFile_LoadObj(const char* filename, const char* extFilename, const char* codePos, bool archive);
		static char* Scr_ReadFile(const char* filename, const char* extFilename, const char* codePos, bool archive);
		static char* Scr_AddSourceBuffer(const char* filename, const char* extFilename, const char* codePos, bool archive);
		static unsigned int Scr_LoadScriptInternal_Hk(const char* filename, Game::PrecacheEntry* entries, int entriesCount);

		static void Scr_Settings_Hk(int developer, int developer_script, int abort_on_error);

		static void MT_Reset_Stub();
		static void SL_ShutdownSystem_Stub(unsigned int user);
	};
}
