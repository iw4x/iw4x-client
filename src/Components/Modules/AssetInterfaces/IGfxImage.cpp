#include <STDInclude.hpp>

namespace Assets
{
	void IGfxImage::Load(Game::XAssetHeader* header, std::string name, Components::ZoneBuilder::Zone* builder)
	{
		Game::GfxImage* image = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_IMAGE, name.data()).image;
		if (image) return; // TODO: Check for default?

		image = builder->GetAllocator()->AllocateArray<Game::GfxImage>();
		if (!image)
		{
			Components::Logger::Error("Failed to allocate GfxImage structure!");
			return;
		}

		image->semantic = 0;
		image->category = 3;
		image->cardMemory = 0;

		Game::Image_LoadFromFileWithReader(image, (Game::Reader_t)0x46CBF0);

		// Free our image when done building zone
		builder->GetAllocator()->Reference(image, [] (void*data)
		{
			Game::Image_Release((Game::GfxImage*)data);
		});

		header->image = image;
	}

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
