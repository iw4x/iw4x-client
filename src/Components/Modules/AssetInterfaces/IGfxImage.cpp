#include <STDInclude.hpp>
#include "IGfxImage.hpp"

namespace Assets
{
	void IGfxImage::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		header->image = builder->getIW4OfApi()->read<Game::GfxImage>(Game::XAssetType::ASSET_TYPE_IMAGE, name);
	}

	void IGfxImage::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::GfxImage, 32);

		Utils::Stream* buffer = builder->getBuffer();
		Game::GfxImage* asset = header.image;
		Game::GfxImage* dest = buffer->dest<Game::GfxImage>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		buffer->pushBlock(Game::XFILE_BLOCK_TEMP);

		if (asset->texture.loadDef)
		{
			buffer->align(Utils::Stream::ALIGN_4);

			Game::GfxImageLoadDef* destTexture = buffer->dest<Game::GfxImageLoadDef>();
			buffer->save(asset->texture.loadDef, 16, 1);

			builder->incrementExternalSize(asset->texture.loadDef->resourceSize);

			if (destTexture->resourceSize > 0)
			{
				buffer->save(asset->texture.loadDef->data, asset->texture.loadDef->resourceSize);
			}

			Utils::Stream::ClearPointer(&dest->texture.loadDef);
		}

		buffer->popBlock();
		buffer->popBlock();
	}
}
