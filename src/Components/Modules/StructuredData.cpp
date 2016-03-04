#include "STDInclude.hpp"

namespace Components
{
	StructuredData* StructuredData::Singleton = nullptr;

	int StructuredData::IndexCount[StructuredData::ENUM_MAX];
	Game::StructuredDataEnumEntry* StructuredData::Indices[StructuredData::ENUM_MAX];
	std::vector<StructuredData::EnumEntry> StructuredData::Entries[StructuredData::ENUM_MAX];

	void StructuredData::AddPlayerDataEntry(StructuredData::PlayerDataType type, int index, std::string name)
	{
		if (type >= StructuredData::ENUM_MAX) return;

		// Check for duplications
		for (auto entry : StructuredData::Entries[type])
		{
			if (entry.name == name || entry.statOffset == index)
			{
				return;
			}
		}

		StructuredData::Entries[type].push_back({ name, index });
	}

	void StructuredData::PatchPlayerDataEnum(Game::StructuredDataDefSet* data, StructuredData::PlayerDataType type, std::vector<StructuredData::EnumEntry>& entries)
	{
		if (!data || !data->data || type >= StructuredData::ENUM_MAX) return;

		Game::StructuredDataEnum* dataEnum = &data->data->enums[type];

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
				if (StructuredData::Indices[type][pos].key == entries[i].name)
				{
					Logger::Error("Duplicate playerdatadef entry found: %s", entries[i].name.data());
				}

				// We found our position
				if (entries[i].name < StructuredData::Indices[type][pos].key)
				{
					break;
				}
			}

			for (unsigned int j = dataEnum->numIndices + i; j > pos && j < static_cast<unsigned int>(StructuredData::IndexCount[type]); j--)
			{
				memcpy(&StructuredData::Indices[type][j], &StructuredData::Indices[type][j - 1], sizeof(Game::StructuredDataEnumEntry));
			}

			StructuredData::Indices[type][pos].index = entries[i].statOffset + lastIndex;
			StructuredData::Indices[type][pos].key = StructuredData::GetSingleton()->MemAllocator.DuplicateString(entries[i].name);
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
					for (int i = 0; i < ARR_SIZE(StructuredData::Entries); ++i)
					{
						if (StructuredData::Entries[i].size())
						{
							StructuredData::PatchPlayerDataEnum(data, static_cast<StructuredData::PlayerDataType>(i), StructuredData::Entries[i]);
						}
					}
				}
			}

			return header;
		});

		// ---------- Weapons ----------
		StructuredData::AddPlayerDataEntry(StructuredData::ENUM_WEAPONS, 1, "m40a3");
		StructuredData::AddPlayerDataEntry(StructuredData::ENUM_WEAPONS, 2, "ak47classic");
		//StructuredData::AddPlayerDataEntry(StructuredData::ENUM_WEAPONS, 3, "ak74u");
		//StructuredData::AddPlayerDataEntry(StructuredData::ENUM_WEAPONS, 4, "peacekeeper");

		// ---------- Cardicons ----------
		StructuredData::AddPlayerDataEntry(StructuredData::ENUM_CARDICONS, 1, "cardicon_rtrolling");

		// ---------- Cardtitles ----------
		StructuredData::AddPlayerDataEntry(StructuredData::ENUM_CARDTITLES, 1, "cardtitle_evilchicken");
		StructuredData::AddPlayerDataEntry(StructuredData::ENUM_CARDTITLES, 2, "cardtitle_nolaststand");
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
