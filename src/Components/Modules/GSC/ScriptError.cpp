#include "Game/Engine/Hunk.hpp"

#include "ScriptError.hpp"

#define SCRIPT_ERROR_PATCH

namespace Components::GSC
{
  using namespace Utils::String;

  namespace
  {
    void Print(const int channel, const char* text)
    {
      Logger::Print(static_cast<Game::conChannel_t>(channel), "{}", text);
    }

    void PrintError(const int channel, const char* text)
    {
      Logger::PrintError(static_cast<Game::conChannel_t>(channel), "{}", text);
    }
  }

  int ScriptError::Developer_;

  std::vector<ScriptError::ThreadStart> ScriptError::ThreadStarts;

  Game::scrParserGlob_t ScriptError::ScrParserGlob;
  Game::scrParserPub_t ScriptError::ScrParserPub;

  int ScriptError::Scr_IsInOpcodeMemory(const char* pos)
  {
    assert(Game::scrVarPub->programBuffer);
    assert(pos);

    return pos - Game::scrVarPub->programBuffer < static_cast<int>(Game::scrCompilePub->programLen);
  }

  void ScriptError::AddOpcodePos(unsigned int sourcePos, int type)
  {
    Game::OpcodeLookup* opcodeLookup;
    Game::SourceLookup* sourcePosLookup;

    if (!Developer_)
    {
      return;
    }

    if (Game::scrCompilePub->developer_statement == 2)
    {
      assert(!Game::scrVarPub->developer_script);
      return;
    }

    if (Game::scrCompilePub->developer_statement == 3)
    {
      return;
    }

    if (!Game::scrCompilePub->allowedBreakpoint)
    {
      type &= ~Game::SOURCE_TYPE_BREAKPOINT;
    }

    assert(ScrParserGlob.opcodeLookup);
    assert(ScrParserGlob.sourcePosLookup);
    assert(Game::scrCompilePub->opcodePos);

    auto size = sizeof(Game::OpcodeLookup) * (ScrParserGlob.opcodeLookupLen + 1);
    if (size > ScrParserGlob.opcodeLookupMaxSize)
    {
      if (ScrParserGlob.opcodeLookupMaxSize >= Game::MAX_OPCODE_LOOKUP_SIZE)
      {
        Game::Sys_Error("MAX_OPCODE_LOOKUP_SIZE exceeded");
      }

      Game::Z_VirtualCommit((char*)ScrParserGlob.opcodeLookup + ScrParserGlob.opcodeLookupMaxSize, 0x20000);
      ScrParserGlob.opcodeLookupMaxSize += 0x20000;
      assert(size <= ScrParserGlob.opcodeLookupMaxSize);
    }

    size = sizeof(Game::SourceLookup) * (ScrParserGlob.sourcePosLookupLen + 1);
    if (size > ScrParserGlob.sourcePosLookupMaxSize)
    {
      if (ScrParserGlob.sourcePosLookupMaxSize >= Game::MAX_SOURCEPOS_LOOKUP_SIZE)
      {
        Game::Sys_Error("MAX_SOURCEPOS_LOOKUP_SIZE exceeded");
      }

      Game::Z_VirtualCommit((char*)ScrParserGlob.sourcePosLookup + ScrParserGlob.sourcePosLookupMaxSize, 0x20000);
      ScrParserGlob.sourcePosLookupMaxSize += 0x20000;
      assert(size <= ScrParserGlob.sourcePosLookupMaxSize);
    }

    if (ScrParserGlob.currentCodePos == Game::scrCompilePub->opcodePos)
    {
      assert(ScrParserGlob.currentSourcePosCount);
      --ScrParserGlob.opcodeLookupLen;
      opcodeLookup = &ScrParserGlob.opcodeLookup[ScrParserGlob.opcodeLookupLen];
      assert(opcodeLookup->sourcePosIndex + ScrParserGlob.currentSourcePosCount == ScrParserGlob.sourcePosLookupLen);
      assert(opcodeLookup->codePos == (char*)ScrParserGlob.currentCodePos);
    }
    else
    {
      ScrParserGlob.currentSourcePosCount = 0;
      ScrParserGlob.currentCodePos = Game::scrCompilePub->opcodePos;
      opcodeLookup = &ScrParserGlob.opcodeLookup[ScrParserGlob.opcodeLookupLen];
      opcodeLookup->sourcePosIndex = ScrParserGlob.sourcePosLookupLen;
      opcodeLookup->codePos = ScrParserGlob.currentCodePos;
    }

    auto sourcePosLookupIndex = ScrParserGlob.currentSourcePosCount + opcodeLookup->sourcePosIndex;
    sourcePosLookup = &ScrParserGlob.sourcePosLookup[sourcePosLookupIndex];
    sourcePosLookup->sourcePos = sourcePos;

    if (sourcePos == static_cast<unsigned int>(-1))
    {
      assert(ScrParserGlob.delayedSourceIndex == -1);
      assert(type & Game::SOURCE_TYPE_BREAKPOINT);
      ScrParserGlob.delayedSourceIndex = static_cast<int>(sourcePosLookupIndex);
    }
    else if (sourcePos == static_cast<unsigned int>(-2))
    {
      ScrParserGlob.threadStartSourceIndex = static_cast<int>(sourcePosLookupIndex);
    }
    else if (ScrParserGlob.delayedSourceIndex >= 0 && (type & Game::SOURCE_TYPE_BREAKPOINT))
    {
      ScrParserGlob.sourcePosLookup[ScrParserGlob.delayedSourceIndex].sourcePos = sourcePos;
      ScrParserGlob.delayedSourceIndex = -1;
    }

    sourcePosLookup->type |= type;
    ++ScrParserGlob.currentSourcePosCount;
    opcodeLookup->sourcePosCount = static_cast<unsigned short>(ScrParserGlob.currentSourcePosCount);
    ++ScrParserGlob.opcodeLookupLen;
    ++ScrParserGlob.sourcePosLookupLen;
  }

  void ScriptError::RemoveOpcodePos()
  {
    if (!Developer_)
    {
      return;
    }

    if (Game::scrCompilePub->developer_statement == 2)
    {
      assert(!Game::scrVarPub->developer_script);
      return;
    }

    assert(ScrParserGlob.opcodeLookup);
    assert(ScrParserGlob.sourcePosLookup);
    assert(Game::scrCompilePub->opcodePos);
    assert(ScrParserGlob.sourcePosLookupLen);

    --ScrParserGlob.sourcePosLookupLen;
    assert(ScrParserGlob.opcodeLookupLen);

    --ScrParserGlob.opcodeLookupLen;
    assert(ScrParserGlob.currentSourcePosCount);
    --ScrParserGlob.currentSourcePosCount;

    auto* opcodeLookup = &ScrParserGlob.opcodeLookup[ScrParserGlob.opcodeLookupLen];

    assert(ScrParserGlob.currentCodePos == Game::scrCompilePub->opcodePos);
    assert(opcodeLookup->sourcePosIndex + ScrParserGlob.currentSourcePosCount == ScrParserGlob.sourcePosLookupLen);
    assert(opcodeLookup->codePos == (char*)ScrParserGlob.currentCodePos);

    if (!ScrParserGlob.currentSourcePosCount)
    {
      ScrParserGlob.currentCodePos = nullptr;
    }

    opcodeLookup->sourcePosCount = static_cast<unsigned short>(ScrParserGlob.currentSourcePosCount);
  }

  void ScriptError::AddThreadStartOpcodePos(unsigned int sourcePos)
  {
    if (!Developer_)
    {
      return;
    }

    if (Game::scrCompilePub->developer_statement == 2)
    {
      assert(!Game::scrVarPub->developer_script);
    }
    else
    {
      assert(ScrParserGlob.threadStartSourceIndex >= 0);
      auto* sourcePosLookup = &ScrParserGlob.sourcePosLookup[ScrParserGlob.threadStartSourceIndex];
      sourcePosLookup->sourcePos = sourcePos;
      assert(!sourcePosLookup->type);
      sourcePosLookup->type = 8;
      ScrParserGlob.threadStartSourceIndex = -1;
    }
  }

  int ScriptError::Scr_GetLineNum(unsigned int bufferIndex, unsigned int sourcePos)
  {
    const char* startLine;
    int col;

    assert(Developer_);
    return Scr_GetLineNumInternal(ScrParserPub.sourceBufferLookup[bufferIndex].sourceBuf, sourcePos, &startLine, &col, nullptr);
  }

  unsigned int ScriptError::Scr_GetPrevSourcePos(const char* codePos, unsigned int index)
  {
    const auto prevIndex = Scr_GetPrevSourcePosOpcodeLookup(codePos)->sourcePosIndex;
    const auto sPos = ScrParserGlob.sourcePosLookup[index + prevIndex].sourcePos;

    // I'm not sure why this is necessary - when given a valid codePos, sometimes
    // the sourcePos ends up being a negative number (got a case where it was -2)
    // which will output a giantic unsigned number and crash
    // So we make sure it's not gonna happen here by clamping the number
    const auto uPos = static_cast<unsigned int>(std::max(0, static_cast<int>(sPos)));

    return uPos;
  }

  Game::OpcodeLookup* ScriptError::Scr_GetPrevSourcePosOpcodeLookup(const char* codePos)
  {
    assert(Scr_IsInOpcodeMemory(codePos));
    assert(ScrParserGlob.opcodeLookup);

    unsigned int low = 0;
    unsigned int high = ScrParserGlob.opcodeLookupLen - 1;
    while (low <= high)
    {
      unsigned int middle = (high + low) >> 1;
      if (codePos < ScrParserGlob.opcodeLookup[middle].codePos)
      {
        high = middle - 1;
      }
      else
      {
        low = middle + 1;
        if (low == ScrParserGlob.opcodeLookupLen || codePos < ScrParserGlob.opcodeLookup[low].codePos)
        {
          return &ScrParserGlob.opcodeLookup[middle];
        }
      }
    }

    AssertUnreachable;
    return nullptr;
  }

  void ScriptError::Scr_CopyFormattedLine(char* line, const char* rawLine)
  {
    auto len = std::strlen(rawLine);
    if (len >= 1024)
    {
      len = 1024 - 1;
    }

    for (std::size_t i = 0; i < len; ++i)
    {
      if (rawLine[i] == '\t')
      {
        line[i] = ' ';
      }
      else
      {
        line[i] = rawLine[i];
      }
    }

    if (line[len - 1] == '\r')
    {
      line[len - 1] = '\0';
    }

    line[len] = '\0';
  }

  int ScriptError::Scr_GetLineNumInternal(const char* buf, unsigned int sourcePos, const char** startLine, int* col, [[maybe_unused]] Game::SourceBufferInfo* binfo)
  {
    assert(buf);

    *startLine = buf;
    unsigned int lineNum = 0;
    while (sourcePos)
    {
      if (!*buf)
      {
        *startLine = buf + 1;
        ++lineNum;
      }
      ++buf;
      --sourcePos;
    }

    *col = static_cast<int>(buf - *startLine);
    return static_cast<int>(lineNum);
  }

  unsigned int ScriptError::Scr_GetSourceBuffer(const char* codePos)
  {
    unsigned int bufferIndex;

    assert(Scr_IsInOpcodeMemory(codePos));
    assert(ScrParserPub.sourceBufferLookupLen > 0);

    for (bufferIndex = ScrParserPub.sourceBufferLookupLen - 1; bufferIndex; --bufferIndex)
    {
      if (!ScrParserPub.sourceBufferLookup[bufferIndex].codePos)
      {
        continue;
      }

      if (ScrParserPub.sourceBufferLookup[bufferIndex].codePos > codePos)
      {
        continue;
      }

      break;
    }

    return bufferIndex;
  }

  void ScriptError::Scr_PrintPrevCodePos(int channel, const char* codePos, unsigned int index)
  {
    if (!codePos)
    {
      Print(channel, "<frozen thread>\n");
      return;
    }

    if (codePos == Game::g_EndPos)
    {
      Print(channel, "<removed thread>\n");
      return;
    }

    if (!Developer_)
    {
      if (Scr_IsInOpcodeMemory(codePos - 1))
      {
        Print(channel, VA("@ %d\n", static_cast<int>(codePos - Game::scrVarPub->programBuffer)));
        return;
      }
    }
    else
    {
      if (Game::scrVarPub->programBuffer && Scr_IsInOpcodeMemory(codePos))
      {
        auto bufferIndex = Scr_GetSourceBuffer(codePos - 1);
        const auto prevSourcePos = Scr_GetPrevSourcePos(codePos - 1, index);

        Scr_PrintSourcePos(channel, ScrParserPub.sourceBufferLookup[bufferIndex].buf, ScrParserPub.sourceBufferLookup[bufferIndex].sourceBuf, prevSourcePos);
        return;
      }
    }

    Print(channel, VA("%s\n\n", codePos));
  }

  int ScriptError::Scr_GetLineInfo(const char* buf, unsigned int sourcePos, int* col, char* line, Game::SourceBufferInfo* binfo)
  {
    const char* startLine;
    unsigned int lineNum;

    if (buf)
    {
      lineNum = Scr_GetLineNumInternal(buf, sourcePos, &startLine, col, binfo);
    }
    else
    {
      lineNum = 0;
      startLine = "";
      *col = 0;
    }

    Scr_CopyFormattedLine(line, startLine);
    return static_cast<int>(lineNum);
  }

  void ScriptError::Scr_PrintSourcePos(int channel, const char* filename, const char* buf, unsigned int sourcePos)
  {
    char line[1024];
    int col;

    assert(filename);
    auto lineNum = Scr_GetLineInfo(buf, sourcePos, &col, line, nullptr);

    Print(channel, VA("(file '%s'%s, line %d)\n", filename, ScrParserGlob.saveSourceBufferLookup ? " (savegame)" : "", lineNum + 1));
    Print(channel, VA("%s\n", line));

    for (auto i = 0; i < col; ++i)
    {
      Print(channel, " ");
    }

    Print(channel, "*\n");
  }

  void ScriptError::RuntimeErrorInternal(int channel, const char* codePos, unsigned int index, const char* msg)
  {
    assert(Scr_IsInOpcodeMemory(codePos));

    PrintError(channel, VA("\n******* script runtime error *******\n%s: ", msg));
    Scr_PrintPrevCodePos(channel, codePos, index);

    if (Game::scrVmPub->function_count)
    {
      for (auto i = Game::scrVmPub->function_count - 1; i >= 1; --i)
      {
        PrintError(channel, "called from:\n");
        Scr_PrintPrevCodePos(0, Game::scrVmPub->function_frame_start[i].fs.pos, Game::scrVmPub->function_frame_start[i].fs.localId == 0);
      }

      PrintError(channel, "started from:\n");
      Scr_PrintPrevCodePos(0, Game::scrVmPub->function_frame_start[0].fs.pos, 1);
    }

    PrintError(channel, "************************************\n");
  }

  void ScriptError::RuntimeError(const char* codePos, unsigned int index, const char* msg, [[maybe_unused]] const char* dialogMessage)
  {
    // Note that x64 has no debugger or abort_on_error and goes on to end
    // the script itself, so all we do is say where the error is.
    //
    if (!Developer_)
    {
      assert(Scr_IsInOpcodeMemory(codePos));
      if (!(*Game::com_developer)->current.enabled)
      {
        return;
      }
    }

    RuntimeErrorInternal(Game::CON_CHANNEL_PARSERSCRIPT, codePos, index, msg);
  }

  void ScriptError::VM_Error(const char* codePos)
  {
    // Note that x64 doesn't keep the error's message, which the script
    // printed when it raised it.
    //
    RuntimeError(codePos, 0, "script runtime error", nullptr);
  }

  void ScriptError::CompileError(unsigned int sourcePos, const char* msg, ...)
  {
    char line[1024];
    char text[1024];
    int col;
    va_list argptr;

    va_start(argptr, msg);
    vsnprintf_s(text, _TRUNCATE, msg, argptr);
    va_end(argptr);

    // Note that x64 doesn't keep the message of an error in an evaluated
    // expression and gives up with it as a fatal error.
    //
    if (Game::scrVarPub->evaluate)
    {
      Game::Sys_Error(text);
    }
    else
    {
      Game::Scr_ShutdownAllocNode();
      PrintError(Game::CON_CHANNEL_PARSERSCRIPT, "\n");
      PrintError(Game::CON_CHANNEL_PARSERSCRIPT, "******* script compile error *******\n");

      if (!Developer_ || !ScrParserPub.sourceBuf)
      {
        PrintError(Game::CON_CHANNEL_PARSERSCRIPT, VA("%s\n", text));
        line[0] = '\0';

        Print(Game::CON_CHANNEL_PARSERSCRIPT, "************************************\n");
        Game::Com_Error(Game::ERR_SCRIPT_DROP, "\x15" "script compile error\n%s\n%s\n(see console for details)\n", text, line);
      }
      else
      {
        assert(ScrParserPub.sourceBuf);
        PrintError(Game::CON_CHANNEL_PARSERSCRIPT, VA("%s: ", text));

        Scr_PrintSourcePos(Game::CON_CHANNEL_PARSERSCRIPT, ScrParserPub.scriptfilename, ScrParserPub.sourceBuf, sourcePos);
        const auto lineNumber = Scr_GetLineInfo(ScrParserPub.sourceBuf, sourcePos, &col, line, nullptr);
        Game::Com_Error(Game::ERR_SCRIPT_DROP, "\x15" "script compile error\n%s\n%s(%d):\n %s\n(see console for details)\n", text, ScrParserPub.scriptfilename, lineNumber, line);
      }
    }
  }

  void ScriptError::CompileError2(const char* codePos, const char* msg, ...)
  {
    char line[1024];
    char text[1024];
    va_list argptr;

    assert(!Game::scrVarPub->evaluate);
    assert(Scr_IsInOpcodeMemory(codePos));

    PrintError(Game::CON_CHANNEL_PARSERSCRIPT, "\n");
    PrintError(Game::CON_CHANNEL_PARSERSCRIPT, "******* script compile error *******\n");

    va_start(argptr, msg);
    vsnprintf_s(text, _TRUNCATE, msg, argptr);
    va_end(argptr);

    PrintError(Game::CON_CHANNEL_PARSERSCRIPT, VA("%s: ", text));

    Scr_PrintPrevCodePos(Game::CON_CHANNEL_PARSERSCRIPT, codePos, 0);

    Print(Game::CON_CHANNEL_PARSERSCRIPT, "************************************\n");

    Scr_GetTextSourcePos(ScrParserPub.sourceBuf, codePos, line);

    Game::Com_Error(Game::ERR_SCRIPT_DROP, "\x15" "script compile error\n%s\n%s\n(see console for details)\n", text, line);
  }

  void ScriptError::Scr_GetTextSourcePos([[maybe_unused]] const char* buf, const char* codePos, char* line)
  {
    int col;

    if (Developer_ && codePos && codePos != Game::g_EndPos && Game::scrVarPub->programBuffer && Scr_IsInOpcodeMemory(codePos))
    {
      auto bufferIndex = Scr_GetSourceBuffer(codePos - 1);
      Scr_GetLineInfo(ScrParserPub.sourceBufferLookup[bufferIndex].sourceBuf, Scr_GetPrevSourcePos(codePos - 1, 0), &col, line, nullptr);
    }
    else
    {
      *line = '\0';
    }
  }

  void ScriptError::Scr_InitOpcodeLookup()
  {
    assert(!ScrParserGlob.opcodeLookup);
    assert(!ScrParserGlob.sourcePosLookup);
    assert(!ScrParserPub.sourceBufferLookup);

    if (!Developer_)
    {
      return;
    }

    ScrParserGlob.delayedSourceIndex = -1;
    ScrParserGlob.opcodeLookupMaxSize = 0;
    ScrParserGlob.opcodeLookupLen = 0;
    ScrParserGlob.opcodeLookup = static_cast<Game::OpcodeLookup*>(Game::Z_VirtualReserve(Game::MAX_OPCODE_LOOKUP_SIZE));

    ScrParserGlob.sourcePosLookupMaxSize = 0;
    ScrParserGlob.sourcePosLookupLen = 0;
    ScrParserGlob.sourcePosLookup = static_cast<Game::SourceLookup*>(Game::Z_VirtualReserve(Game::MAX_SOURCEPOS_LOOKUP_SIZE));
    ScrParserGlob.currentCodePos = nullptr;
    ScrParserGlob.currentSourcePosCount = 0;
    ScrParserGlob.sourceBufferLookupMaxSize = 0;

    ScrParserPub.sourceBufferLookupLen = 0;
    ScrParserPub.sourceBufferLookup = static_cast<Game::SourceBufferInfo*>(Game::Z_VirtualReserve(Game::MAX_SOURCEBUF_LOOKUP_SIZE));
  }

  void ScriptError::Scr_ShutdownOpcodeLookup()
  {
    if (ScrParserGlob.opcodeLookup)
    {
      Z_VirtualFree(ScrParserGlob.opcodeLookup);
      ScrParserGlob.opcodeLookup = nullptr;
    }

    if (ScrParserGlob.sourcePosLookup)
    {
      Z_VirtualFree(ScrParserGlob.sourcePosLookup);
      ScrParserGlob.sourcePosLookup = nullptr;
    }

    if (ScrParserPub.sourceBufferLookup)
    {
      for (unsigned int i = 0; i < ScrParserPub.sourceBufferLookupLen; ++i)
      {
        Game::Engine::Hunk_FreeDebugMem(ScrParserPub.sourceBufferLookup[i].buf);
      }

      Z_VirtualFree(ScrParserPub.sourceBufferLookup);
      ScrParserPub.sourceBufferLookup = nullptr;
    }

    if (ScrParserGlob.saveSourceBufferLookup)
    {
      for (unsigned int i = 0; i < ScrParserGlob.saveSourceBufferLookupLen; ++i)
      {
        if (ScrParserGlob.saveSourceBufferLookup[i].sourceBuf)
        {
          Game::Engine::Hunk_FreeDebugMem(ScrParserGlob.saveSourceBufferLookup[i].buf);
        }
      }

      Game::Engine::Hunk_FreeDebugMem(ScrParserGlob.saveSourceBufferLookup);
      ScrParserGlob.saveSourceBufferLookup = nullptr;
    }
  }

  void ScriptError::EmitThreadInternal_Hk(void* node, const unsigned int sourcePos, void* a3, void* a4)
  {
    AddThreadStartOpcodePos(sourcePos);
    RecordThreadStart(static_cast<const Game::sval_u*>(node));

    EmitThreadInternal_Original(node, sourcePos, a3, a4);
  }

  // Remember where the thread (function) that we are about to emit starts,
  // along with its name and file. See DescribeCodePos() for details.
  //
  // Note that we copy the name since EmitThreadInternal() releases it right
  // away. Note also that we keep the list sorted by position even though the
  // compiler emits the threads in order, so that a lookup can't go wrong.
  //
  void ScriptError::RecordThreadStart (const Game::sval_u* node)
  {
    ThreadStart t {
      Game::TempMalloc (0),
      ScrParserPub.scriptfilename != nullptr ? ScrParserPub.scriptfilename : "",
      Game::SL_ConvertToString (static_cast<unsigned short> (node[1].stringValue))};

    const auto i (std::ranges::upper_bound (ThreadStarts, t.pos, {}, &ThreadStart::pos));
    ThreadStarts.insert (i, std::move (t));
  }

  std::string ScriptError::DescribeCodePos (const char* pos)
  {
    const auto offset (pos - Game::scrVarPub->programBuffer);

    // Find the last thread that starts at or before the position.
    //
    auto i (std::ranges::upper_bound (ThreadStarts, pos, {}, &ThreadStart::pos));

    if (i == ThreadStarts.begin ())
      return std::format ("{}", offset);

    --i;
    return std::format ("{}::{}+{:#x} ({})", i->file, i->name, pos - i->pos, offset);
  }

  Game::SourceBufferInfo* ScriptError::Scr_GetNewSourceBuffer()
  {
    assert(ScrParserPub.sourceBufferLookup);

    auto size = sizeof(Game::SourceBufferInfo) * (ScrParserPub.sourceBufferLookupLen + 1);
    if (size > ScrParserGlob.sourceBufferLookupMaxSize)
    {
      if (ScrParserGlob.sourceBufferLookupMaxSize >= Game::MAX_SOURCEBUF_LOOKUP_SIZE)
      {
        Game::Sys_Error("MAX_SOURCEBUF_LOOKUP_SIZE exceeded");
      }

      Game::Z_VirtualCommit((char*)ScrParserPub.sourceBufferLookup + ScrParserGlob.sourceBufferLookupMaxSize, 0x20000);
      ScrParserGlob.sourceBufferLookupMaxSize += 0x20000;
      assert(size <= ScrParserGlob.sourceBufferLookupMaxSize);
    }

    return &ScrParserPub.sourceBufferLookup[ScrParserPub.sourceBufferLookupLen++];
  }

  void ScriptError::Scr_AddSourceBufferInternal(const char* extFilename, const char* codePos, char* sourceBuf, int len, bool doEolFixup, bool archive)
  {
    int i;

    if (!ScrParserPub.sourceBufferLookup)
    {
      ScrParserPub.sourceBuf = nullptr;
      return;
    }

    assert((len >= -1));
    assert((len >= 0) || !sourceBuf);

    const auto strLen = std::strlen(extFilename) + 1;
    const auto newLen = strLen + len + 2;
    auto* buf = static_cast<char*>(Game::Engine::Hunk_AllocDebugMem(static_cast<int>(newLen))); // Scr_AddSourceBufferInternal

    strcpy(buf, extFilename);
    auto* sourceBuf2 = sourceBuf ? buf + strLen : nullptr;
    auto* source = sourceBuf;
    auto* dest = sourceBuf2;

    if (doEolFixup)
    {
      for (i = 0; i <= len; ++i)
      {
        const auto c = *source++;
        if (c == '\n' || (c == '\r' && *source != '\n'))
        {
          *dest = 0;
        }
        else
        {
          *dest = c;
        }
        ++dest;
      }
    }
    else
    {
      for (i = 0; i <= len; ++i)
      {
        const auto c = *source;
        ++source;
        *dest = c;
        ++dest;
      }
    }

    auto* bufferInfo = Scr_GetNewSourceBuffer();
    bufferInfo->codePos = codePos;
    bufferInfo->buf = buf;
    bufferInfo->sourceBuf = sourceBuf2;
    bufferInfo->len = len;
    bufferInfo->sortedIndex = -1;
    bufferInfo->archive = archive;

    if (sourceBuf2)
    {
      ScrParserPub.sourceBuf = sourceBuf2;
    }
  }

  char* ScriptError::Scr_ReadFile_FastFile([[maybe_unused]] const char* filename, const char* extFilename, const char* codePos, bool archive)
  {
    auto* rawfile = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_RAWFILE, extFilename).rawfile;
    if (Game::DB_IsXAssetDefault(Game::ASSET_TYPE_RAWFILE, extFilename))
    {
      Scr_AddSourceBufferInternal(extFilename, codePos, nullptr, -1, true, true);
      return nullptr;
    }

    const auto len = Game::DB_GetRawFileLen(rawfile);
    auto* sourceBuf = static_cast<char*>(Game::Hunk_AllocateTempMemoryHigh(len)); // Scr_ReadFile_FastFile
    Game::DB_GetRawBuffer(rawfile, sourceBuf, len);
    Scr_AddSourceBufferInternal(extFilename, codePos, sourceBuf, len, true, archive);
    return sourceBuf;
  }

  char* ScriptError::Scr_ReadFile_LoadObj([[maybe_unused]] const char* filename, const char* extFilename, const char* codePos, bool archive)
  {
    Game::fileHandle_t f;

    auto len = Game::FS_FOpenFileByMode(extFilename, &f, Game::FS_READ);
    if (len < 0)
    {
      Scr_AddSourceBufferInternal(extFilename, codePos, nullptr, -1, true, archive);
      return nullptr;
    }

    *Game::g_loadedImpureScript = true;

    auto* sourceBuf = static_cast<char*>(Game::Hunk_AllocateTempMemoryHigh(len + 1));
    Game::FS_Read(sourceBuf, len, f);
    sourceBuf[len] = '\0';

    Game::FS_FCloseFile(f);
    Scr_AddSourceBufferInternal(extFilename, codePos, sourceBuf, len, true, archive);

    return sourceBuf;
  }

  char* ScriptError::Scr_ReadFile(const char* filename, const char* extFilename, const char* codePos, bool archive)
  {
    Game::fileHandle_t file;

    if (Game::FS_FOpenFileRead(extFilename, &file) < 0)
    {
      return Scr_ReadFile_FastFile(filename, extFilename, codePos, archive);
    }

    Game::FS_FCloseFile(file);
    return Scr_ReadFile_LoadObj(filename, extFilename, codePos, archive);
  }

  char* ScriptError::Scr_AddSourceBuffer(const char* filename, const char* extFilename, const char* codePos, bool archive)
  {
    char* sourceBuf;

    if (archive && ScrParserGlob.saveSourceBufferLookup)
    {
      assert(ScrParserGlob.saveSourceBufferLookupLen > 0);
      --ScrParserGlob.saveSourceBufferLookupLen;

      const auto* saveSourceBuffer = ScrParserGlob.saveSourceBufferLookup + ScrParserGlob.saveSourceBufferLookupLen;
      const auto len = saveSourceBuffer->len;
      assert(len >= -1);

      if (len < 0)
      {
        Scr_AddSourceBufferInternal(extFilename, codePos, nullptr, -1, true, archive);
        sourceBuf = nullptr;
      }
      else
      {
        sourceBuf = static_cast<char*>(Game::Hunk_AllocateTempMemoryHigh(len + 1));

        const char* source = saveSourceBuffer->sourceBuf;
        auto* dest = sourceBuf;
        for (int i = 0; i < len; ++i)
        {
          const auto c = *source++;
          *dest = c ? c : '\n';
          dest++;
        }

        *dest = '\0';
        Scr_AddSourceBufferInternal(extFilename, codePos, sourceBuf, len, false, archive);
      }

      return sourceBuf;
    }

    return Scr_ReadFile(filename, extFilename, codePos, archive);
  }

  unsigned int ScriptError::Scr_LoadScriptInternal_Hk(const char* filename, Game::PrecacheEntry* entries, int entriesCount)
  {
    char extFilename[MAX_QPATH];
    Game::sval_u parseData{};

    assert(Game::scrCompilePub->script_loading);
    assert(std::strlen(filename) < MAX_QPATH);

    const auto name = Game::Scr_CreateCanonicalFilename(filename);

    if (Game::FindVariable(Game::scrCompilePub->loadedscripts, name))
    {
      Game::SL_RemoveRefToString(name);
      const auto filePosPtr = Game::FindVariable(Game::scrCompilePub->scriptsPos, name);
      return filePosPtr ? Game::FindObject(Game::scrCompilePub->scriptsPos, filePosPtr) : 0;
    }

    const auto scriptId = Game::GetNewVariable(Game::scrCompilePub->loadedscripts, name);
    Game::SL_RemoveRefToString(name);

    sprintf_s(extFilename, "%s.gsc", Game::SL_ConvertToString(static_cast<unsigned short>(name)));

    const auto* oldSourceBuf = ScrParserPub.sourceBuf;
    const auto* sourceBuffer = Scr_AddSourceBuffer(Game::SL_ConvertToString(static_cast<unsigned short>(name)), extFilename, Game::TempMalloc(0), true);

    if (!sourceBuffer)
    {
      return 0;
    }

    const auto oldAnimTreeNames = Game::scrAnimPub->animTreeNames;
    Game::scrAnimPub->animTreeNames = 0;
    Game::scrCompilePub->far_function_count = 0;

    const auto* oldFilename = ScrParserPub.scriptfilename;
    ScrParserPub.scriptfilename = extFilename;

    Game::scrCompilePub->in_ptr = "+";
    Game::scrCompilePub->in_ptr_valid = false;
    Game::scrCompilePub->parseBuf = sourceBuffer;

    Game::ScriptParse(&parseData, 0);
    Game::scrCompilePub->parseBuf = nullptr;

    const auto filePosId = Game::GetObject(Game::scrCompilePub->scriptsPos, Game::GetVariable(Game::scrCompilePub->scriptsPos, name));
    const auto fileCountId = Game::GetObject(Game::scrCompilePub->scriptsCount, Game::GetVariable(Game::scrCompilePub->scriptsCount, name));

    Game::ScriptCompile(parseData.node, filePosId, fileCountId, scriptId, entries, entriesCount);

    Game::RemoveVariable(Game::scrCompilePub->scriptsCount, name);

    ScrParserPub.scriptfilename = oldFilename;
    ScrParserPub.sourceBuf = oldSourceBuf;

    Game::scrAnimPub->animTreeNames = oldAnimTreeNames;

    return filePosId;
  }

  unsigned int ScriptError::Scr_LoadScript_Hk(const char* filename)
  {
    // Note that x64 inlines Scr_LoadScriptInternal() here, with room for the
    // entries it has no use for.
    //
    Game::PrecacheEntry entries[1024];
    return Scr_LoadScriptInternal_Hk(filename, entries, 0);
  }

  void ScriptError::MT_Reset_Stub()
  {
    Utils::Hook::Call<void()>(0x14021CFE0)(); // MT_Reset

    // Note that x64 has no Scr_Settings(), so we take developer from the
    // dvar as each load of the scripts starts.
    //
    Developer_ = (*Game::com_developer)->current.enabled;

    // We are about to compile the scripts from scratch, so forget the
    // threads of the previous ones.
    //
    ThreadStarts.clear ();

    Scr_InitOpcodeLookup();
    Game::Engine::Hunk_InitDebugMemory();
  }

  void ScriptError::SL_ShutdownSystem_Stub(unsigned int user)
  {
    Utils::Hook::Call<void(unsigned int)>(0x1402203A0)(user); // SL_ShutdownSystem

    Scr_ShutdownOpcodeLookup();
    Game::Engine::Hunk_ShutdownDebugMemory();
  }

  ScriptError::ScriptError()
  {
#ifdef SCRIPT_ERROR_PATCH
    Utils::Hook(0x14021BC6B, MT_Reset_Stub, HOOK_CALL).install()->quick(); // Scr_BeginLoadScripts
    Utils::Hook(0x14021BFD8, SL_ShutdownSystem_Stub, HOOK_CALL).install()->quick(); // Scr_FreeScripts

    Utils::Hook(0x14021B7AA, Scr_LoadScriptInternal_Hk, HOOK_CALL).install()->quick(); // ScriptCompile
    Utils::Hook(0x14021C140, Scr_LoadScript_Hk, HOOK_JUMP).install()->quick();

    // Read animtrees from disk first as well, as x86 does.
    //
    // Note that x64 reads them from the zones only.
    //
    Utils::Hook(0x140210889, Scr_AddSourceBuffer, HOOK_CALL).install()->quick(); // Scr_LoadAnimTreeAtIndex

    // Note that x64 keeps a single RuntimeError() call, for the stack
    // overflow in VM_Execute(), and leaves the others out. So we also take
    // over where VM_Execute() handles an error, which is where x86 calls it.
    //
    Utils::Hook(0x140229ED1, RuntimeError, HOOK_CALL).install()->quick();

    Utils::Hook(0x14022C47D, ScriptError_VM_ErrorStub, HOOK_JUMP).install()->quick();
    Utils::Hook::Nop(0x14022C47D + 5, 4);

    Utils::Hook(0x14021D190, CompileError, HOOK_JUMP).install()->quick();
    Utils::Hook(0x14021D030, CompileError2, HOOK_JUMP).install()->quick();

    EmitThreadInternal_Original = Utils::Hook::Detour(0x140217BB0, 6, EmitThreadInternal_Hk);

    // The compiler's AddOpcodePos() and RemoveOpcodePos() calls.
    //
    // Note that x64 makes them to an empty function shared with others. The removes are the
    // calls of EmitOpcode() (and its copy for one opcode) before it replaces
    // the previous opcode; the adds pass the source position and type.
    //
    for (const auto address : std::initializer_list<std::uintptr_t>
    {
      0x140210FF4, 0x140211011, 0x1402114F6, 0x1402115FE, 0x140211607,
      0x140211C6D, 0x140211EDF, 0x140211EF1, 0x1402123E4, 0x1402125A8,
      0x140212714, 0x140212863, 0x14021290D, 0x140212953, 0x140212995,
      0x1402129DA, 0x140212A17, 0x140212B4B, 0x140212C0F, 0x140212CCA,
      0x140212D50, 0x140212FDB, 0x140212FF8, 0x140213051, 0x140213111,
      0x140213233, 0x140213353, 0x140213361, 0x1402139A5, 0x140213D4E,
      0x140213F18, 0x140213FE3, 0x1402140B1, 0x140214181, 0x140214251,
      0x14021432F, 0x140214419, 0x14021473A, 0x140214825, 0x14021488F,
      0x140214931, 0x140214B45, 0x140214D0A, 0x140214DD6, 0x140214F85,
      0x1402151EF, 0x14021540C, 0x140215588, 0x140215656, 0x140215805,
      0x1402161AD, 0x1402161D4, 0x1402162B7, 0x1402162C0, 0x1402162C9,
      0x1402163C3, 0x140216429, 0x14021669E, 0x14021671E, 0x1402168E0,
      0x1402168E9, 0x140216A11, 0x140216A1A, 0x140216C69, 0x140216EC9,
      0x1402172DE, 0x140217C8C, 0x140217D46, 0x140217DC5, 0x140217DD1,
      0x140217EC2, 0x140217F1B, 0x140217F8D, 0x140217FFA, 0x140218153,
      0x1402181A9, 0x1402181B5, 0x14021847A, 0x140218483, 0x14021848C,
      0x140218499, 0x14021857C, 0x1402187D2, 0x1402187DB, 0x1402187E4,
      0x1402187F2, 0x140218808, 0x140218B1F, 0x140218C8E, 0x140218CA8,
      0x140219307
    })
    {
      Utils::Hook(address, AddOpcodePos, HOOK_CALL).install()->quick();
    }

    for (const auto address : std::initializer_list<std::uintptr_t>
    {
      0x140210DEC, 0x1402112B7, 0x1402113B3, 0x1402128EA, 0x140213389,
      0x140214FC7, 0x14021542F, 0x140215847, 0x140215938, 0x140215B58,
      0x140215C18, 0x140215CD8, 0x140215EEA, 0x140215F0B, 0x140215F20,
      0x140215F41, 0x140215F62, 0x140215F83, 0x140215FB9, 0x140217E81,
      0x140219317
    })
    {
      Utils::Hook(address, AddOpcodePos, HOOK_JUMP).install()->quick();
    }

    for (const auto address : std::initializer_list<std::uintptr_t>
    {
      0x140211C30, 0x140212B0E, 0x140213483, 0x1402134A9, 0x1402134DD,
      0x1402134FB, 0x1402135C3, 0x1402135E1, 0x140213611, 0x14021362F,
      0x140213651, 0x14021368B, 0x1402136AE, 0x14021371D, 0x14021373C,
      0x140213780, 0x140214F40, 0x140215090, 0x140215163, 0x1402157C0,
      0x140215ED2, 0x140215EF3, 0x140215F29, 0x140215F4A, 0x140215F6B,
      0x140216386, 0x140218AE2
    })
    {
      Utils::Hook(address, RemoveOpcodePos, HOOK_CALL).install()->quick();
    }
#endif
  }
}

// Where VM_Execute() handles an error, the stub in ScriptError.asm calls us
// with the position of the code that raised it.
//
void ScriptError_VM_Error(const char* codePos)
{
  Components::GSC::ScriptError::VM_Error(codePos);
}
