#include <STDInclude.hpp>

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

			if (enum_->indices)
			{
				AssertSize(Game::StructuredDataEnumEntry, 8);
				buffer->align(Utils::Stream::ALIGN_4);

				Game::StructuredDataEnumEntry* destIndices = buffer->dest<Game::StructuredDataEnumEntry>();
				buffer->saveArray(enum_->indices, enum_->numIndices);

				for (int j = 0; j < enum_->numIndices; ++j)
				{
					Game::StructuredDataEnumEntry* destIndex = &destIndices[j];
					Game::StructuredDataEnumEntry* index = &enum_->indices[j];

					if (index->key)
					{
						buffer->saveString(index->key);
						Utils::Stream::ClearPointer(&destIndex->key);
					}
				}

				Utils::Stream::ClearPointer(&destEnum->indices);
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

			if (struct_->property)
			{
				AssertSize(Game::StructuredDataStructProperty, 16);
				buffer->align(Utils::Stream::ALIGN_4);

				Game::StructuredDataStructProperty* destProperties = buffer->dest<Game::StructuredDataStructProperty>();
				buffer->saveArray(struct_->property, struct_->numProperties);

				for (int j = 0; j < struct_->numProperties; ++j)
				{
					Game::StructuredDataStructProperty* destProperty = &destProperties[j];
					Game::StructuredDataStructProperty* property = &struct_->property[j];

					if (property->name)
					{
						buffer->saveString(property->name);
						Utils::Stream::ClearPointer(&destProperty->name);
					}
				}

				Utils::Stream::ClearPointer(&destStruct->property);
			}
		}
	}

	void IStructuredDataDefSet::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::StructuredDataDefSet, 12);

		Utils::Stream* buffer = builder->getBuffer();
		Game::StructuredDataDefSet* asset = header.structuredData;
		Game::StructuredDataDefSet* dest = buffer->dest<Game::StructuredDataDefSet>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		if (asset->data)
		{
			AssertSize(Game::StructuredDataDef, 52);
			buffer->align(Utils::Stream::ALIGN_4);

			Game::StructuredDataDef* destDataArray = buffer->dest<Game::StructuredDataDef>();
			buffer->saveArray(asset->data, asset->count);

			for (int i = 0; i < asset->count; ++i)
			{
				Game::StructuredDataDef* destData = &destDataArray[i];
				Game::StructuredDataDef* data = &asset->data[i];

				if (data->enums)
				{
					AssertSize(Game::StructuredDataEnum, 12);
					buffer->align(Utils::Stream::ALIGN_4);

					this->saveStructuredDataEnumArray(data->enums, data->numEnums, builder);
					Utils::Stream::ClearPointer(&destData->enums);
				}

				if (data->structs)
				{
					AssertSize(Game::StructuredDataStruct, 16);
					buffer->align(Utils::Stream::ALIGN_4);

					this->saveStructuredDataStructArray(data->structs, data->numStructs, builder);
					Utils::Stream::ClearPointer(&destData->structs);
				}

				if (data->indexedArrays)
				{
					AssertSize(Game::StructuredDataIndexedArray, 16);
					buffer->align(Utils::Stream::ALIGN_4);

					buffer->saveArray(data->indexedArrays, data->numIndexedArrays);
					Utils::Stream::ClearPointer(&destData->indexedArrays);
				}

				if (data->enumArrays)
				{
					AssertSize(Game::StructuredDataEnumedArray, 16);
					buffer->align(Utils::Stream::ALIGN_4);

					buffer->saveArray(data->enumArrays, data->numEnumArrays);
					Utils::Stream::ClearPointer(&destData->enumArrays);
				}
			}

			Utils::Stream::ClearPointer(&dest->data);
		}

		buffer->popBlock();
	}
}
