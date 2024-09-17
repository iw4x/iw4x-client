#include <STDInclude.hpp>
#include "IPhysPreset.hpp"

namespace Assets
{
	void IPhysPreset::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::PhysPreset, 44);

		auto* buffer = builder->getBuffer();
		auto* asset = header.physPreset;
		auto* dest = buffer->dest<Game::PhysPreset>();
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

	void IPhysPreset::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		loadFromDisk(header, name, builder);
	}

	void IPhysPreset::loadFromDisk(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		header->physPreset = builder->getIW4OfApi()->read<Game::PhysPreset>(Game::XAssetType::ASSET_TYPE_PHYSPRESET, name);
	}
}
