#include "Companion.hpp"

#include <charconv>

#include <libcompanion/protocol.hxx>

namespace Steam
{
  std::mutex Companion::Mutex;
  std::jthread Companion::Worker;
  HANDLE Companion::StopEvent = nullptr;

  HANDLE Companion::RecordMapping = nullptr;
  void* Companion::RecordView = nullptr;
  std::wstring Companion::UnixRecordPath;

  namespace
  {
    // Return true if Companion accepts the title as is (see
    // valid_extra_info() in libcompanion). Note that Companion also rejects
    // invalid UTF-8, which our titles never contain since they are literals.
    //
    bool IsValidTitle(const std::string& title)
    {
      if (title.empty() || title.size() >= companion::extra_info_capacity) return false;

      return std::ranges::none_of(title, [](const char c)
      {
        const auto b = static_cast<unsigned char>(c);
        return b < 0x20 || b == 0x7F;
      });
    }

    // Return the Windows object name with the id substituted into the
    // protocol format, which is an swprintf() format with a single %lu
    // conversion.
    //
    std::wstring ObjectName(const wchar_t* format, DWORD id)
    {
      wchar_t name[companion::object_name_capacity]{};
      std::swprintf(name, std::size(name), format, static_cast<unsigned long>(id));
      return name;
    }

    // As above but for the Wine paths, whose formats have a single %u
    // conversion.
    //
    std::wstring WinePath(const wchar_t* format, std::uint32_t id)
    {
      wchar_t path[companion::unix_path_capacity]{};
      std::swprintf(path, std::size(path), format, static_cast<unsigned int>(id));
      return path;
    }

    // Return the process id that the Steam client records in the registry or
    // 0 if there is none. Note that the client may have exited since.
    //
    DWORD GetSteamProcessId()
    {
      DWORD pid = 0;
      DWORD size = sizeof(pid);

      if (RegGetValueA(HKEY_CURRENT_USER, STEAM_REGISTRY_PROCESS_PATH, "pid", RRF_RT_REG_DWORD, nullptr, &pid, &size) != ERROR_SUCCESS)
      {
        return 0;
      }

      return pid;
    }

    // Tell Companion in the Steam client that our record changed. Companion
    // then resends the last report to the CM server, which is how friends
    // see a record published after Steam reported the game. The event only
    // exists while Companion runs, so without Companion this does nothing.
    //
    void SignalRecordChange(DWORD steamPid)
    {
      if (!steamPid) return;

      const auto name = ObjectName(companion::record_event_format, steamPid);
      const auto event = OpenEventW(EVENT_MODIFY_STATE, FALSE, name.data());

      if (event)
      {
        SetEvent(event);
        CloseHandle(event);
      }
    }

    // Return the state if it is one that we know.
    //
    std::optional<companion::companion_state> ToState(std::uint32_t state)
    {
      if (state > static_cast<std::uint32_t>(companion::companion_state::failed)) return {};
      return static_cast<companion::companion_state>(state);
    }

    // Return the state that Companion publishes in the Steam client. Return
    // nothing if Companion is not running or publishes a status layout that
    // we don't know.
    //
    std::optional<companion::companion_state> ReadStatus(DWORD steamPid)
    {
      const auto name = ObjectName(companion::status_name_format, steamPid);
      const auto mapping = OpenFileMappingW(FILE_MAP_READ, FALSE, name.data());
      if (!mapping) return {};

      auto* view = static_cast<companion::status*>(MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, sizeof(companion::status)));
      CloseHandle(mapping);

      if (!view) return {};

      // Load the state first with acquire ordering and only then copy the
      // other fields. Companion stores the state last with release ordering,
      // so this way we see the fields that go with the state.
      //
      const auto state = std::atomic_ref(view->state).load(std::memory_order_acquire);

      companion::status status;
      std::memcpy(&status, view, sizeof(status));
      UnmapViewOfFile(view);

      if (status.magic != companion::status_magic || status.version != companion::status_version || status.size != sizeof(companion::status))
      {
        return {};
      }

      return ToState(state);
    }

    // Read the Unix file through Wine's \\?\unix\ namespace. Return nothing
    // if we can't open the file.
    //
    std::optional<std::string> ReadUnixFile(const std::wstring& path)
    {
      const auto file = CreateFileW(path.data(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
      if (file == INVALID_HANDLE_VALUE) return {};

      // Read until the end since files in /proc report a size of zero.
      //
      std::string data;
      char buffer[4096];
      DWORD read = 0;

      while (ReadFile(file, buffer, sizeof(buffer), &read, nullptr) && read)
      {
        data.append(buffer, read);
      }

      CloseHandle(file);
      return data;
    }

    // Parse the process id (field 1) and the start time (field 22) from the
    // content of /proc/<pid>/stat. Return false if the content is malformed.
    //
    // The command name (field 2) is in parentheses and can contain anything,
    // including spaces and parentheses, so we count the fields after it from
    // its last closing parenthesis.
    //
    bool ParseStat(const std::string& stat, std::uint32_t& pid, std::uint64_t& startTime)
    {
      const auto open = stat.find(" (");
      const auto close = stat.rfind(')');
      if (open == std::string::npos || close == std::string::npos || close < open) return false;

      const auto [end, error] = std::from_chars(stat.data(), stat.data() + open, pid);
      if (error != std::errc{} || end != stat.data() + open || !pid) return false;

      std::istringstream fields(stat.substr(close + 1));
      std::string field;

      for (auto i = 3; i != 22; ++i)
      {
        if (!(fields >> field)) return false;
      }

      return static_cast<bool>(fields >> startTime);
    }

    // Parse the real user id, which is the first value of the Uid line in
    // the content of /proc/<pid>/status. Return nothing if there is no such
    // value.
    //
    std::optional<std::uint32_t> ParseUid(const std::string& status)
    {
      constexpr std::string_view key = "\nUid:";

      const auto line = status.find(key);
      if (line == std::string::npos) return {};

      const auto begin = status.find_first_not_of(" \t", line + key.size());
      if (begin == std::string::npos) return {};

      std::uint32_t uid = 0;
      const auto [end, error] = std::from_chars(status.data() + begin, status.data() + status.size(), uid);
      if (error != std::errc{}) return {};

      return uid;
    }

    // Return the state that Companion publishes for our user under Wine (see
    // ReadStatus() for details).
    //
    std::optional<companion::companion_state> ReadUnixStatus()
    {
      const auto status = ReadUnixFile(L"\\\\?\\unix\\proc\\self\\status");
      if (!status) return {};

      const auto uid = ParseUid(*status);
      if (!uid) return {};

      // Make sure the status has the exact size. Companion writes it in one
      // go, so a status of any other size isn't one that we know.
      //
      const auto data = ReadUnixFile(WinePath(companion::wine_status_format, *uid));
      if (!data || data->size() != sizeof(companion::unix_status)) return {};

      companion::unix_status unixStatus;
      std::memcpy(&unixStatus, data->data(), sizeof(unixStatus));

      if (unixStatus.magic != companion::unix_status_magic || unixStatus.version != companion::unix_status_version || unixStatus.size != sizeof(companion::unix_status))
      {
        return {};
      }

      return ToState(unixStatus.state);
    }

    // Tell the player about Companion's state.
    //
    void PrintState(companion::companion_state state)
    {
      switch (state)
      {
      case companion::companion_state::starting:
        Components::Logger::Print("Steam companion is starting\n");
        break;
      case companion::companion_state::active:
        Components::Logger::Print("Steam companion is active\n");
        break;
      case companion::companion_state::unsupported:
        Components::Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "Steam companion does not support this Steam client version\n");
        break;
      case companion::companion_state::failed:
        Components::Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "Steam companion failed to hook the Steam client\n");
        break;
      }
    }
  }

  void Companion::Register(std::uint32_t appId, const std::string& title)
  {
    std::lock_guard _(Companion::Mutex);

    if (Companion::StopEvent) return;

    Companion::StopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!Companion::StopEvent) return;

    Companion::Worker = std::jthread(Companion::Run, appId, title);
  }

  void Companion::Unregister()
  {
    std::lock_guard _(Companion::Mutex);

    if (!Companion::StopEvent) return;

    SetEvent(Companion::StopEvent);

    if (Companion::Worker.joinable())
    {
      Companion::Worker.join();
    }

    CloseHandle(Companion::StopEvent);
    Companion::StopEvent = nullptr;

    if (Companion::RecordMapping)
    {
      UnmapViewOfFile(Companion::RecordView);
      CloseHandle(Companion::RecordMapping);

      Companion::RecordView = nullptr;
      Companion::RecordMapping = nullptr;

      SignalRecordChange(GetSteamProcessId());
    }

    if (!Companion::UnixRecordPath.empty())
    {
      DeleteFileW(Companion::UnixRecordPath.data());
      Companion::UnixRecordPath.clear();
    }
  }

  // Register the game with Companion on the background thread. Report any
  // exception to the console since the thread has nobody to propagate it
  // to.
  //
  void Companion::Run(std::uint32_t appId, std::string title)
  {
    try
    {
      if (!IsValidTitle(title))
      {
        Components::Logger::PrintError(Game::CON_CHANNEL_DONT_FILTER, "Invalid Steam companion title '{}'\n", title);
        return;
      }

      if (::Utils::IsWineEnvironment())
      {
        Companion::RunWine(appId, title);
      }
      else
      {
        Companion::RunWindows(appId, title);
      }
    }
    catch (const std::exception& ex)
    {
      Components::Logger::PrintError(Game::CON_CHANNEL_DONT_FILTER, "Steam companion: {}\n", ex.what());
    }
  }

  // Publish the record and make sure that Companion runs in the Windows
  // Steam client.
  //
  void Companion::RunWindows(std::uint32_t appId, const std::string& title)
  {
    if (!Companion::PublishRecord(appId, title)) return;

    const auto steamPid = GetSteamProcessId();
    if (!steamPid) return;

    // See if Companion is already in the Steam client. It stays there until
    // the client exits, so it is normally there from an earlier game
    // session, in which case we signal it to pick up the record. Otherwise
    // we attach it. Note that we also attach while Companion is still
    // starting since the attach waits for it to settle and reports the
    // outcome.
    //
    if (const auto state = ReadStatus(steamPid); state && *state != companion::companion_state::starting)
    {
      if (*state == companion::companion_state::active)
      {
        SignalRecordChange(steamPid);
      }

      PrintState(*state);
      return;
    }

    Companion::Attach(steamPid);
  }

  // Publish the record for Companion in the native Linux Steam client, with
  // which the game runs under Wine. The client can only load Companion when
  // it starts, so all we do is publish the record and tell the player
  // whether Companion is there to pick it up. Companion watches the record
  // files, so there is nothing to signal.
  //
  void Companion::RunWine(std::uint32_t appId, const std::string& title)
  {
    if (!Companion::PublishUnixRecord(appId, title)) return;

    if (const auto state = ReadUnixStatus())
    {
      PrintState(*state);
    }
    else
    {
      Components::Logger::Print("Steam companion is not running, start Steam with companion to show the IW4x title to Steam friends\n");
    }
  }

  // Publish the record as a named file mapping. Return false and warn the
  // player if we can't.
  //
  bool Companion::PublishRecord(std::uint32_t appId, const std::string& title)
  {
    const auto pid = GetCurrentProcessId();
    const auto name = ObjectName(companion::record_name_format, pid);

    // Create the mapping, which exists while we hold this handle. This way
    // the record goes away with the game even if it never unregisters.
    //
    Companion::RecordMapping = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(companion::record), name.data());

    if (!Companion::RecordMapping)
    {
      Components::Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "Unable to create the Steam companion record ({})\n", GetLastError());
      return false;
    }

    Companion::RecordView = MapViewOfFile(Companion::RecordMapping, FILE_MAP_WRITE, 0, 0, sizeof(companion::record));

    if (!Companion::RecordView)
    {
      Components::Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "Unable to map the Steam companion record ({})\n", GetLastError());

      CloseHandle(Companion::RecordMapping);
      Companion::RecordMapping = nullptr;
      return false;
    }

    auto& record = *static_cast<companion::record*>(Companion::RecordView);

    record.version = companion::record_version;
    record.size = sizeof(companion::record);
    record.process_id = pid;
    record.app_id = appId;

    std::memset(record.extra_info, 0, sizeof(record.extra_info));
    std::memcpy(record.extra_info, title.data(), title.size());

    // Store the magic last with release ordering. Companion loads it first
    // with acquire ordering, so a record with the magic in place is always
    // complete.
    //
    std::atomic_ref(record.magic).store(companion::record_magic, std::memory_order_release);

    return true;
  }

  // Publish the record as a file in /dev/shm through Wine. Return false and
  // warn the player if we can't.
  //
  bool Companion::PublishUnixRecord(std::uint32_t appId, const std::string& title)
  {
    // Identify our wineserver. Wine performs file operations in the
    // wineserver of our prefix, so /proc/self is the wineserver. Companion
    // finds the record by this wineserver's process id, either in the
    // process tree of the game that Steam reports or through our Wine
    // prefix.
    //
    const auto stat = ReadUnixFile(L"\\\\?\\unix\\proc\\self\\stat");

    std::uint32_t pid = 0;
    std::uint64_t startTime = 0;

    if (!stat || !ParseStat(*stat, pid, startTime))
    {
      Components::Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "Unable to identify the wineserver process for the Steam companion\n");
      return false;
    }

    companion::unix_record record{};

    record.magic = companion::unix_record_magic;
    record.version = companion::unix_record_version;
    record.size = sizeof(companion::unix_record);
    record.process_id = pid;
    record.start_time_low = static_cast<std::uint32_t>(startTime);
    record.start_time_high = static_cast<std::uint32_t>(startTime >> 32);
    record.app_id = appId;

    std::memcpy(record.extra_info, title.data(), title.size());

    // Write the record in one go since Companion only accepts a record file
    // of the exact record size.
    //
    const auto path = WinePath(companion::wine_record_format, pid);
    const auto file = CreateFileW(path.data(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_DELETE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (file == INVALID_HANDLE_VALUE)
    {
      Components::Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "Unable to create the Steam companion record ({})\n", GetLastError());
      return false;
    }

    DWORD written = 0;
    const auto result = WriteFile(file, &record, sizeof(record), &written, nullptr);
    const auto error = GetLastError();

    CloseHandle(file);

    if (!result || written != sizeof(record))
    {
      Components::Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "Unable to write the Steam companion record ({})\n", error);
      DeleteFileW(path.data());
      return false;
    }

    Companion::UnixRecordPath = path;
    return true;
  }

  // Load Companion into the Steam client. The Steam client is 64-bit and
  // only a 64-bit process can do this, so we run the module's attach entry
  // point in the 64-bit rundll32.exe, which reports the outcome as its exit
  // code.
  //
  void Companion::Attach(DWORD steamPid)
  {
    BOOL wow64 = FALSE;
    if (!IsWow64Process(GetCurrentProcess(), &wow64) || !wow64) return;

    // Find the 64-bit rundll32.exe. In a 32-bit process System32 is
    // redirected to SysWOW64 and Sysnative is the alias under which the
    // 64-bit System32 stays visible.
    //
    wchar_t windows[MAX_PATH]{};
    const auto length = GetWindowsDirectoryW(windows, ARRAYSIZE(windows));
    if (!length || length >= ARRAYSIZE(windows)) return;

    const auto rundll = std::filesystem::path(windows) / L"Sysnative" / L"rundll32.exe";

    // Find Companion's module, which is installed next to iw4x.dll.
    //
    HMODULE self = nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<LPCWSTR>(&Companion::Mutex), &self))
    {
      return;
    }

    std::wstring selfPath(32768, L'\0');
    const auto selfLength = GetModuleFileNameW(self, selfPath.data(), static_cast<DWORD>(selfPath.size()));
    if (!selfLength || selfLength >= selfPath.size()) return;

    selfPath.resize(selfLength);

    const auto module = std::filesystem::path(selfPath).parent_path() / companion::companion_module_name;

    std::error_code ec;
    if (!std::filesystem::is_regular_file(module, ec))
    {
      Components::Logger::Print("{} is not installed, Steam friends see the plain Steam game\n", std::filesystem::path(companion::companion_module_name).string());
      return;
    }

    const std::string entryPoint = companion::attach_entry_point;
    auto commandLine = std::format(L"\"{}\" \"{}\",{} {}", rundll.wstring(), module.wstring(), std::wstring(entryPoint.begin(), entryPoint.end()), steamPid);

    STARTUPINFOW startupInfo{};
    startupInfo.cb = sizeof(startupInfo);

    PROCESS_INFORMATION processInfo{};

    if (!CreateProcessW(rundll.c_str(), commandLine.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &startupInfo, &processInfo))
    {
      Components::Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "Unable to start the Steam companion attach ({})\n", GetLastError());
      return;
    }

    CloseHandle(processInfo.hThread);

    // Wait for the attach, which takes at most 15 seconds (10 to load the
    // module and 5 for it to settle). If the game shuts down first, we leave
    // rundll32.exe to finish on its own.
    //
    HANDLE handles[] = { processInfo.hProcess, Companion::StopEvent };
    const auto wait = WaitForMultipleObjects(ARRAYSIZE(handles), handles, FALSE, INFINITE);

    DWORD exitCode = 0;
    const auto exited = wait == WAIT_OBJECT_0 && GetExitCodeProcess(processInfo.hProcess, &exitCode);

    CloseHandle(processInfo.hProcess);

    if (!exited) return;

    switch (static_cast<companion::attach_result>(exitCode))
    {
    case companion::attach_result::active:
      SignalRecordChange(steamPid);
      PrintState(companion::companion_state::active);
      break;
    case companion::attach_result::unsupported:
      PrintState(companion::companion_state::unsupported);
      break;
    case companion::attach_result::failed:
      PrintState(companion::companion_state::failed);
      break;
    case companion::attach_result::not_steam:
      // The registry still names a Steam client that has exited.
      break;
    case companion::attach_result::access_denied:
      Components::Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "Unable to attach the Steam companion, Steam runs with higher privileges than the game\n");
      break;
    case companion::attach_result::injection_failed:
      Components::Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "Unable to load the Steam companion into the Steam client\n");
      break;
    case companion::attach_result::timeout:
      Components::Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "Steam companion did not start in time\n");
      break;
    case companion::attach_result::invalid_arguments:
    default:
      Components::Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "Steam companion attach failed with exit code {:#x}\n", exitCode);
      break;
    }
  }
}
