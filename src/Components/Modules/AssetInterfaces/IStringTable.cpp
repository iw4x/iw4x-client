#include <STDInclude.hpp>
#include "IStringTable.hpp"

namespace Assets
{
	void IStringTable::saveStringTableCellArray(Components::ZoneBuilder::Zone* builder, Game::StringTableCell* values, int count)
	{
		AssertSize(Game::StringTableCell, 8);

		Utils::Stream* buffer = builder->getBuffer();

		Game::StringTableCell* destValues = buffer->dest<Game::StringTableCell>();
		buffer->saveArray(destValues, count);

		for (int i = 0; i < count; ++i)
		{
			Game::StringTableCell* destValue = &destValues[i];
			Game::StringTableCell* value = &values[i];

			buffer->saveString(value->string);
			Utils::Stream::ClearPointer(&destValue->string);
		}
	}

	void IStringTable::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::StringTable, 16);

		Utils::Stream* buffer = builder->getBuffer();
		Game::StringTable* asset = header.stringTable;
		Game::StringTable* dest = buffer->dest<Game::StringTable>();
		buffer->save(asset, sizeof(Game::StringTable));

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		if (asset->values)
		{
			buffer->align(Utils::Stream::ALIGN_4);

			this->saveStringTableCellArray(builder, asset->values, asset->columnCount * asset->rowCount);
			Utils::Stream::ClearPointer(&dest->values);
		}

		buffer->popBlock();
	}
}
