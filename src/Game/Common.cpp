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

  const char* logFileName = "console_mp.log";
  fileHandle_t logfile = 0;
  bool opening_qconsole = false;
  bool com_consoleLogOpenFailed = false;
  int com_errorPrintsCount = 0;

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

      // Skip the message until the file system can open the log file.
      //
      if (*fs_searchpaths != nullptr)
      {
        if (logfile == 0)
        {
          Com_OpenLogFile();
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
    // Com_LogPrintMessage(), which this flag stops.
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

  void Com_PrintMessage(int channel, const char* msg, int error)
  {
    // Note that x86 first adds the message to the console buffer, with
    // error selecting the color. x64 has no console buffer, so we only
    // print to the system console and the log file.
    //
    static_cast<void>(error);

    // Strip a leading color code.
    //
    if (msg[0] == '^' && msg[1] != '\0')
    {
      msg += 2;
    }

    if (channel != CON_CHANNEL_LOGFILEONLY)
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

    // @@ TODO: x86 also passes msg to sub_4B62C0, which we haven't
    //    identified yet. It then opens the console on the first error, which
    //    x64 doesn't have.
    //
    Com_PrintMessage(channel, msg, 3);
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
