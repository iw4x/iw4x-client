#include <STDInclude.hpp>

namespace Assets
{
	void IStringTable::Save_StringTableCellArray(Components::ZoneBuilder::Zone* builder, Game::StringTableCell* values, int count)
	{
		Assert_Size(Game::StringTableCell, 8);

		Utils::Stream* buffer = builder->GetBuffer();

		Game::StringTableCell* destValues = buffer->Dest<Game::StringTableCell>();
		buffer->SaveArray(destValues, count);

		for (int i = 0; i < count; ++i)
		{
			Game::StringTableCell* destValue = &destValues[i];
			Game::StringTableCell* value = &values[i];

			buffer->SaveString(value->string);
			Utils::Stream::ClearPointer(&destValue->string);
		}
	}

	void IStringTable::Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Assert_Size(Game::StringTable, 16);

		Utils::Stream* buffer = builder->GetBuffer();
		Game::StringTable* asset = header.stringTable;
		Game::StringTable* dest = buffer->Dest<Game::StringTable>();
		buffer->Save(asset, sizeof(Game::StringTable));

		buffer->PushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->SaveString(builder->GetAssetName(this->GetType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		if (asset->values)
		{
			buffer->Align(Utils::Stream::ALIGN_4);

			IStringTable::Save_StringTableCellArray(builder, asset->values, asset->columnCount * asset->rowCount);
			Utils::Stream::ClearPointer(&dest->values);
		}

		buffer->PopBlock();
	}
}
