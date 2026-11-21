#include "Maps.hpp"
#include "ArenaLength.hpp"
#include "AssetHandler.hpp"
#include "Events.hpp"
#include "FastFiles.hpp"
#include "RawFiles.hpp"
#include "StartupMessages.hpp"
#include "Theatre.hpp"
#include "GSC/Script.hpp"

extern "C"
{
  int Maps_RotateConnectionState;
}

namespace Components
{
  Maps::UserMapContainer Maps::UserMap;
  std::string Maps::CurrentMainZone;
  std::vector<std::pair<std::string, std::string>> Maps::DependencyList;
  std::vector<std::string> Maps::CurrentDependencies;
  std::vector<std::string> Maps::FoundCustomMaps;

  Dvar::Var Maps::RListSModels;

  bool Maps::SPMap;
  std::vector<Maps::DLC> Maps::DlcPacks;

  void (*Maps::SV_SpawnServer_Original)(const char*, int, int, int, int);

  const char* Maps::UserMapFiles[4] =
  {
    ".ff",
    "_load.ff",
    ".iwd",
    ".arena",
  };

  Maps::UserMapContainer* Maps::GetUserMap()
  {
    return &Maps::UserMap;
  }

  void Maps::UserMapContainer::loadIwd()
  {
    if (this->isValid() && !this->searchPath.iwd)
    {
      auto iwdName = std::format("{}.iwd", this->mapname);
      auto path = std::format("{}\\usermaps\\{}\\{}", (*Game::fs_basepath)->current.string, this->mapname, iwdName);

      if (Utils::IO::FileExists(path))
      {
        this->searchPath.iwd = Game::FS_IsShippedIWD(path.data(), iwdName.data());

        if (this->searchPath.iwd)
        {
          this->searchPath.bLocalized = false;
          this->searchPath.ignore = 0;
          this->searchPath.ignorePureCheck = 0;
          this->searchPath.language = 0;
          this->searchPath.dir = nullptr;
          this->searchPath.next = *Game::fs_searchpaths;
          *Game::fs_searchpaths = &this->searchPath;
        }
      }
    }
  }

  void Maps::UserMapContainer::reloadIwd()
  {
    if (this->isValid() && this->wasFreed)
    {
      this->wasFreed = false;
      this->searchPath.iwd = nullptr;
      this->loadIwd();
    }
  }

  void Maps::UserMapContainer::handlePackfile(void* packfile)
  {
    if (this->isValid() && this->searchPath.iwd == packfile)
    {
      this->wasFreed = true;
    }
  }

  void Maps::UserMapContainer::freeIwd()
  {
    if (this->isValid() && this->searchPath.iwd && !this->wasFreed)
    {
      this->wasFreed = true;

      // Unchain our searchpath
      for (auto** pathPtr = Game::fs_searchpaths; *pathPtr; pathPtr = &(*pathPtr)->next)
      {
        if (*pathPtr == &this->searchPath)
        {
          *pathPtr = (*pathPtr)->next;
          break;
        }
      }

      Game::unzClose(this->searchPath.iwd->handle);

      // Use game's free function
      Utils::Hook::Call<void(void*)>(0x14027DD10)(this->searchPath.iwd->buildBuffer); // free
      Utils::Hook::Call<void(void*)>(0x14027DD10)(this->searchPath.iwd); // free

      ZeroMemory(&this->searchPath, sizeof(this->searchPath));
    }
  }

  const char* Maps::LoadArenaFileStub(const char* name, char* buffer, int size)
  {
    std::string  data;

    if (Maps::UserMap.isValid())
    {
      const auto mapname = Maps::UserMap.getName();
      const auto arena = GetArenaPath(mapname);

      if (Utils::IO::FileExists(arena))
      {
        // Replace all arenas with just this one
        data = Utils::IO::ReadFile(arena);
      }
    }
    else
    {
      data = RawFiles::ReadRawFile(name, buffer, size);
    }

    strncpy_s(buffer, size, data.data(), _TRUNCATE);
    return buffer;
  }

  void Maps::UnloadMapZones(Game::XZoneInfo* zoneInfo, unsigned int zoneCount, int sync)
  {
    Game::DB_LoadXAssets(zoneInfo, zoneCount, sync);

    if (Maps::UserMap.isValid())
    {
      Maps::UserMap.freeIwd();
      Maps::UserMap.clear();
    }
  }

  void Maps::LoadMapZones(Game::XZoneInfo* zoneInfo, unsigned int zoneCount, int sync)
  {
    if (!zoneInfo) return;

    Maps::SPMap = false;
    Maps::CurrentMainZone = zoneInfo->name;
    Maps::CurrentDependencies.clear();

    auto dependencies = GetDependenciesForMap(zoneInfo->name);

    std::vector<Game::XZoneInfo> data;
    Utils::Merge(&data, zoneInfo, zoneCount);
    Utils::Memory::Allocator allocator;

    if (dependencies.requiresTeamZones)
    {
      auto teams = dependencies.requiredTeams;

      Game::XZoneInfo team;
      team.allocFlags = zoneInfo->allocFlags;
      team.freeFlags = zoneInfo->freeFlags;

      team.name = allocator.duplicateString(std::format("iw4x_team_{}", teams.first));
      data.push_back(team);

      team.name = allocator.duplicateString(std::format("iw4x_team_{}", teams.second));
      data.push_back(team);
    }

    Utils::Merge(&Maps::CurrentDependencies, dependencies.requiredMaps.data(), dependencies.requiredMaps.size());
    for (unsigned int i = 0; i < Maps::CurrentDependencies.size(); ++i)
    {
      Game::XZoneInfo info;

      info.name = (&Maps::CurrentDependencies[i])->data();
      info.allocFlags = zoneInfo->allocFlags;
      info.freeFlags = zoneInfo->freeFlags;

      data.push_back(info);
    }

    // Load patch files
    auto patchZone = std::format("patch_{}", zoneInfo->name);
    if (FastFiles::Exists(patchZone))
    {
      data.push_back({ patchZone.data(), zoneInfo->allocFlags, zoneInfo->freeFlags });
    }

    return FastFiles::LoadLocalizeZones(data.data(), static_cast<unsigned int>(data.size()), sync);
  }

  void Maps::OverrideMapEnts(Game::MapEnts* ents)
  {
    auto callback = [](Game::XAssetHeader header, void* ents)
      {
        Game::MapEnts* mapEnts = reinterpret_cast<Game::MapEnts*>(ents);
        Game::clipMap_t* clipMap = header.clipMap;

        if (clipMap && mapEnts && !_stricmp(mapEnts->name, clipMap->name))
        {
          clipMap->mapEnts = mapEnts;
          //*Game::marMapEntsPtr = mapEnts;
          //Game::G_SpawnEntitiesFromString();
        }
      };

    // Internal doesn't lock the thread, as locking is impossible, due to executing this in the thread that holds the current lock
    Game::DB_EnumXAssets_Internal(Game::XAssetType::ASSET_TYPE_CLIPMAP_MP, callback, ents, true);
    Game::DB_EnumXAssets_Internal(Game::XAssetType::ASSET_TYPE_CLIPMAP_SP, callback, ents, true);
  }

  void Maps::LoadAssetRestrict(Game::XAssetType type, Game::XAssetHeader asset, const std::string& name, bool* restrict)
  {
    if (std::ranges::find(Maps::CurrentDependencies, FastFiles::Current()) != Maps::CurrentDependencies.end()) // Shipment is a special case
    {
      switch (type)
      {
      case Game::XAssetType::ASSET_TYPE_CLIPMAP_MP:
      case Game::XAssetType::ASSET_TYPE_CLIPMAP_SP:
      case Game::XAssetType::ASSET_TYPE_GAMEWORLD_SP:
      case Game::XAssetType::ASSET_TYPE_GAMEWORLD_MP:
      case Game::XAssetType::ASSET_TYPE_GFXWORLD:
      case Game::XAssetType::ASSET_TYPE_MAP_ENTS:
      case Game::XAssetType::ASSET_TYPE_COMWORLD:
      case Game::XAssetType::ASSET_TYPE_FXWORLD:
        *restrict = true;
        return;

      default:
        break;
      }
    }

    if (type == Game::XAssetType::ASSET_TYPE_ADDON_MAP_ENTS)
    {
      *restrict = true;
      return;
    }

    // Note that on x86 we also restrict all the weapons of IW4x zones from
    // the alpha 2 format on, which x64 doesn't load.
    //
    if (type == Game::XAssetType::ASSET_TYPE_WEAPON)
    {
      if (!strstr(name.data(), "_mp") && name != "none" && name != "destructible_car")
      {
        *restrict = true;
        return;
      }
    }

    if (type == Game::XAssetType::ASSET_TYPE_STRINGTABLE)
    {
      if (FastFiles::Current() == "mp_cross_fire")
      {
        *restrict = true;
        return;
      }
    }

    if (type == Game::XAssetType::ASSET_TYPE_MAP_ENTS)
    {
      std::string entitiesFilename = name;

      if (entitiesFilename.starts_with("maps/mp/maps/mp") && entitiesFilename.ends_with(".d3dbsp.d3dbsp"))
      {
        // As of 2026 some MW3 maps have been built with this strange clipmap name, likely an artifact of IW5xport.
        // It is not problematic in itself but it breaks overrides via ents files
        // We can hotfix it here until one day where we re-build and re-ship every MW3 maps with additional fixes
        const auto prefixLength = "maps/mp/"s.size();
        const auto suffixLength = ".d3dbsp"s.size();
        entitiesFilename = entitiesFilename.substr(prefixLength, entitiesFilename.size() - prefixLength - suffixLength);
      }

      static std::string mapEntities;
      FileSystem::File ents(entitiesFilename + ".ents", Game::FS_THREAD_DATABASE);
      if (ents.exists())
      {
        mapEntities = ents.getBuffer();
        asset.mapEnts->entityString = mapEntities.data();
        asset.mapEnts->numEntityChars = static_cast<int>(mapEntities.size() + 1);
      }
    }

    // Note that on x86 we also restrict the menus of IW4x zones from format
    // version 359 on, which x64 doesn't load.
    //
  }

  Game::G_GlassData* Maps::GetWorldData()
  {
    Logger::Print("Waiting for database...\n");
    while (!Game::Sys_IsDatabaseReady()) std::this_thread::sleep_for(10ms);

    if (!Game::DB_XAssetPool[Game::XAssetType::ASSET_TYPE_GAMEWORLD_MP].gameWorldMp || !Game::DB_XAssetPool[Game::XAssetType::ASSET_TYPE_GAMEWORLD_MP].gameWorldMp->name ||
      !Game::DB_XAssetPool[Game::XAssetType::ASSET_TYPE_GAMEWORLD_MP].gameWorldMp->g_glassData || Maps::SPMap)
    {
      return Game::DB_XAssetPool[Game::XAssetType::ASSET_TYPE_GAMEWORLD_SP].gameWorldSp->g_glassData;
    }

    return Game::DB_XAssetPool[Game::XAssetType::ASSET_TYPE_GAMEWORLD_MP].gameWorldMp->g_glassData;
  }

  // Load the sun of the map from its sun file, if it has one.
  //
  // On x86 we replace the call to R_FlushSun() in R_LoadWorld(), which x64
  // inlines. So we replace the call right after it instead, before which
  // R_LoadWorld() doesn't touch the sun.
  //
  void Maps::LoadRawSun()
  {
    AssertOffset(Game::GfxWorld, sun, 0x198);

    Game::GfxWorld* world = *reinterpret_cast<Game::GfxWorld**>(0x148CCA620);

    if (FileSystem::File(std::format("sun/{}.sun", Maps::CurrentMainZone)))
    {
      Game::R_LoadSunThroughDvars(Maps::CurrentMainZone.data(), &world->sun);
    }

    Utils::Hook::Call<void()>(0x14003E560)();
  }

  void Maps::ForceRefreshArenas()
  {
    if (!FastFiles::Ready())
    {
      Logger::Print("Not refreshing arenas (fastfiles are not ready yet)\n");
      return;
    }

    if (Game::Sys_IsMainThread())
    {
      if (*Game::g_quitRequested)
      {
        // No need to refresh if we're exiting the game
        return;
      }

      AssertOffset(Game::sharedUiInfo_t, updateArenas, 0x1E0C);

      Game::sharedUiInfo->updateArenas = 1;
      Game::UI_UpdateArenas();
    }
    else
    {
      // Dangerous!!
      assert(false && "Arenas refreshed from wrong thread??");
      Logger::Print("Tried to refresh arenas from a thread that is NOT the main thread!! This could have lead to a crash. Please report this bug and how you got it!");
    }
  }

  // TODO : Remove hook entirely?
  void Maps::GetBSPName(char* buffer, size_t size, const char* format, const char* mapname)
  {
    if (!Utils::String::StartsWith(mapname, "mp_") && !Utils::String::StartsWith(mapname, "zm_") && !Maps::IsUserMap(mapname))
    {
      format = "maps/%s.d3dbsp";
    }

    _snprintf_s(buffer, size, _TRUNCATE, format, mapname);
  }

  void Maps::HandleAsSPMap()
  {
    Maps::SPMap = true;
  }

  int Maps::IgnoreEntityStub(const char* entity)
  {
    return (Utils::String::StartsWith(entity, "dyn_") || Utils::String::StartsWith(entity, "node_") || Utils::String::StartsWith(entity, "actor_"));
  }

  std::unordered_map<std::string, std::string> Maps::ParseCustomMapArena(const std::string& singleMapArena)
  {
    // This is a linear equivalent of the regex "  (\\w*) *\"?((?:\\w| )*)\"?".
    // We don't use std::regex since under MSVC it recurses once per
    // repetition, which overflows the stack on large (downloaded) arena files.
    //
    const auto isWord = [](const char c)
    {
      return (c >= 'a' && c <= 'z') ||
             (c >= 'A' && c <= 'Z') ||
             (c >= '0' && c <= '9') || c == '_';
    };

    std::unordered_map<std::string, std::string> arena;

    const auto size = singleMapArena.size();
    auto pos = singleMapArena.find("  ");

    while (pos != std::string::npos)
    {
      auto i = pos + 2;

      const auto keyStart = i;
      while (i < size && isWord(singleMapArena[i])) ++i;
      const auto keyEnd = i;

      while (i < size && singleMapArena[i] == ' ') ++i;
      if (i < size && singleMapArena[i] == '"') ++i;

      const auto valueStart = i;
      while (i < size && (isWord(singleMapArena[i]) || singleMapArena[i] == ' ')) ++i;
      const auto valueEnd = i;

      if (i < size && singleMapArena[i] == '"') ++i;

      arena.emplace(singleMapArena.substr(keyStart, keyEnd - keyStart), singleMapArena.substr(valueStart, valueEnd - valueStart));

      pos = singleMapArena.find("  ", i);
    }

    return arena;
  }

  Maps::MapDependencies Maps::GetDependenciesForMap(const std::string& map)
  {
    std::string teamAxis = "opforce_composite";
    std::string teamAllies = "us_army";

    Maps::MapDependencies dependencies;

    // True by default - cause some maps won't have an arenafile entry
    dependencies.requiresTeamZones = true;

    for (int i = 0; i < *Game::arenaCount; ++i)
    {
      Game::newMapArena_t* arena = &ArenaLength::NewArenas[i];
      if (arena->mapName == map)
      {
        // If it's in the arena file, surely it's a vanilla map that doesn't need teams...
        dependencies.requiresTeamZones = false;

        for (std::size_t j = 0; j < std::extent_v<decltype(Game::newMapArena_t::keys)>; ++j)
        {
          const auto* key = arena->keys[j];
          const auto* value = arena->values[j];
          if (key == "dependency"s)
          {
            dependencies.requiredMaps = Utils::String::Split(arena->values[j], ' ');
          }
          else if (key == "allieschar"s)
          {
            teamAllies = value;
          }
          else if (key == "axischar"s)
          {
            teamAxis = value;
          }
          else if (key == "useteamzones"s)
          {
            // ... unless it specifies so!  This allows loading of CODO/COD4 zones that might not have the correct teams
            dependencies.requiresTeamZones = Utils::String::ToLower(value) == "true"s;
          }
        }

        break;
      }
    }

    dependencies.requiredTeams = std::make_pair(teamAllies, teamAxis);

    return dependencies;
  }

  void Maps::PrepareUsermap(const char* mapname)
  {
    // Handle the redundant call scenario first.
    //
    // It appears that this function is called a second time during fastfile
    // loading. If we are already holding a valid IWD for this map, we must
    // return immediately. Proceeding would cause us to free the IWD we just
    // loaded.
    //
    if (Maps::UserMap.isValid() && Maps::UserMap.getName() == mapname)
      return;

    // If we are here, we are either starting up or switching contexts. In
    // either case, we need to tear down any previous state before loading the
    // new one.
    //
    if (Maps::UserMap.isValid())
    {
      Maps::UserMap.freeIwd();
      Maps::UserMap.clear();
    }

    // Now set up the new map.
    //
    if (Maps::IsUserMap(mapname))
    {
      Maps::UserMap = Maps::UserMapContainer(mapname);
      Maps::UserMap.loadIwd();
    }
    else
    {
      // If the mapname isn't valid, we leave the state cleared.
      //
      Maps::UserMap.clear();
    }
  }

  void Maps::SV_SpawnServer_Hk(const char* server, int mapIsPreloaded, int savegame, int devmap, int migrate)
  {
    PrepareUsermap(server);
    SV_SpawnServer_Original(server, mapIsPreloaded, savegame, devmap, migrate);
  }

  // Set sv_cheats for the server that the map or devmap command is spawning
  // and then make the call we replace.
  //
  void Maps::SV_SpawnServer_SetCheats()
  {
    if (Game::sv_spawnCheats)
    {
      Game::Dvar_SetBool(Game::dvar_cheats, *Game::sv_spawnCheats);
    }

    Utils::Hook::Call<void()>(0x1401AA2D0)();
  }

  // Note that the zone is the loadscreen zone of the map (<map>_load).
  //
  void Maps::LoadMapLoadscreenStub(Game::XZoneInfo* zoneInfo, unsigned int zoneCount, int sync)
  {
    std::string_view name (zoneInfo->name);

    assert(name.ends_with("_load"));

    name.remove_suffix(5);
    PrepareUsermap(std::string(name).data());

    Game::DB_LoadXAssets(zoneInfo, zoneCount, sync);
  }

  unsigned int Maps::GetUsermapHash(const std::string& map)
  {
    if (Utils::IO::DirectoryExists(std::format("usermaps/{}", map)))
    {
      std::string hash;

      for (std::size_t i = 0; i < ARRAYSIZE(Maps::UserMapFiles); ++i)
      {
        auto filePath = std::format("usermaps/{}/{}{}", map, map, Maps::UserMapFiles[i]);
        if (Utils::IO::FileExists(filePath))
        {
          // Streamed rather than read whole: usermap iwds run to tens of megabytes, and asking
          // for that much contiguous memory is what tips an already loaded client over.
          hash.append(Utils::Cryptography::SHA256::ComputeFile(filePath));
        }
      }

      return Utils::Cryptography::JenkinsOneAtATime::Compute(hash);
    }

    return 0;
  }

  void Maps::LoadNewMapCommand(char* buffer, size_t size, const char* /*format*/, const char* mapname, const char* gametype)
  {
    unsigned int hash = Maps::GetUsermapHash(mapname);
    _snprintf_s(buffer, size, _TRUNCATE, "loadingnewmap\n%s\n%s\n%d", mapname, gametype, hash);
  }

  int Maps::TriggerReconnectForMap(Game::msg_t* msg, const char* mapname)
  {
    Theatre::StopRecording();

    char hashBuf[100] = { 0 };
    unsigned int hash = atoi(Game::MSG_ReadStringLine(msg, hashBuf, sizeof(hashBuf)));

    // If the server rotated to a DLC map we don't have, then loading it
    // fails with a fatal error. So skip the map change, disconnect, and show
    // the missing DLC error instead, the same as when joining such a server.
    //
    // Note that we cannot just fail with Com_Error(): it does disconnect,
    // but its message is lost on the way back to the main menu. Instead, the
    // disconnect command shows the "disconnected from server" error and we
    // replace its message once cgame is down. Note that opening a popup of
    // our own on top of it would leave the player with a black screen since
    // only that error's popup returns to the main menu.
    //
    if (const auto message = Maps::GetMissingDlcError(mapname))
    {
      Command::Execute("disconnect", false);

      Scheduler::Schedule([message = *message]
      {
        if (Game::CL_IsCgameInitialized())
          return false;

        Game::Dvar_SetStringByName("com_errorMessage", message.data());
        return true;
      }, Scheduler::Pipeline::MAIN);

      return true;
    }

    // If we are still connecting, then the gamestate we have is for the
    // previous map and loading it on top of the new one fails. So reconnect,
    // the same as for a map we don't have.
    //
    // Note that by now the game has already set the state to connected for
    // the map change, so we check the one it had before (see the
    // Maps_SetConnectedStub hook below).
    //
    const auto connecting = Maps_RotateConnectionState < Game::CA_ACTIVE;

    if (connecting || !Maps::CheckMapInstalled(mapname, false, true) || (hash && hash != Maps::GetUsermapHash(mapname)))
    {
      // Reconnecting forces the client to download the new map
      Command::Execute("disconnect", false);
      Command::Execute("awaitDatabase", false); // Wait for the database to load
      Command::Execute("wait 100", false);
      Command::Execute("openmenu popup_reconnectingtoparty", false);
      Command::Execute("delayReconnect", false);
      return true;
    }

    return false;
  }

  // Make sure we have the map before starting a match we host.
  //
  // Starting a match tells the rest of the party to go and then loads the
  // map selected in the lobby (ui_mapname). Nothing on the way checks that we
  // actually have it and, for a DLC map we don't, we only find out when
  // loading its zone, which is a fatal error. So check here, while we are
  // still in the lobby, and show the same missing DLC error we show when
  // joining a server on such a map.
  //
  // Note that cgame isn't running at this point, so we can show the popup
  // directly (compare to TriggerReconnectForMap() above). Note also that the
  // match is started by xpartygo as well as by the lobby's start timer, thus
  // the two hooks.
  //
  // As far as we can tell, this is the only way to end up hosting a map we
  // don't have: the map command (and so map_rotate) checks that the map
  // exists first and we don't rotate while hosting a party anyway (see
  // MapRotation::ShouldRotate()).
  //
  void Maps::StartMatch(void* party, int localControllerIndex)
  {
    if (const auto message = Maps::GetMissingDlcError((*Game::ui_mapname)->current.string))
    {
      Game::ShowMessageBox(*message, "Error");
      return;
    }

    Utils::Hook::Call<void(void*, int)>(0x140111E00)(party, localControllerIndex);
  }

  void Maps::DownloadsComplete(const int localClientNum)
  {
    // The game skips loading the map if it already did so ahead of time,
    // that is, when connecting or when the server announced a map change.
    // But if the server changed map in the meantime (say, it rotated while
    // we were connecting), then the gamestate is for a different map than
    // the one we loaded and starting it fails. So forget about the loaded
    // map in this case, which makes the game load the right one.
    //
    const auto* mapname = Game::Info_ValueForKey(Game::CL_GetConfigString(0), "mapname");

    if (_stricmp(mapname, Game::loadingMapName) != 0)
    {
      Game::CL_GetLocalClientUIGlobals(localClientNum)->mapPreloaded = false;
      *Game::cl_serverChangingMap = false;
    }

    Game::CL_DownloadsComplete(localClientNum);
  }

  void Maps::AddDlc(Maps::DLC dlc)
  {
    for (auto& pack : Maps::DlcPacks)
    {
      if (pack.index == dlc.index)
      {
        pack.maps = dlc.maps;
        Maps::UpdateDlcStatus();
        return;
      }
    }

    Dvar::Register<bool>(Utils::String::VA("isDlcInstalled_%d", dlc.index), false, Game::DVAR_EXTERNAL | Game::DVAR_INIT, "");

    Maps::DlcPacks.push_back(dlc);
    Maps::UpdateDlcStatus();
  }

  void Maps::UpdateDlcStatus()
  {
    bool hasAllDlcs = true;
    std::vector<bool> hasDlc;
    for (auto& pack : Maps::DlcPacks)
    {
      bool hasAllMaps = true;
      for (auto map : pack.maps)
      {
        if (!FastFiles::Exists(map))
        {
          hasAllMaps = false;
          hasAllDlcs = false;
          break;
        }
      }

      hasDlc.push_back(hasAllMaps);
      Dvar::Var(Utils::String::VA("isDlcInstalled_%d", pack.index)).set(hasAllMaps ? true : false);
    }

    // Must have all of dlc 3 to 5 or it causes issues
    static bool sentMessage = false;
    if (hasDlc.size() >= 5 && (hasDlc[2] || hasDlc[3] || hasDlc[4]) && (!hasDlc[2] || !hasDlc[3] || !hasDlc[4]) && !sentMessage)
    {
      StartupMessages::AddMessage("Warning:\n You only have some of DLCs 3-5 which are all required to be installed to work. There may be issues with those maps.");
      sentMessage = true;
    }

    Dvar::Var("isDlcInstalled_All").set(hasAllDlcs ? true : false);
  }

  bool Maps::IsCustomMap()
  {
    Game::GfxWorld*& gameWorld = *reinterpret_cast<Game::GfxWorld**>(0x148CCA620);
    if (gameWorld) return (gameWorld->checksum & 0xFFFF0000) == 0xC0D40000;
    return false;
  }

  bool Maps::IsUserMap(const std::string& mapname)
  {
    return Utils::IO::DirectoryExists(std::format("usermaps/{}", mapname)) && Utils::IO::FileExists(std::format("usermaps/{}/{}.ff", mapname, mapname));
  }

  void Maps::ScanCustomMaps()
  {
    FoundCustomMaps.clear();
    Logger::Print("Looking for custom maps...\n");

    std::filesystem::path basePath = (*Game::fs_basepath)->current.string;
    basePath /= "usermaps";

    if (!std::filesystem::exists(basePath))
    {
      return;
    }

    const auto entries = Utils::IO::ListFiles(basePath);

    for (const auto& entry : entries)
    {
      if (entry.is_directory())
      {
        const auto zoneName = entry.path().filename().string();
        const auto mapPath = std::format("{}\\{}.ff", entry.path().string(), zoneName);
        if (Utils::IO::FileExists(mapPath))
        {
          FoundCustomMaps.push_back(zoneName);
          Logger::Print("Discovered custom map {}\n", zoneName);
        }
      }
    }
  }

  std::string Maps::GetArenaPath(const std::string& mapName)
  {
    return std::format("usermaps/{}/{}.arena", mapName, mapName);
  }

  const std::vector<std::string>& Maps::GetCustomMaps()
  {
    return FoundCustomMaps;
  }

  // Note that on x86 we read the pool back from one of the instructions that
  // AssetHandler patches when it reallocates it. On x64 AssetHandler keeps
  // it in g_assetEntryPool instead.
  //
  Game::XAssetEntry* Maps::GetAssetEntryPool()
  {
    return Game::g_assetEntryPool;
  }

  // dlcIsTrue serves as a check if the map is a custom map and if it's missing
  bool Maps::CheckMapInstalled(const std::string& mapname, bool error, bool dlcIsTrue)
  {
    if (FastFiles::Exists(mapname)) return true;

    if (const auto message = Maps::GetMissingDlcError(mapname))
    {
      if (error)
      {
        Logger::Error(Game::ERR_DISCONNECT, "{}", *message);
      }

      return dlcIsTrue;
    }

    if (error)
    {
      Logger::Error(Game::ERR_DISCONNECT,
        "Missing map file {}.\nYou may have a damaged installation or are attempting to load a non-existent map.", mapname);
    }

    return false;
  }

  std::optional<std::string> Maps::GetMissingDlcError(const std::string& mapname)
  {
    if (FastFiles::Exists(mapname)) return {};

    for (const auto& pack : Maps::DlcPacks)
    {
      if (std::ranges::find(pack.maps, mapname) != pack.maps.end())
      {
        return std::format("Missing DLC pack {} ({}) containing map {} ({}).\nPlease download it to play this map.",
          pack.name, pack.index, Localization::LocalizeMapName(mapname.data()), mapname);
      }
    }

    return {};
  }

  void Maps::HideModel()
  {
    Game::GfxWorld*& gameWorld = *reinterpret_cast<Game::GfxWorld**>(0x148CCA620);
    if (!gameWorld) return;

    std::string model = Dvar::Var("r_hideModel").get<std::string>();
    if (model.empty()) return;

    for (unsigned int i = 0; i < gameWorld->dpvs.smodelCount; ++i)
    {
      if (model == "all"s || gameWorld->dpvs.smodelDrawInsts[i].model->name == model)
      {
        gameWorld->dpvs.smodelVisData[0][i] = 0;
        gameWorld->dpvs.smodelVisData[1][i] = 0;
        gameWorld->dpvs.smodelVisData[2][i] = 0;
      }
    }
  }

  void Maps::HideModelStub(void* viewInfo)
  {
    HideModel();
    Utils::Hook::Call<void(void*)>(0x14005FCC0)(viewInfo);
  }

  void Maps::G_SpawnTurretHook(Game::gentity_s* ent, const char* weaponName, int unk)
  {
    if (Maps::CurrentMainZone == "mp_backlot_sh"s || Maps::CurrentMainZone == "mp_con_spring"s ||
      Maps::CurrentMainZone == "mp_mogadishu_sh"s || Maps::CurrentMainZone == "mp_nightshift_sh"s)
    {
      return;
    }

    Utils::Hook::Call<void(Game::gentity_s*, const char*, int)>(0x14018F730)(ent, weaponName, unk); // G_SpawnTurret
  }

  bool Maps::SV_SetTriggerModelHook(Game::gentity_s* ent) {

    // Use me for debugging
    //std::string classname = Game::SL_ConvertToString(ent->script_classname);
    //std::string targetname = Game::SL_ConvertToString(ent->targetname);

    return Utils::Hook::Call<bool(Game::gentity_s*)>(0x140231FE0)(ent); // SV_SetTriggerModel
  }

  unsigned short Maps::CM_TriggerModelBounds_Hk(unsigned int triggerIndex, Game::Bounds* bounds)
  {

    auto* ents = Game::cm->mapEnts;  // Use me for debugging

    if (ents)
    {
      if (triggerIndex >= ents->trigger.count)
      {
        Logger::Error(Game::errorParm_t::ERR_DROP, "Invalid trigger index ({}) in entities exceeds the maximum trigger count ({}) defined in the clipmap. Check your map ents, or your clipmap!", triggerIndex, ents->trigger.count);
        return 0;
      }
      else
      {
        return Utils::Hook::Call<unsigned short(int, Game::Bounds*)>(0x1401E8210)(triggerIndex, bounds); // CM_TriggerModelBounds
      }
    }

    return 0;
  }

  void Maps::GSCr_GetMapArenaInfo()
  {
    if (Game::Scr_GetNumParam() != 2)
    {
      Game::Scr_Error("GetMapArenaInfo: Needs a map name and a field name!");
      return;
    }

    const std::string mapName = Game::SL_ConvertToString(Game::Scr_GetConstString(0));
    const std::string fieldName = Game::SL_ConvertToString(Game::Scr_GetConstString(1));

    Game::UI_UpdateArenas();
    for (int i = 0; i < *Game::arenaCount; ++i)
    {
      Game::newMapArena_t* arena = &ArenaLength::NewArenas[i];
      if (arena->mapName == mapName)
      {
        // Found the map!
        for (std::size_t j = 0; j < std::extent_v<decltype(Game::newMapArena_t::keys)>; ++j)
        {
          const auto* key = arena->keys[j];
          const auto* value = arena->values[j];

          // Foudn the field!
          if (key == fieldName)
          {
            Game::Scr_AddString(value);
            return;
          }
        }

        const std::string error = std::format("Could not find field {} in arena entry for map {}!\n", fieldName, mapName);
        Game::Scr_Error(error.data());
        return;
      }
    }

    const std::string error = std::format("Map {} has no arena entry!\n", mapName);
    Game::Scr_Error(error.data());
  }

  void Maps::GSCr_GetMapList()
  {
    Game::Scr_MakeArray();

    Game::UI_UpdateArenas();
    for (int i = 0; i < *Game::arenaCount; ++i)
    {
      Game::newMapArena_t* arena = &ArenaLength::NewArenas[i];
      Game::Scr_AddString(arena->mapName);
      Game::Scr_AddArray();
    }
  }

  Maps::Maps()
  {
    Scheduler::Once([]
      {
        Dvar::Register<bool>("isDlcInstalled_All", false, Game::DVAR_EXTERNAL | Game::DVAR_INIT, "");
        Maps::RListSModels = Dvar::Register<bool>("r_listSModels", false, Game::DVAR_NONE, "Display a list of visible SModels");

        Maps::AddDlc({ 1, "Stimulus Pack", {"mp_complex", "mp_compact", "mp_storm", "mp_overgrown", "mp_crash"} });
        Maps::AddDlc({ 2, "Resurgence Pack", {"mp_abandon", "mp_vacant", "mp_trailerpark", "mp_strike", "mp_fuel2"} });
        Maps::AddDlc({ 3, "IW4x Classics", {"mp_nuked", "mp_cross_fire", "mp_cargoship", "mp_bloc", "mp_killhouse", "mp_bog_sh", "mp_cargoship_sh", "mp_shipment", "mp_shipment_long", "mp_rust_long", "mp_firingrange", "mp_bloc_sh", "mp_crash_tropical", "mp_estate_tropical", "mp_fav_tropical", "mp_storm_spring"} });
        Maps::AddDlc({ 4, "Call Of Duty 4 Pack", {"mp_farm", "mp_backlot", "mp_pipeline", "mp_countdown", "mp_crash_snow", "mp_carentan", "mp_broadcast", "mp_showdown", "mp_convoy", "mp_citystreets"} });
        Maps::AddDlc({ 5, "Modern Warfare 3 Pack", {"mp_dome", "mp_hardhat", "mp_paris", "mp_seatown", "mp_bravo", "mp_underground", "mp_plaza2", "mp_village", "mp_alpha"} });

        Maps::UpdateDlcStatus();

        UIScript::Add("downloadDLC", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
          {
            const auto dlc = token.get<int>();

            Game::ShowMessageBox(Utils::String::VA("DLC %d does not exist!", dlc), "ERROR");
          });
      }, Scheduler::Pipeline::MAIN);

    // disable turrets on CoD:OL 448+ maps for now
    Utils::Hook(0x14017A00E, Maps::G_SpawnTurretHook, HOOK_CALL).install()->quick();
    Utils::Hook(0x14018FD3F, Maps::G_SpawnTurretHook, HOOK_CALL).install()->quick();

    // Catch trigger errors before they're critical
    Utils::Hook(0x140231FF4, Maps::CM_TriggerModelBounds_Hk, HOOK_CALL).install()->quick(); // SV_SetTriggerModel

#ifdef DEBUG
    // Check trigger models
    //
    // Note that x64 inlines InitTrigger(), which we hook on x86, into each of
    // the trigger spawn functions.
    //
    Utils::Hook(0x140186D21, Maps::SV_SetTriggerModelHook, HOOK_CALL).install()->quick();
    Utils::Hook(0x140186EC9, Maps::SV_SetTriggerModelHook, HOOK_CALL).install()->quick();
    Utils::Hook(0x140186F89, Maps::SV_SetTriggerModelHook, HOOK_CALL).install()->quick();
    Utils::Hook(0x140187037, Maps::SV_SetTriggerModelHook, HOOK_CALL).install()->quick();
    Utils::Hook(0x14018748C, Maps::SV_SetTriggerModelHook, HOOK_CALL).install()->quick();
#endif

    //

//#define SORT_SMODELS
#if !defined(DEBUG) || !defined(SORT_SMODELS)
    // Don't sort static models
    Utils::Hook::Nop(0x14005381A, 2);
#endif

    // Don't draw smodels
    //Utils::Hook::Set<BYTE>(0x14005FCC0, 0xC3);

    // Restrict asset loading
    AssetHandler::OnLoad(Maps::LoadAssetRestrict);

    // Raise the size of the PMem pool from the stock 300 MiB to 640 MiB, as
    // x86 does. At 450 MiB several ported maps (mp_cement, mp_boardwalk,
    // mp_carbon, mp_radar, mp_exchange, mp_meteora and IW4x's DLC
    // mp_underground) fail mid-load with "Need N more bytes of ram for alloc
    // to succeed". PMem_Init() only reserves the range (MEM_RESERVE) and the
    // pages are committed as zones load. On x86 the size is a trade-off,
    // since the reservation takes address space from the rest of the
    // process, which x64 has plenty of.
    //
    // Note that on x86 we leave the pool to ZoneBuilder when it's enabled,
    // which the x64 build leaves out.
    //
    Utils::Hook::Set<DWORD>(0x140289067, 0x28000000); // 640 MiB
    Utils::Hook::Set<DWORD>(0x1402890B7, 0x28000000);

    // Intercept BSP name resolving
    //
    // Note that on x64 the game ends with a jump to Com_sprintf(), which we
    // replace with a jump to us.
    //
    Utils::Hook(0x140272D5D, Maps::GetBSPName, HOOK_JUMP).install()->quick();

    // Intercept map zone loading/unloading
    Utils::Hook(0x14012EBEA, Maps::LoadMapZones, HOOK_CALL).install()->quick();
    Utils::Hook(0x1401F32FA, Maps::UnloadMapZones, HOOK_CALL).install()->quick();

    // Ignore SP entities
    Utils::Hook(0x14028B110, Maps::IgnoreEntityStub, HOOK_JUMP).install()->quick();

    // WorldData pointer replacement
    //
    // Note that on x64 the game loads the pointer into rcx with a 7-byte
    // instruction.
    //
    Utils::Hook(0x140169706, Maps_GetWorldDataStub, HOOK_CALL).install()->quick();
    Utils::Hook::Nop(0x14016970B, 2);

    // Allow loading raw suns
    Utils::Hook(0x140037320, Maps::LoadRawSun, HOOK_CALL).install()->quick();

    // Intercept map loading for usermap initialization
    //
    // On x86 we hook the two calls to SV_SpawnServer(). On x64 we also call
    // it ourselves for the map and devmap commands, and so we hook it as a
    // whole instead.
    //
    // Note also that x64 inlines the function that loads the loadscreen zone
    // of a map, which we hook the calls to on x86, into most of its callers.
    // So we hook the loading of the zone in each copy, where the one for
    // StartMatch() moved into SV_SpawnServer().
    //
    SV_SpawnServer_Original = Utils::Hook::Detour(0x140239750, 5, &SV_SpawnServer_Hk);
    Utils::Hook(0x1400FA06D, Maps::LoadMapLoadscreenStub, HOOK_CALL).install()->quick(); // CL_ConnectionlessPacket
    Utils::Hook(0x1400FA359, Maps::LoadMapLoadscreenStub, HOOK_CALL).install()->quick(); // CL_DownloadsComplete
    Utils::Hook(0x1400FDF23, Maps::LoadMapLoadscreenStub, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400FE305, Maps::LoadMapLoadscreenStub, HOOK_CALL).install()->quick();
    Utils::Hook(0x140239848, Maps::LoadMapLoadscreenStub, HOOK_CALL).install()->quick(); // SV_SpawnServer

    // Restore the map and devmap commands, which x64 compiles out along with
    // the rest of the operator commands, the way x86 registers them when the
    // server starts.
    //
    // On x86 SV_SpawnServer() then sets sv_cheats to whether the command was
    // devmap, right after clearing the config strings. x64 compiles that out
    // as well, so we set it there ourselves, though only for these commands.
    // x64 passes no devmap for its other spawns and so taking it from there
    // would disable cheats on a fast restart, which on x86 keeps them as they
    // are.
    //
    Events::OnSVInit([]
    {
      Command::AddSV("map", []([[maybe_unused]] const Command::Params* params) {Game::SV_Map_f();});
      Command::AddSV("devmap", []([[maybe_unused]] const Command::Params* params) {Game::SV_DevMap_f();});
    });

    Utils::Hook(0x140239ABC, Maps::SV_SpawnServer_SetCheats, HOOK_CALL).install()->quick();

    Command::Add("delayReconnect", []()
      {
        Scheduler::Once([]
          {
            Command::Execute("closemenu popup_reconnectingtoparty", false);
            Command::Execute("reconnect", false);
          }, Scheduler::Pipeline::CLIENT, 10s);
      });

    if (Dedicated::IsEnabled())
    {
      Utils::Hook(0x140239910, Maps::LoadNewMapCommand, HOOK_CALL).install()->quick(); // SV_SpawnServer
    }

    // Download the map before a map rotation if necessary
    // Conflicts with Theater's SV map rotation check, but this one is safer!
    //
    // Note that x64 inlines the function that handles the map change, which
    // we hook the call to on x86, into CL_ConnectionlessPacket(). So we jump
    // to the stub where the copy starts.
    //
    Utils::Hook(0x1400F9FAF, Maps_RotateCheckStub, HOOK_JUMP).install()->quick();

    // Remember the connection state before the game sets it to connected for
    // a map change.
    //
    Utils::Hook(0x1400F9F9C, Maps_SetConnectedStub, HOOK_CALL).install()->quick();
    Utils::Hook::Nop(0x1400F9FA1, 6);

    // Ignore the server's request for our stats unless we are connected to
    // it.
    //
    // The game starts sending them whatever the state, so a request that
    // arrives after we have disconnected (say, from the server we just left
    // after it rotated to a map we don't have) puts us back to connecting
    // and we sit on a black screen until it times out.
    //
    Utils::Hook(0x1400FA09B, Maps_RequestStatsStub, HOOK_JUMP).install()->quick();
    Utils::Hook(0x14010034E, Maps::DownloadsComplete, HOOK_CALL).install()->quick(); // CL_ParseGamestate
    Utils::Hook(0x1400FBF16, Maps::StartMatch, HOOK_JUMP).install()->quick(); // xpartygo command.
    Utils::Hook(0x14010EDF0, Maps::StartMatch, HOOK_CALL).install()->quick(); // Lobby start timer.

    // Load usermap arena file
    Utils::Hook(0x140269A06, Maps::LoadArenaFileStub, HOOK_CALL).install()->quick();

    // Allow hiding specific smodels
    Utils::Hook(0x140022BB0, Maps::HideModelStub, HOOK_CALL).install()->quick();

    // gsc: GetMapList()
    Components::GSC::Script::AddFunction(
      "GetMapList",
      GSCr_GetMapList
    );

    // gsc: GetMapArenaInfo(mapName, fieldName)
    Components::GSC::Script::AddFunction(
      "GetMapArenaInfo",
      GSCr_GetMapArenaInfo
    );

    if (Dedicated::IsEnabled())
    {
      return;
    }

    // Client only
    Scheduler::Loop([]
      {
        auto*& gameWorld = *reinterpret_cast<Game::GfxWorld**>(0x148CCA620);
        if (!Game::CL_IsCgameInitialized() || !gameWorld || !Maps::RListSModels.get<bool>()) return;

        std::map<std::string, int> models;
        for (unsigned int i = 0; i < gameWorld->dpvs.smodelCount; ++i)
        {
          if (gameWorld->dpvs.smodelVisData[0][i])
          {
            std::string name = gameWorld->dpvs.smodelDrawInsts[i].model->name;

            if (!models.contains(name)) models[name] = 1;
            else models[name]++;
          }
        }

        Game::Font_s* font = Game::R_RegisterFont("fonts/smallFont", 0);
        auto height = Game::R_TextHeight(font);
        auto scale = 0.75f;
        float color[4] = { 0.0f, 1.0f, 0.0f, 1.0f };

        unsigned int i = 0;
        for (auto& model : models)
        {
          Game::R_AddCmdDrawText(Utils::String::VA("%d %s", model.second, model.first.data()), std::numeric_limits<int>::max(), font, 15.0f, (height * scale + 1) * (i++ + 1) + 15.0f, scale, scale, 0.0f, color, Game::ITEM_TEXTSTYLE_NORMAL);
        }
      }, Scheduler::Pipeline::RENDERER);
  }

  Maps::~Maps()
  {
    Maps::DlcPacks.clear();
    Maps::DependencyList.clear();
    Maps::CurrentMainZone.clear();
    Maps::CurrentDependencies.clear();
  }
}

Game::G_GlassData* Maps_GetWorldData()
{
  return Components::Maps::GetWorldData();
}

int Maps_TriggerReconnectForMap(Game::msg_t* msg, const char* mapname)
{
  return Components::Maps::TriggerReconnectForMap(msg, mapname);
}
