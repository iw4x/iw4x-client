#include <STDInclude.hpp>

namespace Assets
{
	void ILocalizedEntry::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::LocalizedEntry, 8);

		Utils::Stream* buffer = builder->getBuffer();
		Game::LocalizedEntry* asset = header.localize;
		Game::LocalizedEntry* dest = buffer->dest<Game::LocalizedEntry>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->value)
		{
			buffer->saveString(asset->value);
			Utils::Stream::ClearPointer(&dest->value);
		}

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		buffer->popBlock();
	}
}
