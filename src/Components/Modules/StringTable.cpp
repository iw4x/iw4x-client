#include "StringTable.hpp"
#include "FastFiles.hpp"

namespace Components
{
  std::unordered_map<std::string, Game::StringTable*> StringTable::StringTableMap;
  std::unordered_set<std::string> StringTable::ModTables;
  std::string StringTable::ModFsGame;

  Game::StringTable* StringTable::LoadObject(std::string filename)
  {
    Utils::Memory::Allocator* allocator = Utils::Memory::GetAllocator();

    filename = Utils::String::ToLower(filename);

    Game::StringTable* table = nullptr;
    FileSystem::File rawTable(filename);

    if (rawTable.exists())
    {
      Utils::CSV parsedTable(rawTable.getBuffer(), false, false);

      table = allocator->allocate<Game::StringTable>();

      if (table)
      {
        table->name = allocator->duplicateString(filename);
        table->columnCount = static_cast<int>(parsedTable.getColumns());
        table->rowCount = static_cast<int>(parsedTable.getRows());

        table->values = allocator->allocateArray<Game::StringTableCell>(table->columnCount * table->rowCount);

        if (!table->values)
        {
          return nullptr;
        }

        for (int i = 0; i < table->rowCount; ++i)
        {
          for (int j = 0; j < table->columnCount; ++j)
          {
            std::string value = parsedTable.getElementAt(i, j);

            Game::StringTableCell* cell = &table->values[i * table->columnCount + j];
            cell->hash = Game::StringTable_HashString(value.data());
            cell->string = allocator->duplicateString(value);
            //if (!cell->string) cell->string = ""; // We have to assume it allocated successfully
          }
        }

        StringTableMap[filename] = table;
      }
    }
    else
    {
      StringTableMap[filename] = nullptr;
    }

    return table;
  }

  StringTable::StringTable()
  {
    AssetHandler::OnFind(Game::XAssetType::ASSET_TYPE_STRINGTABLE, [](Game::XAssetType, const std::string& _filename)
    {
      Game::XAssetHeader header = { nullptr };

      std::string filename = Utils::String::ToLower(_filename);

      if (ModTables.contains(filename) && ModFsGame == (*Game::fs_gameDirVar)->current.string)
      {
        return header;
      }

      if (StringTableMap.contains(filename))
      {
        header.stringTable = StringTableMap[filename];
      }
      else
      {
        header.stringTable = LoadObject(filename);
      }

      return header;
    });

    AssetHandler::OnLoad([](Game::XAssetType type, Game::XAssetHeader /*asset*/, const std::string& name, bool* /*restrict*/)
    {
      if (type == Game::XAssetType::ASSET_TYPE_STRINGTABLE && FastFiles::Current() == "mod")
      {
        if (ModFsGame != (*Game::fs_gameDirVar)->current.string)
        {
          ModTables.clear();
          ModFsGame = (*Game::fs_gameDirVar)->current.string;
        }

        ModTables.insert(Utils::String::ToLower(name));
      }
    });
  }
}
