#include "StructuredData.hpp"
#include "AssetHandler.hpp"
#include "FastFiles.hpp"

namespace Components
{
  constexpr auto BASE_PLAYERSTATS_VERSION = 155;

  Utils::Memory::Allocator StructuredData::MemAllocator;
  Game::StructuredDataDefSet StructuredData::StockPlayerDataDef{};
  std::string StructuredData::ModFsGame;
  Game::StructuredDataDef* StructuredData::StatsPlayerDataDefs = nullptr;

  const char* StructuredData::EnumTranslation[COUNT] =
  {
    "features",
    "weapons",
    "attachements",
    "challenges",
    "camos",
    "perks",
    "killstreaks",
    "accolades",
    "cardicons",
    "cardtitles",
    "cardnameplates",
    "teams",
    "gametypes"
  };

  void StructuredData::PatchPlayerDataEnum(Game::StructuredDataDef* data, StructuredData::PlayerDataType type, std::vector<std::string>& entries)
  {
    if (!data || type >= StructuredData::PlayerDataType::COUNT) return;

    // Reallocate them before patching
    Game::StructuredDataEnum* newEnums = MemAllocator.allocateArray<Game::StructuredDataEnum>(data->enumCount);
    std::memcpy(newEnums, data->enums, sizeof(Game::StructuredDataEnum) * data->enumCount);
    data->enums = newEnums;

    Game::StructuredDataEnum* dataEnum = &data->enums[type];

    // Build index-sorted data vector
    std::vector<const char*> dataVector;
    for (int i = 0; i < dataEnum->entryCount; ++i)
    {
      int index = 0;
      for (; index < dataEnum->entryCount; ++index)
      {
        if (dataEnum->entries[index].index == i)
        {
          break;
        }
      }

      dataVector.push_back(dataEnum->entries[index].string);
    }

    // Rebase or add new entries
    for (auto entry : entries)
    {
      const char* value = nullptr;
      for (auto i = dataVector.begin(); i != dataVector.end(); ++i)
      {
        if (*i == entry)
        {
          value = *i;
          dataVector.erase(i);
          Logger::Print("Playerdatadef entry '{}' will be rebased!\n", value);
          break;
        }
      }

      if (!value) value = StructuredData::MemAllocator.duplicateString(entry);
      dataVector.push_back(value);
    }

    // Map data back to the game structure
    Game::StructuredDataEnumEntry* indices = StructuredData::MemAllocator.allocateArray<Game::StructuredDataEnumEntry>(dataVector.size());
    for (unsigned short i = 0; i < dataVector.size(); ++i)
    {
      indices[i].index = i;
      indices[i].string = dataVector[i];
    }

    // Sort alphabetically
    qsort(indices, dataVector.size(), sizeof(Game::StructuredDataEnumEntry), [](void const* first, void const* second)
      {
        const Game::StructuredDataEnumEntry* entry1 = reinterpret_cast<const Game::StructuredDataEnumEntry*>(first);
        const Game::StructuredDataEnumEntry* entry2 = reinterpret_cast<const Game::StructuredDataEnumEntry*>(second);

        return std::string(entry1->string).compare(entry2->string);
      });

    // Apply our patches
    dataEnum->entryCount = static_cast<int>(dataVector.size());
    dataEnum->entries = indices;
  }

  void StructuredData::PatchCustomClassLimit(Game::StructuredDataDef* data, int count)
  {
    constexpr auto CLASS_ARRAY = 5;
    const auto originalClassCount = data->indexedArrays[CLASS_ARRAY].arraySize;

    if (count != originalClassCount)
    {
      const int customClassSize = 64;

      // We need to recreate the struct list cause it's used in order definitions
      auto newStructs = StructuredData::MemAllocator.allocateArray<Game::StructuredDataStruct>(data->structCount);
      std::memcpy(newStructs, data->structs, data->structCount * sizeof(Game::StructuredDataStruct));
      data->structs = newStructs;

      constexpr auto structIndex = 0;
      {
        auto strct = &data->structs[structIndex];

        auto newProperties = StructuredData::MemAllocator.allocateArray<Game::StructuredDataStructProperty>(strct->propertyCount);
        std::memcpy(newProperties, strct->properties, strct->propertyCount * sizeof(Game::StructuredDataStructProperty));
        strct->properties = newProperties;

        for (int propertyIndex = 0; propertyIndex < strct->propertyCount; ++propertyIndex)
        {
          // 3643 is the offset of the customClasses structure
          if (strct->properties[propertyIndex].offset >= 3643)
          {
            // -10 because 10 is the default amount of custom classes.
            strct->properties[propertyIndex].offset += ((count - originalClassCount) * customClassSize);
          }
        }
      }

      // update structure size
      data->size += ((count - originalClassCount) * customClassSize);

      // Update amount of custom classes
      if (data->indexedArrays[CLASS_ARRAY].arraySize != count)
      {
        // We need to recreate the whole array - this reference could be reused accross definitions
        auto newIndexedArray = StructuredData::MemAllocator.allocateArray<Game::StructuredDataIndexedArray>(data->indexedArrayCount);
        std::memcpy(newIndexedArray, data->indexedArrays, data->indexedArrayCount * sizeof(Game::StructuredDataIndexedArray));
        data->indexedArrays = newIndexedArray;

        // Add classes
        data->indexedArrays[CLASS_ARRAY].arraySize = count;
      }
    }
  }

  void StructuredData::PatchAdditionalData(Game::StructuredDataDef* data, std::unordered_map<std::string, std::string>& patches)
  {
    for (auto& item : patches)
    {
      if (item.first == "classes")
      {
        StructuredData::PatchCustomClassLimit(data, atoi(item.second.data()));
      }
    }
  }

  bool StructuredData::UpdateVersionOffsets(Game::StructuredDataDefSet* set, Game::StructuredDataBuffer* buffer, Game::StructuredDataDef* whatever)
  {
    StructuredData::StatsPlayerDataDefs = set->defs;

    if (set->defCount > 1)
    {
      int bufferVersion = *reinterpret_cast<int*>(buffer->data);

      for (size_t i = 0; i < set->defCount; i++)
      {
        if (set->defs[i].version == bufferVersion)
        {
          if (i == 0)
          {
            // No update to conduct
          }
          else
          {
            Game::StructuredDataDef* newDef = &set->defs[i - 1];
            Game::StructuredDataDef* oldDef = &set->defs[i];

            if (newDef->version == 159 && oldDef->version <= 158)
            {
              // this should move the data 320 bytes infront
              std::memmove(&buffer->data[3963], &buffer->data[3643], oldDef->size - 3643);
            }
            else if (newDef->version > 159 && false)
            {
              // 159 cannot be translated upwards and it's hard to say why
              // Reading it fails in various ways
              // might be the funky class bump...?

              Command::Execute("setPlayerData prestige 10");
              Command::Execute("setPlayerData experience 2516000");
            }

            return Utils::Hook::Call<bool(void*, void*, void*)>(0x140281700)(set, buffer, whatever);
          }
        }
      }

      return Utils::Hook::Call<bool(void*, void*, void*)>(0x140281700)(set, buffer, whatever);
    }

    // StructuredData_UpdateVersion
    return Utils::Hook::Call<bool(void*, void*, void*)>(0x140281700)(set, buffer, whatever);
  }

  StructuredData::StructuredData()
  {
    if (Dedicated::IsEnabled()) return;

    // Note that on x86 we do not execute this when building zones, which x64
    // can't.
    //
    {
      // Correctly upgrade stats
      Utils::Hook(0x1401F7594, StructuredData::UpdateVersionOffsets, HOOK_CALL).install()->quick();

      // 15 or more custom classes
      Utils::Hook::Set<BYTE>(0x1401FA32A, NUM_CUSTOM_CLASSES);

      AssetHandler::OnLoad([](Game::XAssetType type, Game::XAssetHeader asset, const std::string& filename, bool* /*restrict*/)
        {
          if (type != Game::ASSET_TYPE_STRUCTURED_DATA_DEF || filename != "mp/playerdata.def") return;

          if (FastFiles::Current() == "mod")
          {
            StructuredData::ModFsGame = (*Game::fs_gameDirVar)->current.string;
          }
          else
          {
            StructuredData::StockPlayerDataDef = *asset.structuredDataDefSet;
            StructuredData::ModFsGame.clear();
          }
        });

      AssetHandler::OnFind(Game::ASSET_TYPE_STRUCTURED_DATA_DEF, [](Game::XAssetType, const std::string& filename)
        {
          Game::XAssetHeader header = { nullptr };

          if (filename == "mp/playerdata.def" && FastFiles::Ready() && !StructuredData::ModFsGame.empty() && StructuredData::ModFsGame != (*Game::fs_gameDirVar)->current.string)
          {
            header.structuredDataDefSet = &StructuredData::StockPlayerDataDef;
          }

          return header;
        });

      Scheduler::Loop([]
        {
          if (!StructuredData::StatsPlayerDataDefs || !FastFiles::Ready()) return;

          const auto* set = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_STRUCTURED_DATA_DEF, "mp/playerdata.def").structuredDataDefSet;

          if (!set || set->defs == StructuredData::StatsPlayerDataDefs)
            return;

          if (Game::CL_GetLocalClientConnectionState(0) >= Game::CA_CONNECTING)
            return;

          StructuredData::StatsPlayerDataDefs = set->defs;
          Utils::Hook::Call<void(int)>(0x1402A70F0)(0);
        }, Scheduler::Pipeline::MAIN, 500ms);
    }

    // Note that on x86 we also patch the player data definition with the
    // versions of IW4x when ZoneBuilder builds the zones, which x64 lacks.
  }
}
