#pragma once

namespace Components
{
  class FastFiles : public Component
  {
  public:
    FastFiles();

    static void AddZonePath(const std::string& path);
    static std::string Current();
    static bool Ready();
    static bool Exists(const std::string& file);
    // Return true if zone ships with the install (or the current mod), in
    // which case a usermap must never be allowed to take its name.
    //
    static bool IsInstalledZone(const std::string& zone);

    static void LoadLocalizeZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);

    static float GetFullLoadedFraction();

  private:
    static std::atomic<unsigned int> CurrentZone;
    static std::atomic<unsigned int> MaxZones;

    static bool StreamRead;

    static Dvar::Var g_loadingInitialZones;

    static std::vector<std::string> ZonePaths;
    static std::optional<std::string> GetInstalledZoneLocation(const char* file);
    static const char* GetZoneLocation(const char* file);
    static void LoadInitialZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);
    static void LoadDLCUIZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);
    static void LoadGfxZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);

    static void ReadHeaderStub(unsigned int* header, int size);
    static void ReadVersionStub(unsigned int* version, int size);

    static void LoadZonesStub();

#ifdef DEBUG
    static void LogStreamRead(int len);
#endif


    static void DB_BuildOSPath_FromSource_Default(const char* zoneName, Game::FF_DIR source, unsigned int size, char* filename);
    static void DB_BuildOSPath_FromSource_Custom(const char* zoneName, Game::FF_DIR source, unsigned int size, char* filename);
    static Game::Sys_File Sys_CreateFile_Stub(const char* dir, const char* filename);

    static void Sys_Error_MissingZone (const char* format, const char* zone);
    static void Sys_Error_DirtyDisc (const char* message);
    static bool DB_FileExists_Hk(const char* zoneName, Game::FF_DIR source);

    static void ReallocateZoneList();
  };
}
