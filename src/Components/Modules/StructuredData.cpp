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

		std::vector<std::string> rebaseEntries;
		for (unsigned int i = 0; i < entries.size(); ++i)
		{
			for (int pos = 0; pos < dataEnum->numIndices; ++pos)
			{
				if (dataEnum->indices[pos].key == entries[i])
				{
					rebaseEntries.push_back(entries[i]);
				}
			}
		}

		// Calculate new count
		unsigned int indexCount = dataEnum->numIndices + entries.size() - rebaseEntries.size();

		// Allocate new entries
		Game::StructuredDataEnumEntry* indices = StructuredData::MemAllocator.AllocateArray<Game::StructuredDataEnumEntry>(indexCount);
		std::memcpy(indices, dataEnum->indices, sizeof(Game::StructuredDataEnumEntry) * dataEnum->numIndices);

		int skipped = 0;
		for (unsigned int i = 0; i < entries.size(); ++i)
		{
			unsigned int pos = 0;

			for (; pos < (dataEnum->numIndices + i - rebaseEntries.size()); ++pos)
			{
				if (indices[pos].key == entries[i])
				{
					Logger::Print("Playerdatadef entry %s will be rebased!\n", entries[i].data());
					pos = 0xFFFFFFFF;
					break;
					//Logger::Error("Duplicate playerdatadef entry found: %s", entries[i].data());
				}

				// We found our position
				if (entries[i] < indices[pos].key)
				{
					break;
				}
			}

			// Rebasing needed
			if (pos == 0xFFFFFFFF)
			{
				++skipped;
				continue;
			}

			// TODO directly shift the data using memmove
			for (unsigned int j = dataEnum->numIndices + i - skipped; j > pos && j < indexCount; --j)
			{
				std::memcpy(&indices[j], &indices[j - 1], sizeof(Game::StructuredDataEnumEntry));
			}

			indices[pos].index = i + lastIndex - skipped;
			indices[pos].key = StructuredData::MemAllocator.DuplicateString(entries[i]);
		}

		// Meh, but best way for now
		for (unsigned int i = 0; i < rebaseEntries.size(); ++i)
		{
			int index = -1;
			unsigned int pos = 0;
			for (; pos < indexCount; ++pos)
			{
				if (indices[pos].key == rebaseEntries[i])
				{
					index = indices[pos].index;
					indices[pos].index = entries.size() + lastIndex - rebaseEntries.size() + i;
					break;
				}
			}

			if (index < 0)
			{
				Logger::Error("Playerdatadef entry %s cannot be rebased!", rebaseEntries[i].data());
			}

			for (unsigned int j = 0; j < indexCount; ++j)
			{
				if (pos != j && indices[j].index > index)
				{
					indices[j].index--;
				}
			}
		}

		for (unsigned int j = 0; j < indexCount && type == StructuredData::PlayerDataType::ENUM_WEAPONS; ++j)
		{
			OutputDebugStringA(Utils::String::VA("%s: %d\n", indices[j].key, indices[j].index));
		}

		// Apply our patches
		dataEnum->numIndices = indexCount;
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
				FileSystem::File definition(fmt::sprintf("%s/%d.json", filename.data(), i));
				if (!definition.Exists()) break;

				std::vector<std::vector<std::string>> enumContainer;

				std::string errors;
				json11::Json defData = json11::Json::parse(definition.GetBuffer(), errors);

				if (!errors.empty())
				{
					Logger::Error("Parsing patch file '%s' for PlayerDataDef version %d failed: %s", definition.GetName().data(), i, errors.data());
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
			Game::StructuredDataDef* newData = StructuredData::MemAllocator.AllocateArray<Game::StructuredDataDef>(data->count + patchDefinitions.size());
			std::memcpy(&newData[patchDefinitions.size()], data->data, sizeof Game::StructuredDataDef * data->count);

			// Prepare the buffers
			for (unsigned int i = 0; i < patchDefinitions.size(); ++i)
			{
				std::memcpy(&newData[i], data->data, sizeof Game::StructuredDataDef);
				newData[i].version = (patchDefinitions.size() - i) + 155;

				// Reallocate the enum array
				Game::StructuredDataEnum* newEnums = StructuredData::MemAllocator.AllocateArray<Game::StructuredDataEnum>(data->data->numEnums);
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
		StructuredData::MemAllocator.Clear();
	}
}
