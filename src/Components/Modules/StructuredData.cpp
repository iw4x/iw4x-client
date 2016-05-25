#include "STDInclude.hpp"

namespace Components
{
	StructuredData* StructuredData::Singleton = nullptr;

	int StructuredData::IndexCount[StructuredData::ENUM_MAX];
	Game::StructuredDataEnumEntry* StructuredData::Indices[StructuredData::ENUM_MAX];
	std::vector<std::string> StructuredData::Entries[StructuredData::ENUM_MAX];

	void StructuredData::AddPlayerDataEntry(StructuredData::PlayerDataType type, std::string name)
	{
		if (type >= StructuredData::ENUM_MAX) return;

		// Check for duplications
		auto v = &StructuredData::Entries[type];
		if (std::find(v->begin(), v->end(), name) == v->end())
		{
			StructuredData::Entries[type].push_back(name);
		}
	}

	void StructuredData::PatchPlayerDataEnum(Game::StructuredDataDef* data, StructuredData::PlayerDataType type, std::vector<std::string>& entries)
	{
		if (!data || type >= StructuredData::ENUM_MAX) return;

		Game::StructuredDataEnum* dataEnum = &data->enums[type];

		if (StructuredData::IndexCount[type])
		{
			dataEnum->numIndices = StructuredData::IndexCount[type];
			dataEnum->indices = StructuredData::Indices[type];
			return;
		}

		// Find last index so we can add our offset to it.
		// This whole procedure is potentially unsafe.
		// If any index changes, everything gets shifted and the stats are fucked.
		int lastIndex = 0;
		for (int i = 0; i < dataEnum->numIndices; ++i)
		{
			if (dataEnum->indices[i].index > lastIndex)
			{
				lastIndex = dataEnum->indices[i].index;
			}
		}

		// Calculate new count
		StructuredData::IndexCount[type] = dataEnum->numIndices + entries.size();

		// Allocate new entries
		StructuredData::Indices[type] = StructuredData::GetSingleton()->MemAllocator.AllocateArray<Game::StructuredDataEnumEntry>(StructuredData::IndexCount[type]);
		memcpy(StructuredData::Indices[type], dataEnum->indices, sizeof(Game::StructuredDataEnumEntry) * dataEnum->numIndices);

		for (unsigned int i = 0; i < entries.size(); ++i)
		{
			unsigned int pos = 0;

			for (; pos < (dataEnum->numIndices + i); pos++)
			{
				if (StructuredData::Indices[type][pos].key == entries[i])
				{
					Logger::Error("Duplicate playerdatadef entry found: %s", entries[i].data());
				}

				// We found our position
				if (entries[i] < StructuredData::Indices[type][pos].key)
				{
					break;
				}
			}

			for (unsigned int j = dataEnum->numIndices + i; j > pos && j < static_cast<unsigned int>(StructuredData::IndexCount[type]); j--)
			{
				memcpy(&StructuredData::Indices[type][j], &StructuredData::Indices[type][j - 1], sizeof(Game::StructuredDataEnumEntry));
			}

			StructuredData::Indices[type][pos].index = i + lastIndex;
			StructuredData::Indices[type][pos].key = StructuredData::GetSingleton()->MemAllocator.DuplicateString(entries[i]);
		}

		// Apply our patches
		dataEnum->numIndices = StructuredData::IndexCount[type];
		dataEnum->indices = StructuredData::Indices[type];
	}

	StructuredData* StructuredData::GetSingleton()
	{
		if (!StructuredData::Singleton)
		{
			Logger::Error("StructuredData singleton is null!");
		}

		return StructuredData::Singleton;
	}

	StructuredData::StructuredData()
	{
		// Only execute this when building zones
		if (!ZoneBuilder::IsEnabled()) return;

		// TODO: Increment the version once you change something below
		static int version = 156;

		// ---------- Weapons ----------
		StructuredData::AddPlayerDataEntry(StructuredData::ENUM_WEAPONS, "m40a3");
		StructuredData::AddPlayerDataEntry(StructuredData::ENUM_WEAPONS, "ak47classic");
		//StructuredData::AddPlayerDataEntry(StructuredData::ENUM_WEAPONS, "ak74u");
		//StructuredData::AddPlayerDataEntry(StructuredData::ENUM_WEAPONS, "peacekeeper");

		// ---------- Cardicons ----------
		StructuredData::AddPlayerDataEntry(StructuredData::ENUM_CARDICONS, "cardicon_rtrolling");

		// ---------- Cardtitles ----------
		StructuredData::AddPlayerDataEntry(StructuredData::ENUM_CARDTITLES, "cardtitle_evilchicken");
		StructuredData::AddPlayerDataEntry(StructuredData::ENUM_CARDTITLES, "cardtitle_nolaststand");


		// Patch handing and initialization

		StructuredData::Singleton = this;
		ZeroMemory(StructuredData::IndexCount, sizeof(StructuredData));

		// TODO: Write these into fastfiles and only hotpatch them when building fastfiles!
		AssetHandler::OnFind(Game::XAssetType::ASSET_TYPE_STRUCTUREDDATADEF, [] (Game::XAssetType type, std::string filename)
		{
			Game::XAssetHeader header = { 0 };

			if (filename == "mp/playerdata.def")
			{
				Game::StructuredDataDefSet* data = AssetHandler::FindOriginalAsset(Game::XAssetType::ASSET_TYPE_STRUCTUREDDATADEF, filename.data()).structuredData;
				header.structuredData = data;

				if (data)
				{
					// First check if all versions are present
					for (int ver = 155; ver <= version; ++ver)
					{
						bool foundVersion = false;

						for (int i = 0; i < data->count; ++i)
						{
							if (data->data[i].version == ver)
							{
								foundVersion = true;
								break;
							}
						}

						if (foundVersion && ver == version)
						{
							//Logger::Print("PlayerDataDef already patched for version %d, no need to patch anything!\n", version);
							return header;
						}
						else if (!foundVersion && ver != version)
						{
							Logger::Error("PlayerDataDef for version %d is missing!", ver);
							return header;
						}
					}

					if (data->data[data->count - 1].version != 155)
					{
						Logger::Error("Initial PlayerDataDef is not version 155, patching not possible!");
						return header;
					}

					Logger::Print("Patching PlayerDataDef for version %d...\n", version);

					// Reallocate the definition
					Game::StructuredDataDef* newData = StructuredData::GetSingleton()->MemAllocator.AllocateArray<Game::StructuredDataDef>(data->count + 1);
					memcpy(&newData[1], data->data, sizeof Game::StructuredDataDef * data->count);

					// Fill our new definition with the base data (155)
					memcpy(newData, &data->data[data->count - 1], sizeof Game::StructuredDataDef);
					newData->version = version;

					// Reallocate the enum array
					Game::StructuredDataEnum* newEnums = StructuredData::GetSingleton()->MemAllocator.AllocateArray<Game::StructuredDataEnum>(newData->numEnums);
					memcpy(newEnums, newData->enums, sizeof Game::StructuredDataEnum * newData->numEnums);
					newData->enums = newEnums;

					// Patch the definition
					for (int i = 0; i < ARR_SIZE(StructuredData::Entries); ++i)
					{
						if (StructuredData::Entries[i].size())
						{
							StructuredData::PatchPlayerDataEnum(newData, static_cast<StructuredData::PlayerDataType>(i), StructuredData::Entries[i]);
						}
					}

					// Apply the patch
					data->data = newData;
					data->count++;
				}
			}

			return header;
		});
	}

	StructuredData::~StructuredData()
	{
		for (int i = 0; i < ARR_SIZE(StructuredData::Entries); ++i)
		{
			StructuredData::Entries[i].clear();
		}

		StructuredData::Singleton = nullptr;
	}
}
