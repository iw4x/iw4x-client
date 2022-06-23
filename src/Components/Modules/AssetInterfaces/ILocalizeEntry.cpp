#include <STDInclude.hpp>
#include "ILocalizeEntry.hpp"

namespace Assets
{
	void ILocalizeEntry::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::LocalizeEntry, 8);

		Utils::Stream* buffer = builder->getBuffer();
		Game::LocalizeEntry* asset = header.localize;
		Game::LocalizeEntry* dest = buffer->dest<Game::LocalizeEntry>();
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
