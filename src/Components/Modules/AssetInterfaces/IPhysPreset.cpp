#include <STDInclude.hpp>
#include "IPhysPreset.hpp"

namespace Assets
{
	void IPhysPreset::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::PhysPreset, 44);

		Utils::Stream* buffer = builder->getBuffer();
		Game::PhysPreset* asset = header.physPreset;
		Game::PhysPreset* dest = buffer->dest<Game::PhysPreset>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		if (asset->sndAliasPrefix)
		{
			buffer->saveString(asset->sndAliasPrefix);
			Utils::Stream::ClearPointer(&dest->sndAliasPrefix);
		}

		buffer->popBlock();
	}
}
