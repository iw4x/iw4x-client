#include <STDInclude.hpp>
#include "IStructuredDataDefSet.hpp"

namespace Assets
{
	void IStructuredDataDefSet::saveStructuredDataEnumArray(Game::StructuredDataEnum* enums, int numEnums, Components::ZoneBuilder::Zone* builder)
	{
		Utils::Stream* buffer = builder->getBuffer();

		Game::StructuredDataEnum* destEnums = buffer->dest<Game::StructuredDataEnum>();
		buffer->saveArray(enums, numEnums);

		for (int i = 0; i < numEnums; ++i)
		{
			Game::StructuredDataEnum* destEnum = &destEnums[i];
			Game::StructuredDataEnum* enum_ = &enums[i];

			if (enum_->entries)
			{
				AssertSize(Game::StructuredDataEnumEntry, 8);
				buffer->align(Utils::Stream::ALIGN_4);

				Game::StructuredDataEnumEntry* destIndices = buffer->dest<Game::StructuredDataEnumEntry>();
				buffer->saveArray(enum_->entries, enum_->entryCount);

				for (int j = 0; j < enum_->entryCount; ++j)
				{
					Game::StructuredDataEnumEntry* destIndex = &destIndices[j];
					Game::StructuredDataEnumEntry* index = &enum_->entries[j];

					if (index->string)
					{
						buffer->saveString(index->string);
						Utils::Stream::ClearPointer(&destIndex->string);
					}
				}

				Utils::Stream::ClearPointer(&destEnum->entries);
			}
		}
	}

	void IStructuredDataDefSet::saveStructuredDataStructArray(Game::StructuredDataStruct* structs, int numStructs, Components::ZoneBuilder::Zone* builder)
	{
		Utils::Stream* buffer = builder->getBuffer();

		Game::StructuredDataStruct* destStructs = buffer->dest<Game::StructuredDataStruct>();
		buffer->saveArray(structs, numStructs);

		for (int i = 0; i < numStructs; ++i)
		{
			Game::StructuredDataStruct* destStruct = &destStructs[i];
			Game::StructuredDataStruct* struct_ = &structs[i];

			if (struct_->properties)
			{
				AssertSize(Game::StructuredDataStructProperty, 16);
				buffer->align(Utils::Stream::ALIGN_4);

				Game::StructuredDataStructProperty* destProperties = buffer->dest<Game::StructuredDataStructProperty>();
				buffer->saveArray(struct_->properties, struct_->propertyCount);

				for (int j = 0; j < struct_->propertyCount; ++j)
				{
					Game::StructuredDataStructProperty* destProperty = &destProperties[j];
					Game::StructuredDataStructProperty* property = &struct_->properties[j];

					if (property->name)
					{
						buffer->saveString(property->name);
						Utils::Stream::ClearPointer(&destProperty->name);
					}
				}

				Utils::Stream::ClearPointer(&destStruct->properties);
			}
		}
	}

	void IStructuredDataDefSet::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::StructuredDataDefSet, 12);

		Utils::Stream* buffer = builder->getBuffer();
		Game::StructuredDataDefSet* asset = header.structuredDataDefSet;
		Game::StructuredDataDefSet* dest = buffer->dest<Game::StructuredDataDefSet>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		if (asset->defs)
		{
			AssertSize(Game::StructuredDataDef, 52);
			buffer->align(Utils::Stream::ALIGN_4);

			Game::StructuredDataDef* destDataArray = buffer->dest<Game::StructuredDataDef>();
			buffer->saveArray(asset->defs, asset->defCount);

			for (unsigned int i = 0; i < asset->defCount; ++i)
			{
				Game::StructuredDataDef* destData = &destDataArray[i];
				Game::StructuredDataDef* data = &asset->defs[i];

				if (data->enums)
				{
					AssertSize(Game::StructuredDataEnum, 12);
					buffer->align(Utils::Stream::ALIGN_4);

					this->saveStructuredDataEnumArray(data->enums, data->enumCount, builder);
					Utils::Stream::ClearPointer(&destData->enums);
				}

				if (data->structs)
				{
					AssertSize(Game::StructuredDataStruct, 16);
					buffer->align(Utils::Stream::ALIGN_4);

					this->saveStructuredDataStructArray(data->structs, data->structCount, builder);
					Utils::Stream::ClearPointer(&destData->structs);
				}

				if (data->indexedArrays)
				{
					AssertSize(Game::StructuredDataIndexedArray, 16);
					buffer->align(Utils::Stream::ALIGN_4);

					buffer->saveArray(data->indexedArrays, data->indexedArrayCount);
					Utils::Stream::ClearPointer(&destData->indexedArrays);
				}

				if (data->enumedArrays)
				{
					AssertSize(Game::StructuredDataEnumedArray, 16);
					buffer->align(Utils::Stream::ALIGN_4);

					buffer->saveArray(data->enumedArrays, data->enumedArrayCount);
					Utils::Stream::ClearPointer(&destData->enumedArrays);
				}
			}

			Utils::Stream::ClearPointer(&dest->defs);
		}

		buffer->popBlock();
	}
}
