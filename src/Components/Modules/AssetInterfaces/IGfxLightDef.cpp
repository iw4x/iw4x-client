#include <STDInclude.hpp>
#include "IGfxLightDef.hpp"

namespace Assets
{
	void IGfxLightDef::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		header->lightDef = builder->getIW4OfApi()->read<Game::GfxLightDef>(Game::XAssetType::ASSET_TYPE_LIGHT_DEF, name);
	}

	void IGfxLightDef::mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::GfxLightDef* asset = header.lightDef;

		if (asset->attenuation.image)
		{
			builder->loadAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->attenuation.image);
		}
	}

	void IGfxLightDef::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::GfxLightDef, 16);
		AssertSize(Game::GfxLightImage, 8);

		Utils::Stream* buffer = builder->getBuffer();
		Game::GfxLightDef* asset = header.lightDef;
		Game::GfxLightDef* dest = buffer->dest<Game::GfxLightDef>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		if (asset->attenuation.image)
		{
			dest->attenuation.image = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->attenuation.image).image;
		}

		buffer->popBlock();
	}
}
