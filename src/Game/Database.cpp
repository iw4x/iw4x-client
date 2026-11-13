#include "Database.hpp"

namespace Game
{
  DB_BeginRecoverLostDevice_t DB_BeginRecoverLostDevice = DB_BeginRecoverLostDevice_t(0x14012CF10);
  DB_EndRecoverLostDevice_t DB_EndRecoverLostDevice = DB_EndRecoverLostDevice_t(0x14012D190);
  DB_EnumXAssets_t DB_EnumXAssets = DB_EnumXAssets_t(0x14012D240);
  DB_EnumXAssets_Internal_t DB_EnumXAssets_Internal = DB_EnumXAssets_Internal_t(0x14012D2C0);
  DB_FindXAssetHeader_t DB_FindXAssetHeader = DB_FindXAssetHeader_t(0x14012D6B0);
  DB_GetRawBuffer_t DB_GetRawBuffer = DB_GetRawBuffer_t(0x14012DD60);
  DB_GetRawFileLen_t DB_GetRawFileLen = DB_GetRawFileLen_t(0x14012DE90);
  DB_GetLoadedFraction_t DB_GetLoadedFraction = DB_GetLoadedFraction_t(0x140117CC0);
  DB_IsXAssetDefault_t DB_IsXAssetDefault = DB_IsXAssetDefault_t(0x14012E160);
  DB_LoadXAssets_t DB_LoadXAssets = DB_LoadXAssets_t(0x14012EC20);
  DB_LoadXFileData_t DB_LoadXFileData = DB_LoadXFileData_t(0x1401182B0);
  DB_ReadXFile_t DB_ReadXFile = DB_ReadXFile_t(0x1401182B0);
  DB_ReadXFileUncompressed_t DB_ReadXFileUncompressed = DB_ReadXFileUncompressed_t(0x1401183E0);
  DB_SetXAssetName_t DB_SetXAssetName = DB_SetXAssetName_t(0x140117620);
  DB_FindXAssetDefaultHeaderInternal_t DB_FindXAssetDefaultHeaderInternal = DB_FindXAssetDefaultHeaderInternal_t(0x14012D4D0);
  DB_FindXAssetEntry_t DB_FindXAssetEntry = DB_FindXAssetEntry_t(0x14012D5E0);
  DB_XModelSurfsFixup_t DB_XModelSurfsFixup = DB_XModelSurfsFixup_t(0x140130100);

  DB_SetXAssetNameHandler_t* DB_SetXAssetNameHandlers = reinterpret_cast<DB_SetXAssetNameHandler_t*>(0x14041F540);
  DB_GetXAssetNameHandler_t* DB_GetXAssetNameHandlers = reinterpret_cast<DB_GetXAssetNameHandler_t*>(0x14041F3E0);
  DB_GetXAssetSizeHandler_t* DB_GetXAssetSizeHandlers = reinterpret_cast<DB_GetXAssetSizeHandler_t*>(0x14041F6A0);
  DB_ReleaseXAssetHandler_t* DB_ReleaseXAssetHandlers = reinterpret_cast<DB_ReleaseXAssetHandler_t*>(0x140420180);
  DB_DynamicCloneXAssetHandler_t* DB_DynamicCloneXAssetHandler = reinterpret_cast<DB_DynamicCloneXAssetHandler_t*>(0x140420020);

  XAssetHeader* DB_XAssetPool = reinterpret_cast<XAssetHeader*>(0x14041FD60);
  unsigned int* g_poolSize = reinterpret_cast<unsigned int*>(0x14041F890);
  const char** g_assetNames = reinterpret_cast<const char**>(0x14041F280);

  XBlock** g_streamBlocks = reinterpret_cast<XBlock**>(0x1415FAEB8);
  char** g_streamPos = reinterpret_cast<char**>(0x1415FAEC0);
  int* g_streamPosIndex = reinterpret_cast<int*>(0x1415FAEB0);

  FastCriticalSection* db_hashCritSect = reinterpret_cast<FastCriticalSection*>(0x14151B4B0);

  XZone* g_zones = reinterpret_cast<XZone*>(0x1415F81A0);
  unsigned short* db_hashTable = reinterpret_cast<unsigned short*>(0x1415093A0);
  XAssetEntry* g_assetEntryPool = reinterpret_cast<XAssetEntry*>(0x14151F4E0);

  XAssetHeader ReallocateAssetPool(XAssetType type, unsigned int newSize)
  {
    // Note that a pool starts with its free list head, which the game's
    // pool initialization points at the first of the entries that follow.
    //
    const int size = DB_GetXAssetSizeHandlers[type]();
    XAssetHeader poolEntry = {Utils::Memory::GetAllocator()->allocate(sizeof(void*) + static_cast<size_t>(newSize) * size)};
    DB_XAssetPool[type] = poolEntry;
    g_poolSize[type] = newSize;
    return poolEntry;
  }

  // Note that x64 inlines this lookup.
  //
  const char* DB_GetXAssetTypeName(XAssetType type)
  {
    return type >= 0 && type < ASSET_TYPE_COUNT ? g_assetNames[type] : "";
  }

  const char* DB_GetXAssetName(XAsset* asset)
  {
    if (!asset) return "";

    assert(asset->header.data);

    return DB_GetXAssetNameHandlers[asset->type](&asset->header);
  }

  XAssetType DB_GetXAssetNameType(const char* name)
  {
    for (int i = 0; i < ASSET_TYPE_COUNT; ++i)
    {
      XAssetType type = static_cast<XAssetType>(i);
      if (!_stricmp(DB_GetXAssetTypeName(type), name))
      {
        // Col map workaround!
        if (type == Game::ASSET_TYPE_CLIPMAP_SP)
        {
          return Game::ASSET_TYPE_CLIPMAP_MP;
        }

        return type;
      }
    }

    return ASSET_TYPE_INVALID;
  }

  int DB_GetZoneIndex(const std::string& name)
  {
    for (int i = 0; i < 32; ++i)
    {
      if (Game::g_zones[i].name == name)
      {
        return i;
      }
    }

    return -1;
  }

  bool DB_IsZoneLoaded(const char* zone)
  {
    int zoneCount = Utils::Hook::Get<int>(0x1415FA1B8);
    char* zoneIndices = reinterpret_cast<char*>(0x1415FA198);
    char* zoneData = reinterpret_cast<char*>(0x1415F81A0);

    for (int i = 0; i < zoneCount; ++i)
    {
      std::string name = zoneData + 4 + 0xA4 * (zoneIndices[i] & 0xFF);

      if (name == zone)
      {
        return true;
      }
    }

    return false;
  }

  std::vector<std::string> DB_GetLoadedZones()
  {
    int zoneCount = Utils::Hook::Get<int>(0x1415FA1B8);
    char* zoneIndices = reinterpret_cast<char*>(0x1415FA198);
    char* zoneData = reinterpret_cast<char*>(0x1415F81A0);

    std::vector<std::string> zones;

    for (int i = 0; i < zoneCount; ++i)
    {
      zones.emplace_back(zoneData + 4 + 0xA4 * (zoneIndices[i] & 0xFF));
    }

    return zones;
  }

  void DB_EnumXAssetEntries(XAssetType type, std::function<void(XAssetEntry*)> callback, bool overrides)
  {
    Sys_LockRead(db_hashCritSect);

    XAssetEntry* pool = g_assetEntryPool;
    for (auto hash = 0; hash < 37000; hash++)
    {
      auto hashIndex = db_hashTable[hash];
      while (hashIndex)
      {
        auto* assetEntry = &pool[hashIndex];

        if (assetEntry->asset.type == type)
        {
          callback(assetEntry);
          if (overrides)
          {
            auto overrideIndex = assetEntry->nextOverride;
            while (overrideIndex)
            {
              auto* overrideEntry = &pool[overrideIndex];
              callback(overrideEntry);
              overrideIndex = overrideEntry->nextOverride;
            }
          }
        }

        hashIndex = assetEntry->nextHash;
      }
    }

    Sys_UnlockRead(db_hashCritSect);
  }
}
