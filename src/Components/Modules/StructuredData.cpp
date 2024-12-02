#include <STDInclude.hpp>
#include "StructuredData.hpp"

namespace Components
{
	constexpr auto BASE_PLAYERSTATS_VERSION = 155;

	Utils::Memory::Allocator StructuredData::MemAllocator;

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
		dataEnum->entryCount = dataVector.size();
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

						return Utils::Hook::Call<bool(void*, void*, void*)>(0x456830)(set, buffer, whatever);
					}
				}
			}

			return Utils::Hook::Call<bool(void*, void*, void*)>(0x456830)(set, buffer, whatever);
		}

		// StructuredData_UpdateVersion
		return Utils::Hook::Call<bool(void*, void*, void*)>(0x456830)(set, buffer, whatever);
	}

	StructuredData::StructuredData()
	{
		if (Dedicated::IsEnabled()) return;

		// Do not execute this when building zones
		if (!ZoneBuilder::IsEnabled())
		{
			// Correctly upgrade stats
			Utils::Hook(0x42F088, StructuredData::UpdateVersionOffsets, HOOK_CALL).install()->quick();

			// 15 or more custom classes
			Utils::Hook::Set<BYTE>(0x60A2FE, NUM_CUSTOM_CLASSES);

			return;
		}


		// TODO: Since all of the following is zonebuilder-only code, move it to IW4OF or IStructuredDataDefSet.cpp
		AssetHandler::OnLoad([](Game::XAssetType type, Game::XAssetHeader asset, const std::string& filename, bool* /*restrict*/)
			{
				if (ZoneBuilder::IsDumpingZone()) {
					return;
				}

				// Only intercept playerdatadef loading
				if (type != Game::ASSET_TYPE_STRUCTURED_DATA_DEF || filename != "mp/playerdata.def") return;

				// Store asset
				Game::StructuredDataDefSet* data = asset.structuredDataDefSet;
				if (!data) return;

				if (data->defCount != 1)
				{
					Logger::Error(Game::ERR_FATAL, "PlayerDataDefSet contains more than 1 definition!");
					return;
				}

				if (data->defs[0].version != BASE_PLAYERSTATS_VERSION)
				{
					Logger::Error(Game::ERR_FATAL, "Initial PlayerDataDef is not version 155, patching not possible!");
					return;
				}

				std::unordered_map<int, std::vector<std::vector<std::string>>> patchDefinitions;
				std::unordered_map<int, std::unordered_map<std::string, std::string>> otherPatchDefinitions;

				// First check if all versions are present
				for (int i = 156;; ++i)
				{
					// We're on DB thread (OnLoad) so use DB thread for FS
					FileSystem::File definition(std::format("{}/{}.json", filename, i), Game::FsThread::FS_THREAD_DATABASE);
					if (!definition.exists()) break;

					std::vector<std::vector<std::string>> enumContainer;
					std::unordered_map<std::string, std::string> otherPatches;

					nlohmann::json defData;
					try
					{
						defData = nlohmann::json::parse(definition.getBuffer());
					}
					catch (const nlohmann::json::parse_error& ex)
					{
						Logger::PrintError(Game::CON_CHANNEL_ERROR, "JSON Parse Error: {}\n", ex.what());
						return;
					}

					if (!defData.is_object())
					{
						Logger::Error(Game::ERR_FATAL, "PlayerDataDef patch for version {} is invalid!", i);
						return;
					}

					for (auto pType = 0; pType < StructuredData::PlayerDataType::COUNT; ++pType)
					{
						auto enumData = defData[StructuredData::EnumTranslation[pType]];

						std::vector<std::string> entryData;

						if (enumData.is_array())
						{
							for (const auto& rawEntry : enumData)
							{
								if (rawEntry.is_string())
								{
									entryData.push_back(rawEntry.get<std::string>());
								}
							}
						}

						enumContainer.push_back(entryData);
					}

					auto other = defData["other"];

					if (other.is_object())
					{
						for (auto& item : other.items())
						{
							if (item.value().is_string())
							{
								otherPatches[item.key()] = item.value().get<std::string>();
							}
						}
					}

					patchDefinitions[i] = enumContainer;
					otherPatchDefinitions[i] = otherPatches;
				}

				// Nothing to patch
				if (patchDefinitions.empty()) return;

				// Reallocate the definition
				auto* newData = StructuredData::MemAllocator.allocateArray<Game::StructuredDataDef>(data->defCount + patchDefinitions.size());
				std::memcpy(&newData[patchDefinitions.size()], data->defs, sizeof(Game::StructuredDataDef) * data->defCount);

				// Set the versions
				for (unsigned int i = 0; i < patchDefinitions.size(); ++i)
				{
					newData[i].version = (patchDefinitions.size() - i) + BASE_PLAYERSTATS_VERSION;
				}

				// Apply new data
				data->defs = newData;
				data->defCount += patchDefinitions.size();

				// Patch the definition
				for (int i = data->defCount - 1; i >= 0; --i)
				{
					// No need to patch version 155
					if (newData[i].version == BASE_PLAYERSTATS_VERSION)
					{
						continue;
					}

					// We start from the previous one
					const auto version = newData[i].version;
					data->defs[i] = data->defs[i + 1];
					newData[i].version = version;

					if (patchDefinitions.contains(newData[i].version))
					{
						auto patchData = patchDefinitions[newData[i].version];
						auto otherData = otherPatchDefinitions[newData[i].version];

						// Invalid patch data
						if (patchData.size() != StructuredData::PlayerDataType::COUNT)
						{
							Logger::Error(Game::ERR_FATAL, "PlayerDataDef patch for version {} wasn't parsed correctly!", newData[i].version);
							continue;
						}

						// Apply the patch data
						for (auto pType = 0; pType < StructuredData::PlayerDataType::COUNT; ++pType)
						{
							if (!patchData[pType].empty())
							{
								StructuredData::PatchPlayerDataEnum(&newData[i], static_cast<StructuredData::PlayerDataType>(pType), patchData[pType]);
							}
						}

						StructuredData::PatchAdditionalData(&newData[i], otherData);
					}
				}
			});
	}
}
