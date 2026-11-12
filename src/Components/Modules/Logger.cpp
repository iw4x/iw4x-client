#include "Logger.hpp"
#include "Console.hpp"
#include "Events.hpp"
#include "Sentry.hpp"

namespace Components
{
  using namespace Utils::String;

  void (*Logger::PipeCallback)(const std::string&) = nullptr;

  std::mutex Logger::MessageMutex;
  std::vector<std::string> Logger::MessageQueue;

  std::recursive_mutex Logger::LoggingMutex;
  std::vector<Network::Address> Logger::LoggingAddresses[2];

  Dvar::Var Logger::IW4x_one_log;
  Dvar::Var Logger::IW4x_fail2ban_location;


  bool Logger::IsConsoleReady()
  {
    return (IsWindow(Console::GetWindow()) != FALSE || (Dedicated::IsEnabled() && !Flags::HasFlag("console")));
  }

  void Logger::Print_Stub(const int channel, const char* message, ...)
  {
    char buf[4096]{};

    va_list va;
    va_start(va, message);
    vsnprintf_s(buf, _TRUNCATE, message, va);
    va_end(va);

    MessagePrint(channel, std::string{ buf });
  }

  void Logger::MessagePrint(int channel, const std::string& msg)
  {
    static const auto shouldPrint = []() -> bool
    {
      return Flags::HasFlag("stdout");
    }();

      if (shouldPrint)
      {
        if (channel == Game::CON_CHANNEL_LOGFILEONLY)
        {
          channel = Game::CON_CHANNEL_DONT_FILTER;
        }
      }

#ifdef _DEBUG
      if (!IsConsoleReady())
      {
        OutputDebugStringA(msg.data());
      }
#endif

      if (!Game::Sys_IsMainThread())
      {
        EnqueueMessage(msg);
      }
      else
      {
        Game::Com_PrintMessage(channel, msg.data(), 0);
      }
  }

  void Logger::DebugInternal(const std::string_view& fmt, std::format_args&& args, [[maybe_unused]] const std::source_location& loc)
  {
#ifdef LOGGER_TRACE
    const auto msg = std::vformat(fmt, args);
    const auto out = std::format("Debug:\n    {}\nFile:    {}\nLine:    {}\n", msg, loc.file_name(), loc.line());
#else
    const auto msg = std::vformat(fmt, args);
    const auto out = std::format("^2{}\n", msg);
#endif

    MessagePrint(Game::CON_CHANNEL_DONT_FILTER, out);
  }

  void Logger::PrintInternal(Game::conChannel_t channel, const std::string_view& fmt, std::format_args&& args)
  {
    const auto msg = std::vformat(fmt, args);

    MessagePrint(channel, msg);
  }

  void Logger::ErrorInternal(const Game::errorParm_t error, const std::string_view& fmt, std::format_args&& args)
  {
    const auto msg = std::vformat(fmt, args);

#ifdef _DEBUG
    if (IsDebuggerPresent()) __debugbreak();
#endif

    Sentry::CaptureError(error, msg);

    Game::Com_Error(error, "%s", msg.data());
  }

  void Logger::PrintErrorInternal(Game::conChannel_t channel, const std::string_view& fmt, std::format_args&& args)
  {
    const auto msg = "^1Error: " + std::vformat(fmt, args);

    ++Game::com_errorPrintsCount;
    MessagePrint(channel, msg);

    if (Game::cls->uiStarted && (*Game::com_fixedConsolePosition == 0))
    {
      Game::CL_ConsoleFixPosition();
    }
  }

  void Logger::WarningInternal(Game::conChannel_t channel, const std::string_view& fmt, std::format_args&& args)
  {
    const auto msg = "^3" + std::vformat(fmt, args);

    MessagePrint(channel, msg);
  }

  void Logger::PrintFail2BanInternal(const std::string_view& fmt, std::format_args&& args)
  {
    static const auto shouldPrint = []() -> bool
    {
      return Flags::HasFlag("fail2ban");
    }();

    if (!shouldPrint)
    {
      return;
    }

    auto msg = std::vformat(fmt, args);

    static auto log_next_time_stamp = true;
    if (log_next_time_stamp)
    {
      auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
      // Convert to local time
      std::tm timeInfo = *std::localtime(&now);

      std::ostringstream ss;
      ss << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S ");

      msg.insert(0, ss.str());
    }

    log_next_time_stamp = (msg.find('\n') != std::string::npos);

    Utils::IO::WriteFile(IW4x_fail2ban_location.get<std::string>(), msg, true);
  }

  void Logger::Frame()
  {
    std::unique_lock _(MessageMutex);

    for (auto i = MessageQueue.begin(); i != MessageQueue.end();)
    {
      Game::Com_PrintMessage(Game::CON_CHANNEL_DONT_FILTER, i->data(), 0);

#ifdef _DEBUG
      if (!IsConsoleReady())
      {
        OutputDebugStringA(i->data());
      }
#endif

      i = MessageQueue.erase(i);
    }
  }

  void Logger::PipeOutput(void(*callback)(const std::string&))
  {
    PipeCallback = callback;
  }

  void Logger::PrintMessagePipe(const char* data)
  {
    if (PipeCallback)
    {
      PipeCallback(data);
    }
  }

  // Pass each printed message to Sentry and then either to the pipe, which
  // takes it, or to the network log.
  //
  bool Logger::InterceptPrintMessage([[maybe_unused]] int channel, const char* msg)
  {
    PrintMessageSentry(msg);

    if (PipeCallback)
    {
      PrintMessagePipe(msg);
      return true;
    }

    NetworkLog(msg, false);
    return false;
  }

  const Game::dvar_t* Logger::Dvar_RegisterLogfile(const char* dvarName, [[maybe_unused]] int value, int min, int max, std::uint16_t flags, const char* description)
  {
    return Game::Dvar_RegisterInt(dvarName, 1, min, max, flags, description);
  }

  void Logger::PrintMessageSentry(const char* data)
  {
    Sentry::Print(data);
  }

  void Logger::NetworkLog(const char* data, bool gLog)
  {
    if (!data)
    {
      return;
    }

    std::unique_lock lock(LoggingMutex);
    for (const auto& addr : LoggingAddresses[gLog & 1])
    {
      Network::SendCommand(addr, "print", data);
    }
  }

  void Logger::G_LogPrintf_Hk(const char* fmt, ...)
  {
    char string[1024]{};
    char string2[1024]{};

    va_list ap;
    va_start(ap, fmt);
    vsnprintf_s(string2, _TRUNCATE, fmt, ap);
    va_end(ap);

    const auto time = Game::level->time / 1000;
    // Format the line, truncating if necessary. The prefix plus a message of
    // up to 1023 characters may not fit, and sprintf_s() would invoke the
    // invalid parameter handler (and terminate us) in that case.
    //
    if (_snprintf_s(string, _TRUNCATE, "%3i:%i%i %s", time / 60, time % 60 / 10, time % 60 % 10, string2) < 0)
    {
      // Keep the line terminated.
      //
      string[sizeof(string) - 2] = '\n';
    }

    const auto len = static_cast<int>(std::strlen(string));

    if (Game::level->logFile)
    {
      Game::FS_Write(string, len, Game::level->logFile);
    }

    // Allow the network log to run even if logFile was not opened
    NetworkLog(string, true);
  }

  void Logger::EnqueueMessage(const std::string& message)
  {
    std::unique_lock _(MessageMutex);
    MessageQueue.push_back(message);
  }

  void Logger::RedirectOSPath(const char* file, char* folder)
  {
    const auto* g_log = (*Game::g_log) ? (*Game::g_log)->current.string : "";

    if (g_log) // This can be null - has happened before
    {
      if (std::strcmp(g_log, file) == 0)
      {
        if (std::strcmp(folder, "userraw") != 0)
        {
          if (IW4x_one_log.get<bool>())
          {
            strncpy_s(folder, 256, "userraw", _TRUNCATE);
          }
        }
      }
    }
  }

  void Logger::LSP_LogString_Stub([[maybe_unused]] int localControllerIndex, const char* string)
  {
    NetworkLog(string, false);
  }

  void Logger::LSP_LogStringAboutUser_Stub([[maybe_unused]] int localControllerIndex, std::uint64_t xuid, const char* string)
  {
    NetworkLog(VA("%" PRIx64 ";%s", xuid, string), false);
  }

  void Logger::AddServerCommands()
  {
    Command::AddSV("log_add", [](const Command::Params* params)
      {
        if (params->size() < 2) return;

        std::unique_lock lock(LoggingMutex);

        Network::Address addr(params->get(1));
        if (std::ranges::find(LoggingAddresses[0], addr) == LoggingAddresses[0].end())
        {
          LoggingAddresses[0].push_back(addr);
        }
      });

    Command::AddSV("log_del", [](const Command::Params* params)
      {
        if (params->size() < 2) return;

        std::unique_lock lock(LoggingMutex);

        const auto num = std::atoi(params->get(1));
        if (!std::strcmp(VA("%i", num), params->get(1)) && static_cast<unsigned int>(num) < LoggingAddresses[0].size())
        {
          auto addr = Logger::LoggingAddresses[0].begin() + num;
          Print("Address {} removed\n", addr->getString());
          LoggingAddresses[0].erase(addr);
        }
        else
        {
          Network::Address addr(params->get(1));

          if (const auto i = std::ranges::find(LoggingAddresses[0], addr); i != LoggingAddresses[0].end())
          {
            LoggingAddresses[0].erase(i);
            Print("Address {} removed\n", addr.getString());
          }
          else
          {
            Print("Address {} not found!\n", addr.getString());
          }
        }
      });

    Command::AddSV("log_list", []([[maybe_unused]] const Command::Params* params)
      {
        Print("# ID: Address\n");
        Print("-------------\n");

        std::unique_lock lock(LoggingMutex);

        for (unsigned int i = 0; i < LoggingAddresses[0].size(); ++i)
        {
          Print("#{:03d}: {}\n", i, LoggingAddresses[0][i].getString());
        }
      });

    Command::AddSV("g_log_add", [](const Command::Params* params)
      {
        if (params->size() < 2) return;

        std::unique_lock lock(LoggingMutex);

        const Network::Address addr(params->get(1));
        if (std::ranges::find(LoggingAddresses[1], addr) == LoggingAddresses[1].end())
        {
          LoggingAddresses[1].push_back(addr);
        }
      });

    Command::AddSV("g_log_del", [](const Command::Params* params)
      {
        if (params->size() < 2) return;

        std::unique_lock lock(LoggingMutex);

        const auto num = std::atoi(params->get(1));
        if (!std::strcmp(VA("%i", num), params->get(1)) && static_cast<unsigned int>(num) < LoggingAddresses[1].size())
        {
          const auto addr = LoggingAddresses[1].begin() + num;
          Print("Address {} removed\n", addr->getString());
          LoggingAddresses[1].erase(addr);
        }
        else
        {
          const Network::Address addr(params->get(1));
          const auto i = std::ranges::find(LoggingAddresses[1].begin(), LoggingAddresses[1].end(), addr);
          if (i != LoggingAddresses[1].end())
          {
            LoggingAddresses[1].erase(i);
            Print("Address {} removed\n", addr.getString());
          }
          else
          {
            Print("Address {} not found!\n", addr.getString());
          }
        }
      });

    Command::AddSV("g_log_list", []([[maybe_unused]] const Command::Params* params)
      {
        Print("# ID: Address\n");
        Print("-------------\n");

        std::unique_lock lock(LoggingMutex);
        for (std::size_t i = 0; i < LoggingAddresses[1].size(); ++i)
        {
          Print("#{:03d}: {}\n", i, LoggingAddresses[1][i].getString());
        }
      });
  }

  Logger::Logger()
  {
    // set logfile to 1 by default (logs enabled)
    Utils::Hook(0x1401F4A75, Dvar_RegisterLogfile, HOOK_CALL).install()->quick();

    // See Logger.asm for the registers at the hook point, after
    // FS_BuildOSPath() falls back to the default game folder.
    //
    Utils::Hook(0x140273B19, Logger_BuildOSPath_Stub, HOOK_JUMP).install()->quick();

    Scheduler::Loop(Frame, Scheduler::Pipeline::SERVER);

    Utils::Hook(Game::G_LogPrintf, G_LogPrintf_Hk, HOOK_JUMP).install()->quick();
    Game::Com_PrintMessageInterceptor = InterceptPrintMessage;

    Utils::Hook(0x1401A3108, LSP_LogString_Stub, HOOK_JUMP).install()->quick(); // Scr_LogString
    Utils::Hook(0x1401A3144, LSP_LogStringAboutUser_Stub, HOOK_JUMP).install()->quick(); // ScrCmd_LogString_Stub

    Events::OnSVInit(AddServerCommands);
    Events::OnDvarInit([]
      {
        IW4x_one_log = Dvar::Register<bool>("iw4x_onelog", false, Game::DVAR_LATCH, "Only write the game log to the 'userraw' OS folder");
        IW4x_fail2ban_location = Dvar::Register<const char*>("iw4x_fail2ban_location", "/var/log/iw4x.log", Game::DVAR_NONE, "Fail2Ban logfile location");
      });
  }

  Logger::~Logger()
  {
    std::unique_lock lock_logging(LoggingMutex);
    LoggingAddresses[0].clear();
    LoggingAddresses[1].clear();

    std::unique_lock lock_message(MessageMutex);
    MessageQueue.clear();

    Game::Com_PrintMessageInterceptor = nullptr;

    // Flush the console log
    if (Game::logfile)
    {
      Game::FS_FCloseFile(Game::logfile);
    }
  }
}

void Logger_RedirectOSPath(const char* file, char* folder)
{
  Components::Logger::RedirectOSPath(file, folder);
}
