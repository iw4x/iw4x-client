#pragma once

namespace Steam
{
  // Game side of the IW4x Steam companion protocol.
  //
  // Companion is a module in the Steam client that replaces what Steam
  // friends see as the game being played with the title that the game
  // registers. The game publishes a record with the app id and the title and
  // makes sure that Companion is running in the Steam client. See the
  // companion manual (deps/companion/libcompanion/doc/manual.cli) for the
  // protocol details.
  //
  // On Windows the record is a named file mapping and the game attaches
  // Companion to the Steam client with the 64-bit rundll32.exe. Under Wine
  // (Proton) the game runs with the native Linux Steam client, which cannot
  // see Windows objects, so the record is a file in /dev/shm and the
  // companion launcher starts Steam with Companion preloaded.
  //
  class Companion
  {
  public:
    // Publish the record and, on Windows, attach Companion to the Steam
    // client, all on a background thread. Companion must accept the title as
    // is (see IsValidTitle() in Companion.cpp), otherwise we report it and do
    // nothing. Only the first call has an effect.
    //
    static void Register(std::uint32_t appId, const std::string& title);

    // Stop the background thread and withdraw the record. Steam friends then
    // see the plain Steam app.
    //
    static void Unregister();

  private:
    // Background thread and the event that stops it. The mutex serializes
    // Register() and Unregister().
    //
    static std::mutex Mutex;
    static std::jthread Worker;
    static HANDLE StopEvent;

    // Published record: the file mapping and its view on Windows or the
    // record file path under Wine.
    //
    static HANDLE RecordMapping;
    static void* RecordView;
    static std::wstring UnixRecordPath;

    static void Run(std::uint32_t appId, std::string title);

    static void RunWindows(std::uint32_t appId, const std::string& title);
    static void RunWine(std::uint32_t appId, const std::string& title);

    static bool PublishRecord(std::uint32_t appId, const std::string& title);
    static bool PublishUnixRecord(std::uint32_t appId, const std::string& title);

    static void Attach(DWORD steamPid);
  };
}
