#include <STDInclude.hpp>

namespace Assets
{
	void IGfxImage::Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Assert_AssetStruct(Game::GfxImage, 32);

		Utils::Stream* buffer = builder->GetBuffer();
		Game::GfxImage* asset = header.image;
		Game::GfxImage* dest = (Game::GfxImage*)buffer->At();
		buffer->Save(asset, sizeof(Game::GfxImage));

		buffer->PushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->SaveString(builder->GetAssetName(this->GetType(), asset->name));
			dest->name = (char *)-1;
		}

		if (asset->texture)
		{
			buffer->Align(Utils::Stream::ALIGN_4);
			
			Game::GfxImageLoadDef* destTexture = (Game::GfxImageLoadDef*)buffer->At();
			buffer->Save(asset->texture, 16);

			// Zero the size!
			destTexture->dataSize = 0;

			dest->texture = (Game::GfxImageLoadDef*)-1;
		}

		buffer->PopBlock();
	}
}
