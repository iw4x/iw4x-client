#include "STDInclude.hpp"

namespace Components
{
	std::map<std::string, Game::StringTable*> StringTable::StringTableMap;

	int StringTable::Hash(const char* data)
	{
		int hash = 0;

		while (*data != 0)
		{
			hash = tolower(*data) + (31 * hash);

			data++;
		}

		return hash;
	}

	Game::StringTable* StringTable::LoadObject(const char* filename)
	{
		Game::StringTable* table = nullptr;
		FileSystem::File rawTable(filename);

		if (rawTable.Exists())
		{
			Utils::CSV parsedTable(rawTable.GetBuffer(), false);

			table = Utils::Memory::AllocateArray<Game::StringTable>(1);

			if (table)
			{
				table->name = Utils::Memory::DuplicateString(filename);
				table->columnCount = parsedTable.GetColumns();
				table->rowCount = parsedTable.GetRows();

				table->values = Utils::Memory::AllocateArray<Game::StringTableCell>(table->columnCount * table->rowCount);

				if (!table->values)
				{
					Utils::Memory::Free(table);
					return nullptr;
				}

				for (int i = 0; i < table->rowCount; i++)
				{
					for (int j = 0; j < table->columnCount; j++)
					{
						Game::StringTableCell* cell = &table->values[i * table->columnCount + j];
						cell->hash = StringTable::Hash(parsedTable.GetElementAt(i, j).data());
						cell->string = Utils::Memory::DuplicateString(parsedTable.GetElementAt(i, j));
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
		// Disable StringTable loading until our StructuredData handler is finished!
		return;

		AssetHandler::OnFind(Game::XAssetType::ASSET_TYPE_STRINGTABLE, [] (Game::XAssetType, const char* filename)
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
		for (auto i = StringTable::StringTableMap.begin(); i != StringTable::StringTableMap.end(); i++)
		{
			Game::StringTable* table = i->second;
			if (table)
			{
				if (table->values)
				{
					for (int i = 0; i < table->rowCount * table->columnCount; i++)
					{
						if (table->values[i].string)
						{
							Utils::Memory::Free(table->values[i].string);
						}
					}

					Utils::Memory::Free(table->values);
				}

				Utils::Memory::Free(table);
			}
		}

		StringTable::StringTableMap.clear();
	}
}
