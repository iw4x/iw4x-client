#include "Common.hpp"

namespace Game
{
  Com_ServerPacketEvent_t Com_ServerPacketEvent = Com_ServerPacketEvent_t(0x1401F5B70);
  Com_AddStartupCommands_t Com_AddStartupCommands = Com_AddStartupCommands_t(0x1401F3200);
  Com_EventLoop_t Com_EventLoop = Com_EventLoop_t(0x1401F3BE0);

  Com_Error_t Com_Error = Com_Error_t(0x1401F3540);
  Com_Shutdown_t Com_Shutdown = Com_Shutdown_t(0x1401F6620);
  char* com_errorMessage = reinterpret_cast<char*>(0x141BD7A00);
  Com_sprintf_t Com_sprintf = Com_sprintf_t(0x14028A3D0);
  va_t va = va_t(0x14028B700);
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
      // index of the current one, and negativeNumbers is at 0x408. We don't
      // have the structures declared, hence the arithmetic.
      //
      g_parse[0x438 * *reinterpret_cast<int*>(g_parse + 0x4380) + 0x408] = parse != 0;
    }
  }

  char* Com_GetCommandLine()
  {
    return reinterpret_cast<char*>(0x141BD7450);
  }

  const char* Com_LoadInfoString_FastFile(const char* fileName, const char* fileDesc, const char* ident, char* loadBuffer)
  {
    auto* rawfile = DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, fileName).rawfile;

    if (DB_IsXAssetDefault(ASSET_TYPE_RAWFILE, fileName))
    {
      Com_Error(ERR_DROP, "\x15" "Could not load %s file [%s]\n", fileDesc, fileName);
    }

    DB_GetRawBuffer(rawfile, loadBuffer, 0x2000);

    const auto identLen = std::strlen(ident);

    if (std::strncmp(loadBuffer, ident, identLen) != 0)
    {
      Com_Error(ERR_DROP, "\x15" "File [%s] is not a %s file\n", fileName, fileDesc);
    }

    return loadBuffer + identLen;
  }

  const char* logFileName = "console_mp.log";
  fileHandle_t logfile = 0;
  bool opening_qconsole = false;
  bool com_consoleLogOpenFailed = false;
  int com_errorPrintsCount = 0;
  void (*Com_OpenLogFileOverride)() = nullptr;

  namespace
  {
    // Move an existing log file out of the way.
    //
    // Rename it to the first free name.NNN, with NNN from 000 to 019, or, if
    // all of them are taken, to the oldest one.
    //
    void Com_RotateLogFile()
    {
      char p[MAX_OSPATH];
      FS_BuildOSPath((*fs_homepath)->current.string, fs_gamedir, logFileName, p);

      std::filesystem::path path(p);
      std::error_code ec;

      if (!std::filesystem::exists(path, ec))
      {
        return;
      }

      constexpr int backupCount = 20;

      auto backup = [&path](int i)
      {
        return std::filesystem::path(std::format("{}.{:03}", path.string(), i));
      };

      int slot(-1);
      std::filesystem::file_time_type oldest;

      for (int i = 0; i < backupCount; ++i)
      {
        std::filesystem::file_time_type t(std::filesystem::last_write_time(backup(i), ec));

        if (ec)
        {
          slot = i;
          break;
        }

        if (slot == -1 || t < oldest)
        {
          slot = i;
          oldest = t;
        }
      }

      std::filesystem::path b(backup(slot));
      std::filesystem::remove(b, ec);
      std::filesystem::rename(path, b, ec);
    }

    // Whether the next log write starts a line.
    //
    bool logLineStart = true;

    // Write msg to the log file, opening it first if needed.
    //
    void Com_LogPrintMessage(const char* msg)
    {
      Sys_EnterCriticalSection(CRITSECT_CONSOLE);

      // Skip the message until the file system can open the log file. Anything
      // printed before that never makes it into the log, which is a pity but
      // hard to avoid.
      //
      if (*fs_searchpaths != nullptr)
      {
        if (logfile == 0)
        {
          if (Com_OpenLogFileOverride != nullptr)
          {
            Com_OpenLogFileOverride();
          }
          else
          {
            Com_OpenLogFile();
          }
        }

        if (logfile != 0)
        {
          // Prefix each line with the time since startup.
          //
          if (logLineStart)
          {
            char timestamp[16];
            std::snprintf(timestamp, sizeof(timestamp), "[%10i] ", Sys_Milliseconds());
            FS_Write(timestamp, static_cast<int>(std::strlen(timestamp)), logfile);
          }

          logLineStart = std::strchr(msg, '\n') != nullptr;
          FS_Write(msg, static_cast<int>(std::strlen(msg)), logfile);

          // Flush every write if logfile is 2 or more.
          //
          if ((*com_logfile)->current.integer > 1)
          {
            FS_Flush(logfile);
          }
        }
      }

      Sys_LeaveCriticalSection(CRITSECT_CONSOLE);
    }
  }

  void Com_OpenLogFile()
  {
    if (!Sys_IsMainThread() || opening_qconsole)
    {
      return;
    }

    // Note that Com_Printf() below comes back here through
    // Com_LogPrintMessage(), and this flag is what stops that from recursing.
    //
    opening_qconsole = true;

    Com_RotateLogFile();

    logfile = FS_FOpenFileWrite(logFileName);
    com_consoleLogOpenFailed = logfile == 0;

    if (logfile != 0)
    {
      std::time_t now = std::time(nullptr);
      Com_Printf(CON_CHANNEL_SYSTEM, "'%s'\n", Com_GetCommandLine());
      Com_Printf(CON_CHANNEL_SYSTEM, "Logfile opened on %s\n", std::asctime(std::localtime(&now)));
    }

    opening_qconsole = false;
  }

  bool (*Com_PrintMessageInterceptor)(int channel, const char* msg) = nullptr;

  void Com_PrintMessage(int channel, const char* msg, int error)
  {
    if (Com_PrintMessageInterceptor != nullptr && Com_PrintMessageInterceptor(channel, msg))
    {
      return;
    }

    // Add the message to the in-game console, with the error type selecting
    // its color.
    //
    if (channel != CON_CHANNEL_LOGFILEONLY)
    {
      CL_ConsolePrint(0, channel, msg, 0, 0, error << 5);
    }

    // Strip a leading color code.
    //
    if (msg[0] == '^' && msg[1] != '\0')
    {
      msg += 2;
    }

    // With com_filter_output, the system console only shows what the in-game
    // console does.
    //
    if (channel != CON_CHANNEL_LOGFILEONLY &&
        (*com_filter_output == nullptr ||
         !(*com_filter_output)->current.enabled ||
         Con_IsConsoleChannelVisible(channel, 3)))
    {
      Sys_Print(msg);
    }

    if (channel != CON_CHANNEL_CONSOLEONLY && *com_logfile != nullptr && (*com_logfile)->current.integer != 0)
    {
      Com_LogPrintMessage(msg);
    }
  }

  void Com_Printf(int channel, const char* fmt, ...)
  {
    char msg[0x1000];

    va_list ap;
    va_start(ap, fmt);
    std::vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    Com_PrintMessage(channel, msg, 0);
  }

  void Com_DPrintf(int channel, const char* fmt, ...)
  {
    if (*com_developer == nullptr || (*com_developer)->current.integer == 0)
    {
      return;
    }

    char msg[0x1000];

    va_list ap;
    va_start(ap, fmt);
    std::vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    Com_Printf(channel, "%s", msg);
  }

  void Com_PrintError(int channel, const char* fmt, ...)
  {
    // Mark the message red and, unless it already says so, as an error.
    //
    std::string_view f(fmt);
    std::string_view e("error");
    bool mentionsError(!std::ranges::search(f, e, [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) == b; }).empty());

    char msg[0x1000];
    std::strcpy(msg, mentionsError ? "^1" : "^1Error: ");
    std::size_t n(std::strlen(msg));

    va_list ap;
    va_start(ap, fmt);
    std::vsnprintf(msg + n, sizeof(msg) - n, fmt, ap);
    va_end(ap);

    ++com_errorPrintsCount;

    // @@ x86 also passes msg to sub_4B62C0, which we haven't identified yet.
    //    It then opens the console on the first error, which x64 doesn't have.
    //    Probably nothing important, but we don't really know.
    //
    Com_PrintMessage(channel, msg, 3);
  }

  void Com_StartupVariable(const char* match)
  {
    for (int i = 0; i < *com_numConsoleLines; ++i)
    {
      Cmd_TokenizeString(com_consoleLines[i]);

      const int n = cmd_args->nesting;
      const int argc = cmd_args->argc[n];
      const char** argv = cmd_args->argv[n];

      if (argc > 1 && (!match || !std::strcmp(argv[1], match)))
      {
        if (!_stricmp(argv[0], "set"))
        {
          Dvar_Set_f();
        }
        else if (!_stricmp(argv[0], "seta"))
        {
          Com_Printf(CON_CHANNEL_SYSTEM,
            "\"SetA\" is deprecated.  Forcing \"%s\" to be a \"Set\" command, instead.\n",
            com_consoleLines[i]);

          Dvar_Set_f();
        }
      }

      Cmd_EndTokenizedString();
    }
  }

  void Com_ClientPacketEvent()
  {
    // The loopback packets of a socket, then the packets the network thread
    // queued, and then those still on the socket (Sys_GetPacket()).
    //
    const auto NET_GetLoopPacket(reinterpret_cast<int (*)(netsrc_t sock, netadr_t* from, msg_t* msg)>(0x140204D30));
    const auto NET_GetQueuedPacket(reinterpret_cast<bool (*)(netadr_t* from, msg_t* msg)>(0x140204C70));
    const auto Sys_GetPacket(reinterpret_cast<int (*)(netadr_t* from, msg_t* msg)>(0x140204C60));

    const auto CL_PacketEvent(reinterpret_cast<bool (*)(int localClientNum, netadr_t* from, msg_t* msg, int time)>(0x1400FC620));

    // Note that the game takes the buffer from its stack allocator, while we
    // keep our own, which is fine since only the main thread ever gets here.
    //
    assert(Sys_IsMainThread());

    static unsigned char buf[0x20000];

    msg_t msg;
    MSG_Init(&msg, buf, sizeof(buf));

    netadr_t from;
    netadr_t to;

    while (NET_GetLoopPacket(NS_CLIENT1, &from, &msg))
    {
      to = from;
      CL_PacketEvent(0, &to, &msg, Sys_Milliseconds());
    }

    while (NET_GetQueuedPacket(&from, &msg))
    {
      to = from;
      CL_PacketEvent(0, &to, &msg, Sys_Milliseconds());
    }

    if (!(*com_sv_running)->current.enabled)
    {
      while (Sys_GetPacket(&from, &msg))
      {
        to = from;
        CL_PacketEvent(0, &to, &msg, Sys_Milliseconds());
      }
    }
  }

  void Com_PrintWarning(int channel, const char* fmt, ...)
  {
    // Mark the message yellow.
    //
    char msg[0x1000];
    std::strcpy(msg, "^3");
    std::size_t n(std::strlen(msg));

    va_list ap;
    va_start(ap, fmt);
    std::vsnprintf(msg + n, sizeof(msg) - n, fmt, ap);
    va_end(ap);

    Com_PrintMessage(channel, msg, 2);
  }
}
