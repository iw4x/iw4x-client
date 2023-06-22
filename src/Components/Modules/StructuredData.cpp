#include <STDInclude.hpp>
#include "StructuredData.hpp"

namespace Components
{
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
		const int customClassSize = 64;

		for (int i = 0; i < data->structs[0].propertyCount; ++i)
		{
			// 3003 is the offset of the customClasses structure
			if (data->structs[0].properties[i].offset >= 3643)
			{
				// -10 because 10 is the default amount of custom classes.
				data->structs[0].properties[i].offset += ((count - 10) * customClassSize);
			}
		}

		// update structure size
		data->size += ((count - 10) * customClassSize);

		// Update amount of custom classes
		data->indexedArrays[5].arraySize = count;
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

	bool StructuredData::UpdateVersionOffsets(Game::StructuredDataDefSet *set, Game::StructuredDataBuffer *buffer, Game::StructuredDataDef *whatever)
	{
		Game::StructuredDataDef* newDef = &set->defs[0];
		Game::StructuredDataDef* oldDef = &set->defs[0];

		for (unsigned int i = 0; i < set->defCount; ++i)
		{
			if (newDef->version < set->defs[i].version)
			{
				newDef = &set->defs[i];
			}

			if (set->defs[i].version == *reinterpret_cast<int*>(buffer->data))
			{
				oldDef = &set->defs[i];
			}
		}

		if (newDef->version >= 159 && oldDef->version <= 158)
		{
			// this should move the data 320 bytes infront
			std::memmove(&buffer->data[3963], &buffer->data[3643], oldDef->size - 3643);
		}

		// StructuredData_UpdateVersion
		return Utils::Hook::Call<bool(void*, void*, void*)>(0x456830)(set, buffer, whatever);
	}

	StructuredData::StructuredData()
	{
		if (Dedicated::IsEnabled()) return;

		// Correctly upgrade stats
		Utils::Hook(0x42F088, StructuredData::UpdateVersionOffsets, HOOK_CALL).install()->quick();

		// 15 or more custom classes
		Utils::Hook::Set<BYTE>(0x60A2FE, NUM_CUSTOM_CLASSES);
	}
}
