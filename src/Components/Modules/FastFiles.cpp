#include "FastFiles.hpp"
#include "Maps.hpp"
#include "Renderer.hpp"
#include "Sentry.hpp"

namespace Components
{
  namespace
  {
    // Return the directory of a zone group the way the game puts a zone's file
    // name after it (zone\<group>\).
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

    // Take the UI zones of the map packs out of the list, since we decide
    // below whether to load them. x64 lists them itself, along with the stock
    // UI zones and ahead of ui_mp, while x86 leaves them to us. Presumably
    // the later revision just took the map packs for granted. But we only
    // want them if there is no iw4x_ui_mp, so on x64 we have to undo the
    // game's choice first.
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

  int FastFiles::ModZoneIndex()
  {
    const auto index = Game::DB_GetZoneIndex("mod");
    return index > 0 ? index : -1;
  }

  Game::XAssetHeader FastFiles::FindModZoneAsset(const Game::XAssetType type, const char* name)
  {
    Game::XAssetHeader header{};

    const auto modZoneIndex = ModZoneIndex();

    if (modZoneIndex < 0)
    {
      return header;
    }

    Game::Sys_LockRead(Game::db_hashCritSect);

    // Note that the entry we find may be the one of a zone that overrides
    // the mod's (one of a higher priority, or one that loads after it), so we
    // follow the overrides as well.
    //
    for (auto* entry = Game::DB_FindXAssetEntry(type, name); entry;
      entry = entry->nextOverride ? &Game::g_assetEntryPool[entry->nextOverride] : nullptr)
    {
      if (static_cast<unsigned char>(entry->zoneIndex) == modZoneIndex)
      {
        header = entry->asset.header;
        break;
      }
    }

    Game::Sys_UnlockRead(Game::db_hashCritSect);

    return header;
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
  // once per zone, right after the magic, which makes it a handy place for
  // that.
  //
  // Note that on x86 we also map the versions of our zones onto the stock
  // one here. On x64 we only ever load zones of the stock version (see
  // ReadHeaderStub()), so there is nothing to map.
  //
  void FastFiles::ReadVersionStub(unsigned int* version, int size)
  {
    ++FastFiles::CurrentZone;
    Game::DB_ReadXFileUncompressed(version, size);
  }

  // Leave a breadcrumb with the zone being loaded, so that a crash report
  // says which one it was.
  //
  // Note that on x86 this is also where we recognize our zones by their
  // header and set up decrypting the rest of the stream. On x64 our zones
  // are converted into the stock unsigned format before the game ever sees
  // them (see ZoneConvert), so there is nothing to recognize.
  //
  void FastFiles::ReadHeaderStub(unsigned int* header, int size)
  {
    Sentry::Leave(::Sentry::trail::fastfile, SENTRY_LEVEL_INFO, std::format("loading fastfile {}", FastFiles::Current()));

    Game::DB_ReadXFileUncompressed(header, size);
  }

  // Return the progress of the whole load rather than of the current zone,
  // which is all DB_GetLoadedFraction() knows about.
  //
  float FastFiles::GetFullLoadedFraction()
  {
    float singleProgress = 1.0f / FastFiles::MaxZones;
    float partialProgress = singleProgress * (FastFiles::CurrentZone - 1);
    float currentProgress = std::max(std::min(Game::DB_GetLoadedFraction(), 1.0f), 0.0f);
    return std::min(partialProgress + (currentProgress * singleProgress), 1.0f);
  }

  // Start counting the zones of a load.
  //
  // On x86 we hook the call of DB_LoadXZone(), but x64 inlines it into
  // DB_LoadXAssets(). So instead we hook the Sys_WakeDatabase() call that
  // DB_LoadXAssets() ends with. It feels a bit indirect, but by then it has
  // copied the zones over for the database thread and we can just take
  // their count from there, which, as a bonus, leaves out the nameless
  // entries it skips.
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
  void FastFiles::Sys_Error_DirtyDisc (const char*)
  {
    const char* zone ((*Game::g_loadFile)->name);

    Game::Sys_Error ("Could not read the game file '%s.ff'. It is probably "
                     "damaged, so verify your game files or reinstall them.",
                     zone);
  }

  // If the zone isn't in the install, then try the home path, the same as
  // DB_FileExists_Hk() does.
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

  // Move the list of zones to load somewhere with room for 32 of them.
  //
  // DB_LoadXAssets() copies each zone it's asked to load into this list,
  // which the database thread then loads them from, and nothing checks that
  // they fit. On x64 the list only has room for 8, with their count right
  // after it. So the startup load, with our zones added, writes past the
  // end and the count ends up on top of the name of the ninth zone, which
  // makes for some rather confusing errors. Since the game refuses to have
  // more than 32 zones loaded at once, a list of 32 should hold any load it
  // would accept.
  //
  void FastFiles::ReallocateZoneList()
  {
    constexpr uintptr_t list = 0x1415FA9F0;
    constexpr size_t zoneSize = 0x44;
    constexpr size_t zoneCount = 32;

    const uintptr_t to = reinterpret_cast<uintptr_t>(Utils::Hook::AllocateNear(0x14012F0A1, zoneSize * zoneCount));

    Utils::Hook::Displace(0x14012F0A1, 7, list, to); // DB_LoadXAssets(), names
    Utils::Hook::Displace(0x14012F9E9, 7, list, to); // DB_Thread(), names
    Utils::Hook::Displace(0x14012F9E2, 7, list + 0x40, to + 0x40); // DB_Thread(), flags
  }

  FastFiles::FastFiles()
  {
    Dvar::Register<bool>("ui_zoneDebug", false, Game::DVAR_ARCHIVE, "Display current loaded zone.");
    g_loadingInitialZones = Dvar::Register<bool>("g_loadingInitialZones", true, Game::DVAR_NONE, "Is loading initial zones");

    FastFiles::ReallocateZoneList();

    Utils::Hook(0x14012FA7B, Sys_CreateFile_Stub, HOOK_CALL).install()->quick();
    Utils::Hook (0x14012FAFE, Sys_Error_MissingZone, HOOK_CALL).install ()->quick ();

    // Note that two of the copies of the dirty disc handler end with a jump to
    // Sys_Error() rather than a call.
    //
    Utils::Hook (0x140117FE7, Sys_Error_DirtyDisc, HOOK_JUMP).install ()->quick ();
    Utils::Hook (0x1401180C8, Sys_Error_DirtyDisc, HOOK_JUMP).install ()->quick ();
    Utils::Hook (0x140118316, Sys_Error_DirtyDisc, HOOK_CALL).install ()->quick ();
    Utils::Hook (0x1401184BF, Sys_Error_DirtyDisc, HOOK_CALL).install ()->quick ();
    Utils::Hook (0x1401181EE, Sys_Error_DirtyDisc, HOOK_CALL).install ()->quick ();

    Utils::Hook(0x14012D3A0, DB_FileExists_Hk, HOOK_JUMP).install()->quick();

    // A top-level XModelSurfs asset reaches this loader without setting varXModelLodInfo
    // first, so the pointer is whatever the last XModel load left behind. That XModel may
    // belong to a zone that has since been unloaded and decommitted, making the read fault.
    // Where it doesn't fault, it yields another model's surface count.
    //
    // So we take the count from the asset being loaded instead,
    // varXModelSurfs->numsurfs, which is valid either way. Note that on x64
    // it's at 0x10 rather than 8, since the two pointers before it are 8
    // bytes each.
    //
    AssertOffset(Game::XModelSurfs, numsurfs, 0x10);
    Utils::Hook::Displace(0x140123945, 7, 0x140DB5180, 0x140DB5170); // mov rax, varXModelSurfs
    Utils::Hook::Set<BYTE>(0x14012394C + 3, 0x10);                  // movzx edx, word ptr [rax+numsurfs]

    // Redirect zone paths
    //
    // Note that x64 looks up the directory of the playlist zone with a second
    // copy of DB_GetZoneLocation() that takes no name, which we leave alone.
    // Should we hook it too? Probably not: we don't ship a playlist zone, and
    // for a zone we don't have we would return the language directory
    // anyway, the same as the game does.
    //
    Utils::Hook(0x1402A3910, FastFiles::GetZoneLocation, HOOK_JUMP).install()->quick();

    Utils::Hook(0x140117D80, FastFiles::ReadVersionStub, HOOK_CALL).install()->quick();
    Utils::Hook(0x140117D46, FastFiles::ReadHeaderStub, HOOK_CALL).install()->quick();

    // Allow custom zone loading
    Utils::Hook(0x140030D70, FastFiles::LoadInitialZones, HOOK_CALL).install()->quick();
    Utils::Hook(0x1401F335E, FastFiles::LoadDLCUIZones, HOOK_CALL).install()->quick();
    Utils::Hook(0x140030C84, FastFiles::LoadGfxZones, HOOK_CALL).install()->quick();

    // Allow loading unsigned zones, which is what ours are converted into.
    //
    // DB_LoadXFileInternal() accepts the unsigned magic and passes on that the
    // zone isn't signed, and DB_AuthLoad_InflateInit() could then set up a
    // plain inflate stream for it. But before that it fails every such zone as
    // not supported. That looks like a leftover from development (why else
    // accept the magic?), and dropping the jump to the failure is all it
    // takes.
    //
    // Note that on x86 we also accept any header and skip the hash checks of
    // signed zones, but those are only needed for our zones in the x86
    // format.
    //
    Utils::Hook::Nop(0x140118066, 2);

    // Fix fastfile progress
    //
    // Note that x64 inlines the two small x86 functions whose
    // DB_GetLoadedFraction() calls we hook there, so here we hook those calls
    // wherever they ended up.
    //
    Utils::Hook(0x14012F101, FastFiles::LoadZonesStub, HOOK_CALL).install()->quick();
    Utils::Hook(0x140109A3D, FastFiles::GetFullLoadedFraction, HOOK_CALL).install()->quick();
    Utils::Hook(0x14026CE92, FastFiles::GetFullLoadedFraction, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400B2991, FastFiles::GetFullLoadedFraction, HOOK_CALL).install()->quick();
    Utils::Hook(0x14024F565, FastFiles::GetFullLoadedFraction, HOOK_CALL).install()->quick();

    // The installed zones are in the x64 layout, so we can read those in
    // place. Our own zones, on the other hand, are in the x86 layout, so for
    // those we read the copies that ZoneConvert makes (on x86 it's the other
    // way around, and there the game can only read converted copies of the
    // installed zones). Steam installs its map packs into dlc in the x64
    // layout, so if there are no converted copies of those, we fall back to
    // reading them in place.
    //
    FastFiles::AddZonePath(ZoneConvert::SearchPath("patch"));
    FastFiles::AddZonePath(ZoneConvert::SearchPath("dlc"));
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
