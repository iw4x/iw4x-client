#include "STDInclude.hpp"

namespace Components
{
	Utils::Memory::Allocator StructuredData::MemAllocator;

	const char* StructuredData::EnumTranslation[ENUM_MAX] =
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
		if (!data || type >= StructuredData::PlayerDataType::ENUM_MAX) return;

		Game::StructuredDataEnum* dataEnum = &data->enums[type];

		// Build index-sorted data vector
		std::vector<const char*> dataVector;
		for (int i = 0; i < dataEnum->numIndices; ++i)
		{
			int index = 0;
			for (; index < dataEnum->numIndices; ++index)
			{
				if (dataEnum->indices[index].index == i)
				{
					break;
				}
			}

			dataVector.push_back(dataEnum->indices[index].key);
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
					Logger::Print("Playerdatadef entry '%s' will be rebased!\n", value);
					break;
				}
			}

			if (!value) value = StructuredData::MemAllocator.duplicateString(entry);
			dataVector.push_back(value);
		}

		// Map data back to the game structure
		Game::StructuredDataEnumEntry* indices = StructuredData::MemAllocator.allocateArray<Game::StructuredDataEnumEntry>(dataVector.size());
		for (unsigned int i = 0; i < dataVector.size(); ++i)
		{
			indices[i].index = i;
			indices[i].key = dataVector[i];
		}

		// Sort alphabetically
		qsort(indices, dataVector.size(), sizeof(Game::StructuredDataEnumEntry), [] (const void* first, const void* second)
		{
			const Game::StructuredDataEnumEntry* entry1 = reinterpret_cast<const Game::StructuredDataEnumEntry*>(first);
			const Game::StructuredDataEnumEntry* entry2 = reinterpret_cast<const Game::StructuredDataEnumEntry*>(second);

			return std::string(entry1->key).compare(entry2->key);
		});

		// Apply our patches
		dataEnum->numIndices = dataVector.size();
		dataEnum->indices = indices;
	}

	StructuredData::StructuredData()
	{
		// Only execute this when building zones
		if (!ZoneBuilder::IsEnabled()) return;

		AssetHandler::OnLoad([] (Game::XAssetType type, Game::XAssetHeader asset, std::string filename, bool* /*restrict*/)
		{
			// Only intercept playerdatadef loading
			if (filename != "mp/playerdata.def" || type != Game::XAssetType::ASSET_TYPE_STRUCTUREDDATADEF) return;

			// Store asset
			Game::StructuredDataDefSet* data = asset.structuredData;
			if (!data) return;

			if (data->count != 1)
			{
				Logger::Error("PlayerDataDefSet contains more than 1 definition!");
				return;
			}

			if (data->data[0].version != 155)
			{
				Logger::Error("Initial PlayerDataDef is not version 155, patching not possible!");
				return;
			}

			std::map<int, std::vector<std::vector<std::string>>> patchDefinitions;

			// First check if all versions are present
			for (int i = 156;; ++i)
			{
				FileSystem::File definition(Utils::String::VA("%s/%d.json", filename.data(), i));
				if (!definition.exists()) break;

				std::vector<std::vector<std::string>> enumContainer;

				std::string errors;
				json11::Json defData = json11::Json::parse(definition.getBuffer(), errors);

				if (!errors.empty())
				{
					Logger::Error("Parsing patch file '%s' for PlayerDataDef version %d failed: %s", definition.getName().data(), i, errors.data());
					return;
				}

				if (!defData.is_object())
				{
					Logger::Error("PlayerDataDef patch for version %d is invalid!", i);
					return;
				}

				for (unsigned int pType = 0; pType < StructuredData::PlayerDataType::ENUM_MAX; ++pType)
				{
					auto enumData = defData[StructuredData::EnumTranslation[pType]];

					std::vector<std::string> entryData;

					if (enumData.is_array())
					{
						for (auto rawEntry : enumData.array_items())
						{
							if (rawEntry.is_string())
							{
								entryData.push_back(rawEntry.string_value());
							}
						}
					}

					enumContainer.push_back(entryData);
				}

				patchDefinitions[i] = enumContainer;
			}

			// Nothing to patch
			if (patchDefinitions.empty()) return;

			// Reallocate the definition
			Game::StructuredDataDef* newData = StructuredData::MemAllocator.allocateArray<Game::StructuredDataDef>(data->count + patchDefinitions.size());
			std::memcpy(&newData[patchDefinitions.size()], data->data, sizeof Game::StructuredDataDef * data->count);

			// Prepare the buffers
			for (unsigned int i = 0; i < patchDefinitions.size(); ++i)
			{
				std::memcpy(&newData[i], data->data, sizeof Game::StructuredDataDef);
				newData[i].version = (patchDefinitions.size() - i) + 155;

				// Reallocate the enum array
				Game::StructuredDataEnum* newEnums = StructuredData::MemAllocator.allocateArray<Game::StructuredDataEnum>(data->data->numEnums);
				std::memcpy(newEnums, data->data->enums, sizeof Game::StructuredDataEnum * data->data->numEnums);
				newData[i].enums = newEnums;
			}

			// Apply new data
			data->data = newData;
			data->count += patchDefinitions.size();

			// Patch the definition
			for (int i = 0; i < data->count; ++i)
			{
				// No need to patch version 155
				if (newData[i].version == 155) continue;

				if(patchDefinitions.find(newData[i].version) != patchDefinitions.end())
				{
					auto patchData = patchDefinitions[newData[i].version];

					// Invalid patch data
					if (patchData.size() != StructuredData::PlayerDataType::ENUM_MAX)
					{
						Logger::Error("PlayerDataDef patch for version %d wasn't parsed correctly!", newData[i].version);
						continue;
					}

					// Apply the patch data
					for (unsigned int pType = 0; pType < StructuredData::PlayerDataType::ENUM_MAX; ++pType)
					{
						if (!patchData[pType].empty())
						{
							StructuredData::PatchPlayerDataEnum(&newData[i], static_cast<StructuredData::PlayerDataType>(pType), patchData[pType]);
						}
					}
				}
			}
		});
	}

	StructuredData::~StructuredData()
	{
		StructuredData::MemAllocator.clear();
	}
}
