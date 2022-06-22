#include <STDInclude.hpp>
#include "ITracerDef.hpp"

namespace Assets
{
	void ITracerDef::load(Game::XAssetHeader* /*header*/, const std::string& /*name*/, Components::ZoneBuilder::Zone* /*builder*/)
	{
		// don't load from filesystem right now
	}

	void ITracerDef::mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::TracerDef* asset = header.tracerDef;

		if (asset->material)
		{
			builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->material);
		}
	}

	void ITracerDef::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::TracerDef, 0x70);

		Utils::Stream* buffer = builder->getBuffer();
		Game::TracerDef* asset = header.tracerDef;
		Game::TracerDef* dest = buffer->dest<Game::TracerDef>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		if (asset->material)
		{
			dest->material = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->material).material;
		}

		buffer->popBlock();
	}
}
