#include "FastFiles.hpp"
#include "Maps.hpp"
#include "Renderer.hpp"
#include "Sentry.hpp"

namespace Components
{
  namespace
  {
    // Return the directory of the zone group as the prefix the game puts a
    // zone file name after (zone\<group>\).
    //
    // Note that the installed zones are in the x64 layout and so we read
    // them in place. On x86 the game can only read the copies that
    // ZoneConvert converts into its own layout.
    //
    std::string ZoneDirectory(const std::string_view group)
    {
      return std::format("zone\\{}\\", group);
    }
  }

  std::vector<std::string> FastFiles::ZonePaths;

  Dvar::Var FastFiles::g_loadingInitialZones;

  bool FastFiles::StreamRead = false;

  std::atomic<unsigned int> FastFiles::CurrentZone;
  std::atomic<unsigned int> FastFiles::MaxZones;

  // This has to be called only once, when the game starts
  void FastFiles::LoadInitialZones(Game::XZoneInfo* zoneInfo, unsigned int zoneCount, int sync)
  {
    g_loadingInitialZones.set(true);

    std::vector<Game::XZoneInfo> data;
    Utils::Merge(&data, zoneInfo, zoneCount);

    if (FastFiles::Exists("iw4x_patch_mp"))
    {
      data.push_back({ "iw4x_patch_mp", 1, 0 });
    }

    if (Utils::IO::FileExists(Dvar::Var("fs_game").get<std::string>() + "/mod.ff"))
    {
      data.push_back({ "mod", 1, 0 });
    }

    return FastFiles::LoadDLCUIZones(data.data(), static_cast<unsigned int>(data.size()), sync);
  }

  // This has to be called every time the cgame is reinitialized
  void FastFiles::LoadDLCUIZones(Game::XZoneInfo* zoneInfo, unsigned int zoneCount, int sync)
  {
    std::vector<Game::XZoneInfo> data;
    Utils::Merge(&data, zoneInfo, zoneCount);

    // Take the UI zones of the map packs out of the list since the code below
    // decides whether to load them.
    //
    // Note that on x64 the game lists them with the stock UI zones, ahead of
    // ui_mp. On x86 it leaves them to us and we only load them when there is
    // no iw4x_ui_mp.
    //
    std::erase_if(data, [] (const Game::XZoneInfo& zone)
    {
      return zone.name && (!std::strcmp(zone.name, "dlc1_ui_mp") || !std::strcmp(zone.name, "dlc2_ui_mp"));
    });

    Game::XZoneInfo info = { nullptr, 2, 0 };

    // Custom ui stuff
    if (FastFiles::Exists("iw4x_ui_mp"))
    {
      info.name = "iw4x_ui_mp";
      data.push_back(info);
    }
    else // Fallback
    {
      info.name = "dlc1_ui_mp";
      data.push_back(info);

      info.name = "dlc2_ui_mp";
      data.push_back(info);
    }

    return FastFiles::LoadLocalizeZones(data.data(), static_cast<unsigned int>(data.size()), sync);
  }

  void FastFiles::LoadGfxZones(Game::XZoneInfo* zoneInfo, unsigned int zoneCount, int sync)
  {
    std::vector<Game::XZoneInfo> data;
    Utils::Merge(&data, zoneInfo, zoneCount);

    if (FastFiles::Exists("iw4x_code_post_gfx_mp"))
    {
      data.push_back({ "iw4x_code_post_gfx_mp", zoneInfo->allocFlags, zoneInfo->freeFlags });
    }

    Game::DB_LoadXAssets(data.data(), static_cast<unsigned int>(data.size()), sync);
  }

  // This has to be called every time fastfiles are loaded :D
  void FastFiles::LoadLocalizeZones(Game::XZoneInfo* zoneInfo, unsigned int zoneCount, int sync)
  {
    std::vector<Game::XZoneInfo> data;
    Utils::Merge(&data, zoneInfo, zoneCount);

    Game::XZoneInfo info = { nullptr, 4, 0 };

    // Not sure how they should be loaded :S
    std::string langZone = Utils::String::VA("iw4x_localized_%s", Game::Win_GetLanguage());

    if (FastFiles::Exists(langZone))
    {
      info.name = langZone.data();
    }
    else if (FastFiles::Exists("iw4x_localized_english")) // Fallback
    {
      info.name = "iw4x_localized_english";
    }

    data.push_back(info);

    Game::DB_LoadXAssets(data.data(), static_cast<unsigned int>(data.size()), sync);

    Scheduler::OnGameInitialized([]
    {
      g_loadingInitialZones.set(false);
    }, Scheduler::Pipeline::MAIN);
  }

  // Name is a bit weird, due to FasFileS and ExistS :P
  bool FastFiles::Exists(const std::string& file)
  {
    std::string path = FastFiles::GetZoneLocation(file.data());
    path.append(file);

    if (!Utils::String::EndsWith(path, ".ff"))
    {
      path.append(".ff");
    }

    return Utils::IO::FileExists(path);
  }

  bool FastFiles::Ready()
  {
    return (Game::Sys_IsDatabaseReady() && Game::Sys_IsDatabaseReady2());
  }

  std::optional<std::string> FastFiles::GetInstalledZoneLocation(const char* file)
  {
    std::vector<std::string> paths;
    const std::string fsGame = (*Game::fs_gameDirVar)->current.string;

    if ((file == "mod"s || file == "mod.ff"s) && !fsGame.empty())
    {
      paths.push_back(std::format("{}\\", fsGame));
    }

    Utils::Merge(&paths, FastFiles::ZonePaths);

    paths.push_back(ZoneDirectory(Game::Win_GetLanguage()));

    for (auto& path : paths)
    {
      const auto* dir = (*Game::fs_basepath)->current.string;
      auto absoluteFile = std::format("{}\\{}{}", dir, path, file);

      // No ".ff" appended, append it manually
      if (!Utils::String::EndsWith(absoluteFile, ".ff"))
      {
        absoluteFile.append(".ff");
      }

      // Check if FastFile exists
      if (Utils::IO::FileExists(absoluteFile))
      {
        return path;
      }
    }

    return std::nullopt;
  }

  bool FastFiles::IsInstalledZone(const std::string& zone)
  {
    return GetInstalledZoneLocation(zone.data()).has_value();
  }

  const char* FastFiles::GetZoneLocation(const char* file)
  {
    // See if the zone ships with the install (or the mod) first. Usermaps are
    // downloaded from remote servers, so they must never be able to shadow a
    // stock or core zone.
    //
    if (const auto path = GetInstalledZoneLocation(file))
    {
      return Utils::String::Format("{}", *path);
    }

    {
      std::string zone = file;
      if (Utils::String::EndsWith(zone, ".ff"))
      {
        Utils::String::Replace(zone, ".ff", "");
      }

      std::string filename = zone;

      if (Utils::String::EndsWith(zone, "_load"))
      {
        Utils::String::Replace(zone, "_load", "");
      }

      // Only check for usermaps on our working directory
      if (Utils::IO::FileExists(std::format("usermaps\\{}\\{}.ff", zone, filename)))
      {
        return Utils::String::Format("usermaps\\{}\\", zone);
      }
    }

    return Utils::String::Format("{}", ZoneDirectory(Game::Win_GetLanguage()));
  }

  void FastFiles::AddZonePath(const std::string& path)
  {
    FastFiles::ZonePaths.push_back(path);
  }

  std::string FastFiles::Current()
  {
    const Game::DBFile* file = *Game::g_loadFile;

    if (!file)
    {
      return "";
    }

    return file->name;
  }

  // Count the zone towards the loading progress. The game reads the version
  // once per zone, right after the magic.
  //
  // Note that on x86 we also map the versions of IW4x zones onto the stock
  // one here. On x64 we only load zones of the stock version (see
  // ReadHeaderStub() for details).
  //
  void FastFiles::ReadVersionStub(unsigned int* version, int size)
  {
    ++FastFiles::CurrentZone;
    Game::DB_ReadXFileUncompressed(version, size);
  }

  // Leave a breadcrumb with the zone we are about to load.
  //
  // Note that on x86 this is also where we recognize IW4x zones by their
  // header and set up decrypting and deobfuscating the rest of the stream.
  // On x64 our zones are in the stock unsigned format, so there is nothing
  // to recognize.
  //
  void FastFiles::ReadHeaderStub(unsigned int* header, int size)
  {
    Sentry::Leave(::Sentry::trail::fastfile, SENTRY_LEVEL_INFO, std::format("loading fastfile {}", FastFiles::Current()));

    Game::DB_ReadXFileUncompressed(header, size);
  }

  float FastFiles::GetFullLoadedFraction()
  {
    float singleProgress = 1.0f / FastFiles::MaxZones;
    float partialProgress = singleProgress * (FastFiles::CurrentZone - 1);
    float currentProgress = std::max(std::min(Game::DB_GetLoadedFraction(), 1.0f), 0.0f);
    return std::min(partialProgress + (currentProgress * singleProgress), 1.0f);
  }

  // Start counting the zones of a load.
  //
  // Note that x64 inlines DB_LoadXZone(), whose call we hook on x86, into
  // DB_LoadXAssets(), so here we hook the Sys_WakeDatabase() call it ends
  // with. By then it has copied the zones for the database thread and so we
  // take their count from there, which also leaves out the entries without
  // a name that it skips.
  //
  void FastFiles::LoadZonesStub()
  {
    FastFiles::CurrentZone = 0;
    FastFiles::MaxZones = *Game::g_zoneInfoCount;

    Utils::Hook::Call<void()>(0x140208F20)(); // Sys_WakeDatabase
  }

#ifdef DEBUG
  void FastFiles::LogStreamRead(int len)
  {
    *Game::g_streamPos += len;

    if (FastFiles::StreamRead)
    {
      std::string data = Utils::String::VA("%d\n", len);
      if (*Game::g_streamPosIndex == 2) data = Utils::String::VA("(%d)\n", len);
      Utils::IO::WriteFile("userraw/logs/iw4_reads.log", data, true);
    }
  }
#endif

  void FastFiles::DB_BuildOSPath_FromSource_Default(const char* zoneName, Game::FF_DIR source, unsigned int size, char* filename)
  {
    // TODO: this is where user map and mod.ff check should happen
    if (source == Game::FFD_DEFAULT)
    {
      (void)sprintf_s(filename, size, "%s\\%s%s.ff", FileSystem::Sys_DefaultInstallPath_Hk(), GetZoneLocation(zoneName), zoneName);
    }
  }

  void FastFiles::DB_BuildOSPath_FromSource_Custom(const char* zoneName, Game::FF_DIR source, unsigned int size, char* filename)
  {
    // TODO: this is where user map and mod.ff check should happen
    if (source == Game::FFD_DEFAULT)
    {
      (void)sprintf_s(filename, size, "%s\\%s%s.ff", FileSystem::Sys_HomePath_Hk(), GetZoneLocation(zoneName), zoneName);
    }
  }

  bool FastFiles::DB_FileExists_Hk(const char* zoneName, Game::FF_DIR source)
  {
    char filename[256]{};

    DB_BuildOSPath_FromSource_Default(zoneName, source, sizeof(filename), filename);
    if (auto zoneFile = Game::Sys_OpenFileReliable(filename); zoneFile != INVALID_HANDLE_VALUE)
    {
      CloseHandle(zoneFile);
      return true;
    }

    DB_BuildOSPath_FromSource_Custom(zoneName, source, sizeof(filename), filename);
    if (auto zoneFile = Game::Sys_OpenFileReliable(filename); zoneFile != INVALID_HANDLE_VALUE)
    {
      CloseHandle(zoneFile);
      return true;
    }

    return false;
  }

  // Explain to the player that a zone the game can't do without is missing
  // and exit. This replaces the zone loader's Sys_Error() call.
  //
  // There are three cases we've seen. If the zone is a map from a map pack
  // the player doesn't have, we say so the same way as when they try to join
  // a server running it. If we are still loading the initial zones, then
  // most likely IW4x is not in the game's directory, for example, it was
  // started from inside the launcher's archive, from a copy of our own files,
  // or from a directory without zone/. Otherwise, the game files are probably
  // incomplete. In all three cases the engine's error only names the zone,
  // which doesn't help the player much.
  //
  // Note that we don't go through Sys_Error() since none of these is a bug
  // and there is no point in asking the player to report it.
  //
  void FastFiles::Sys_Error_MissingZone (const char*, const char* zone)
  {
    const std::string file (std::format ("zone\\{}\\{}", Game::Win_GetLanguage (), zone));

    std::string name (zone);
    if (Utils::String::EndsWith (name, ".ff"))
      name.resize (name.size () - 3);

    std::string text;
    if (const auto e = Maps::GetMissingDlcError (name))
    {
      text = *e;
    }
    else if (g_loadingInitialZones.get<bool> ())
    {
      text = std::format ("Could not find the game file '{}' in '{}'.\n\n"
                          "IW4x must be installed in your Call of Duty: Modern "
                          "Warfare 2 folder, the one with the 'main' and "
                          "'zone' folders. If you started it from inside a zip "
                          "archive, extract the archive first.",
                          file,
                          FileSystem::Sys_DefaultInstallPath_Hk ());
    }
    else
    {
      text = std::format ("Could not find the game file '{}'. Your game files "
                          "are likely incomplete.",
                          file);
    }

    Logger::Print ("{}\n", text);
    MessageBoxA (GetActiveWindow (), text.c_str (), "Error", MB_ICONERROR);

    TerminateProcess (GetCurrentProcess (), EXIT_FAILURE);
  }

  // Say which zone the game failed to read. This replaces the Sys_Error()
  // call of the engine's dirty disc handler, which only says "Disc read
  // error.".
  //
  // The handler does print the zone but on the database thread, which
  // doesn't make it into the console log before the error. So all we've
  // seen in reports is the message, with the game loading anything from
  // common_mp to a map. Most likely the file is damaged but, until we know
  // which ones fail, we still let the player report it.
  //
  // Note also that on x64 the print is compiled out and the handler is
  // inlined into most of its callers, and so we hook the Sys_Error() call of
  // each copy (see the constructor).
  //
  void FastFiles::Sys_Error_DirtyDisc (const char*)
  {
    const char* zone ((*Game::g_loadFile)->name);

    Game::Sys_Error ("Could not read the game file '%s.ff'. It is probably "
                     "damaged, so verify your game files or reinstall them.",
                     zone);
  }

  // Note that on x86 we also look for the zone in the directories of the
  // zone builder, which isn't part of the x64 build.
  //
  Game::Sys_File FastFiles::Sys_CreateFile_Stub(const char* dir, const char* filename)
  {
    static_assert(sizeof(Game::Sys_File) == sizeof(HANDLE));

    auto file = Game::Sys_CreateFile(dir, filename);

    static const std::filesystem::path home = FileSystem::Sys_HomePath_Hk();
    if (file.handle == INVALID_HANDLE_VALUE && !home.empty())
    {
      file.handle = Game::Sys_OpenFileReliable(Utils::String::VA("%s\\%s%s", FileSystem::Sys_HomePath_Hk(), dir, filename));
    }

    return file;
  }

  // Give the zone list of DB_LoadXAssets() room for as many zones as the
  // game can have loaded at once.
  //
  // DB_LoadXAssets() copies each zone it loads into this list, which the
  // database thread then loads them from, and nothing checks that they fit.
  // On x64 the list has room for 8 zones with their count stored right
  // after it, so the startup load with our zones added writes past its end
  // and the count lands on the name of the ninth zone. The game refuses to
  // have more than 32 zones loaded, so a list of 32 holds any load it
  // accepts.
  //
  void FastFiles::ReallocateZoneList()
  {
    constexpr uintptr_t list = 0x1415FA9F0;
    constexpr size_t zoneSize = 0x44;
    constexpr size_t zoneCount = 32;

    const uintptr_t to = reinterpret_cast<uintptr_t>(Utils::Hook::AllocateNear(0x14012F0A1, zoneSize * zoneCount));

    Utils::Hook::Displace(0x14012F0A1, 7, list, to);               // DB_LoadXAssets(), names
    Utils::Hook::Displace(0x14012F9E9, 7, list, to);               // DB_Thread(), names
    Utils::Hook::Displace(0x14012F9E2, 7, list + 0x40, to + 0x40); // DB_Thread(), flags
  }

  FastFiles::FastFiles()
  {
    Dvar::Register<bool>("ui_zoneDebug", false, Game::DVAR_ARCHIVE, "Display current loaded zone.");
    g_loadingInitialZones = Dvar::Register<bool>("g_loadingInitialZones", true, Game::DVAR_NONE, "Is loading initial zones");

    FastFiles::ReallocateZoneList();

    Utils::Hook(0x14012FA7B, Sys_CreateFile_Stub, HOOK_CALL).install()->quick();
    Utils::Hook (0x14012FAFE, Sys_Error_MissingZone, HOOK_CALL).install ()->quick ();

    // Note that two of the copies of DB_DirtyDiscError() end with a jump to
    // Sys_Error() (see Sys_Error_DirtyDisc() for details).
    //
    Utils::Hook (0x140117FE7, Sys_Error_DirtyDisc, HOOK_JUMP).install ()->quick ();
    Utils::Hook (0x1401180C8, Sys_Error_DirtyDisc, HOOK_JUMP).install ()->quick ();
    Utils::Hook (0x140118316, Sys_Error_DirtyDisc, HOOK_CALL).install ()->quick ();
    Utils::Hook (0x1401184BF, Sys_Error_DirtyDisc, HOOK_CALL).install ()->quick ();
    Utils::Hook (0x1401181EE, Sys_Error_DirtyDisc, HOOK_CALL).install ()->quick ();

    Utils::Hook(0x14012D3A0, DB_FileExists_Hk, HOOK_JUMP).install()->quick();

    // Load_XModelSurfs takes the count it passes to Load_XSurfaceArray from
    // varXModelLodInfo->numsurfs. That is valid when the surfs are loaded inline
    // under an XModel, where Load_XModelLodInfoArray points varXModelLodInfo at the current LOD.
    //
    // A top-level XModelSurfs asset reaches this loader without setting varXModelLodInfo
    // first, so the pointer is whatever the last XModel load left behind. That XModel may
    // belong to a zone that has since been unloaded and decommitted, making the read fault.
    // Where it doesn't fault, it yields another model's surface count.
    //
    // Take the count from the asset being loaded instead, varXModelSurfs->numsurfs,
    // which is valid on both paths.
    //
    // Note that on x64 the count is at 0x10.
    //
    AssertOffset(Game::XModelSurfs, numsurfs, 0x10);
    Utils::Hook::Displace(0x140123945, 7, 0x140DB5180, 0x140DB5170); // mov rax, varXModelSurfs
    Utils::Hook::Set<BYTE>(0x14012394C + 3, 0x10);                  // movzx edx, word ptr [rax+numsurfs]

    // Redirect zone paths.
    //
    // Note that x64 looks up the directory of the playlist zone with a copy
    // of DB_GetZoneLocation() that takes no name, so we leave that one
    // alone. We don't ship the playlist zone and for a zone we don't have we
    // return the language directory, the same as the game.
    //
    Utils::Hook(0x1402A3910, FastFiles::GetZoneLocation, HOOK_JUMP).install()->quick();

    Utils::Hook(0x140117D80, FastFiles::ReadVersionStub, HOOK_CALL).install()->quick();
    Utils::Hook(0x140117D46, FastFiles::ReadHeaderStub, HOOK_CALL).install()->quick();

    // Allow custom zone loading
    Utils::Hook(0x140030D70, FastFiles::LoadInitialZones, HOOK_CALL).install()->quick();
    Utils::Hook(0x1401F335E, FastFiles::LoadDLCUIZones, HOOK_CALL).install()->quick();
    Utils::Hook(0x140030C84, FastFiles::LoadGfxZones, HOOK_CALL).install()->quick();

    // Allow loading of unsigned zones, which is the format our zones are in.
    //
    // DB_LoadXFileInternal() accepts the unsigned magic and passes on that
    // the zone isn't signed. DB_AuthLoad_InflateInit() can then set up a
    // plain inflate stream for it but first fails every such zone as not
    // supported, so we drop the jump to the failure.
    //
    // Note that on x86 we also accept any header and skip the hash checks of
    // signed zones, both of which only IW4x zones need.
    //
    Utils::Hook::Nop(0x140118066, 2);

    // Fix fastfile progress.
    //
    // Note that x64 inlines the two small x86 functions whose
    // DB_GetLoadedFraction() call we hook there, so here we hook the calls
    // where they ended up.
    //
    Utils::Hook(0x14012F101, FastFiles::LoadZonesStub, HOOK_CALL).install()->quick();
    Utils::Hook(0x140109A3D, FastFiles::GetFullLoadedFraction, HOOK_CALL).install()->quick();
    Utils::Hook(0x14026CE92, FastFiles::GetFullLoadedFraction, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400B2991, FastFiles::GetFullLoadedFraction, HOOK_CALL).install()->quick();
    Utils::Hook(0x14024F565, FastFiles::GetFullLoadedFraction, HOOK_CALL).install()->quick();

    // Add custom zone paths
    FastFiles::AddZonePath(ZoneDirectory("patch"));
    FastFiles::AddZonePath(ZoneDirectory("dlc"));

    if (!Dedicated::IsEnabled())
    {
      Scheduler::Loop([]
      {
        if (FastFiles::Current().empty() || !Dvar::Var("ui_zoneDebug").get<bool>()) return;

        auto* const font = Game::R_RegisterFont("fonts/consoleFont", 0);
        float color[4] = { 1.0f, 1.0f, 1.0f, (Game::CL_IsCgameInitialized() ? 0.3f : 1.0f) };

        auto FFTotalSize = *reinterpret_cast<std::uint32_t*>(0x140D34590);
        auto FFCurrentOffset = *reinterpret_cast<std::uint32_t*>(0x140D34600);

        float fastfileLoadProgress = (float(FFCurrentOffset) / float(FFTotalSize)) * 100.0f;
        if (std::isinf(fastfileLoadProgress))
        {
          fastfileLoadProgress = 100.0f;
        }
        else if (std::isnan(fastfileLoadProgress))
        {
          fastfileLoadProgress = 0.0f;
        }

        Game::R_AddCmdDrawText(Utils::String::VA("Loading FastFile: %s [%0.1f%%]", FastFiles::Current().data(), fastfileLoadProgress), std::numeric_limits<int>::max(), font, 5.0f, static_cast<float>(Renderer::Height() - 5), 1.0f, 1.0f, 0.0f, color, Game::ITEM_TEXTSTYLE_NORMAL);
      }, Scheduler::Pipeline::RENDERER);
    }

    Command::Add("loadzone", [](const Command::Params* params)
    {
      if (params->size() < 2) return;

      const auto* zoneName = params->get(1);
      if (!FastFiles::Exists(zoneName))
      {
        Logger::PrintError(Game::CON_CHANNEL_ERROR, "Zone '{}' does not exist\n", zoneName);
        return;
      }

      Game::XZoneInfo info;
      info.name = zoneName;
      info.allocFlags = 1;//0x01000000;
      info.freeFlags = 0;

      Game::DB_LoadXAssets(&info, 1, true);
    });

    Command::Add("listassetpool", [](const Command::Params* params)
    {
      auto first = 0;
      auto last = Game::ASSET_TYPE_COUNT - 1;

      if (params->size() >= 2)
      {
        const auto type = Game::DB_GetXAssetNameType(params->get(1));
        if (type == Game::ASSET_TYPE_INVALID)
        {
          Logger::PrintError(Game::CON_CHANNEL_ERROR, "Invalid asset type '{}'\n", params->get(1));
          return;
        }

        first = last = type;
      }

      for (auto i = first; i <= last; ++i)
      {
        const auto type = static_cast<Game::XAssetType>(i);
        auto count = 0u;

        Game::DB_EnumXAssets(type, [](Game::XAssetHeader, void* data)
        {
          ++*static_cast<unsigned int*>(data);
        }, &count, false);

        Logger::Print("{}: {} / {}\n", Game::DB_GetXAssetTypeName(type), count, Game::g_poolSize[type]);
      }
    });

    Command::Add("awaitDatabase", []()
    {
      Logger::Print("Waiting for database...\n");
      while (!Game::Sys_IsDatabaseReady()) std::this_thread::sleep_for(100ms);
    });
  }
}
