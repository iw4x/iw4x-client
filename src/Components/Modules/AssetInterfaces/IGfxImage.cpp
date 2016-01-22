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

		image->name = builder->GetAllocator()->DuplicateString(name);
		image->semantic = 2;
		image->category = 0;
		image->cardMemory = 0;

		image->texture = builder->GetAllocator()->AllocateArray<Game::GfxImageLoadDef>();
		if (!image->texture)
		{
			Components::Logger::Error("Failed to allocate GfxImageLoadDef structure!");
			return;
		}

		Components::FileSystem::File iwi(Utils::VA("images/%s.iwi", name.data()));

		if (!iwi.Exists())
		{
			Components::Logger::Error("Loading image '%s' failed!", iwi.GetName());
			return;
		}

		const Game::GfxImageFileHeader* iwiHeader = reinterpret_cast<const Game::GfxImageFileHeader*>(iwi.GetBuffer().data());

		image->mapType = 3;
		image->dataLen1 = iwiHeader->fileSizeForPicmip[0] - 32;
		image->dataLen2 = iwiHeader->fileSizeForPicmip[0] - 32;

		// TODO: Check tag
		image->texture->dimensions[0] = iwiHeader->dimensions[0]; // Width
		image->texture->dimensions[1] = iwiHeader->dimensions[1]; // Height
		image->texture->dimensions[2] = iwiHeader->dimensions[2]; // Depth
		image->texture->flags = 0;//iwiHeader->flags;
		image->texture->mipLevels = 0;

		switch (iwiHeader->format)
		{
		case Game::IWI_COMPRESSION::IWI_ARGB:
			image->texture->format = 21;
			break;
		case Game::IWI_COMPRESSION::IWI_RGB8:
			image->texture->format = 20;
			break;
		case Game::IWI_COMPRESSION::IWI_DXT1:
			image->texture->format = 0x31545844;
			break;
		case Game::IWI_COMPRESSION::IWI_DXT3:
			image->texture->format = 0x33545844;
			break;
		case Game::IWI_COMPRESSION::IWI_DXT5:
			image->texture->format = 0x35545844;
			break;
		}

		Components::AssetHandler::StoreTemporaryAsset(Game::XAssetType::ASSET_TYPE_IMAGE, { image });
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
