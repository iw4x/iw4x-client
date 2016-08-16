#include "STDInclude.hpp"

namespace Components
{
	Utils::Memory::Allocator StringTable::MemAllocator;
	std::map<std::string, Game::StringTable*> StringTable::StringTableMap;

	int StringTable::Hash(const char* data)
	{
		int hash = 0;

		while (*data != 0)
		{
			hash = tolower(*data) + (31 * hash);

			++data;
		}

		return hash;
	}

	Game::StringTable* StringTable::LoadObject(std::string filename)
	{
		Game::StringTable* table = nullptr;
		FileSystem::File rawTable(filename);

		if (rawTable.Exists())
		{
			Utils::CSV parsedTable(rawTable.GetBuffer(), false, false);

			table = StringTable::MemAllocator.Allocate<Game::StringTable>();

			if (table)
			{
				table->name = StringTable::MemAllocator.DuplicateString(filename);
				table->columnCount = parsedTable.GetColumns();
				table->rowCount = parsedTable.GetRows();

				table->values = StringTable::MemAllocator.AllocateArray<Game::StringTableCell>(table->columnCount * table->rowCount);

				if (!table->values)
				{
					return nullptr;
				}

				for (int i = 0; i < table->rowCount; ++i)
				{
					for (int j = 0; j < table->columnCount; ++j)
					{
						std::string value = parsedTable.GetElementAt(i, j);

						Game::StringTableCell* cell = &table->values[i * table->columnCount + j];
						cell->hash = StringTable::Hash(value.data());
						cell->string = StringTable::MemAllocator.DuplicateString(value);
						//if (!cell->string) cell->string = ""; // We have to assume it allocated successfully
					}
				}

				StringTable::StringTableMap[filename] = table;
			}
		}
		else
		{
			StringTable::StringTableMap[filename] = nullptr;
		}

		return table;
	}

	StringTable::StringTable()
	{
		AssetHandler::OnFind(Game::XAssetType::ASSET_TYPE_STRINGTABLE, [] (Game::XAssetType, std::string filename)
		{
			Game::XAssetHeader header = { 0 };

			if (StringTable::StringTableMap.find(filename) != StringTable::StringTableMap.end())
			{
				header.stringTable = StringTable::StringTableMap[filename];
			}
			else
			{
				header.stringTable = StringTable::LoadObject(filename);
			}

			return header;
		});
	}

	StringTable::~StringTable()
	{
		StringTable::StringTableMap.clear();
		StringTable::MemAllocator.Clear();
	}
}
