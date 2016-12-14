#include <STDInclude.hpp>

namespace Assets
{
	void IGfxImage::load(Game::XAssetHeader* header, std::string name, Components::ZoneBuilder::Zone* builder)
	{
		Game::GfxImage* image = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_IMAGE, name.data()).image;
		if (image) return;

		image = builder->getAllocator()->allocate<Game::GfxImage>();
		if (!image)
		{
			Components::Logger::Error("Failed to allocate GfxImage structure!");
			return;
		}

		image->name = builder->getAllocator()->duplicateString(name);
		image->semantic = 2;
		image->category = 0;
		image->cardMemory = 0;

		image->loadDef = builder->getAllocator()->allocate<Game::GfxImageLoadDef>();
		if (!image->loadDef)
		{
			Components::Logger::Error("Failed to allocate GfxImageLoadDef structure!");
			return;
		}

		char nameBuffer[MAX_PATH] = { 0 };
		Components::Materials::FormatImagePath(nameBuffer, sizeof(nameBuffer), 0, 0, name.data());
		Components::FileSystem::File iwi(nameBuffer);

		if (!iwi.exists())
		{
			Components::Logger::Error("Loading image '%s' failed!", iwi.getName().data());
			return;
		}

		auto iwiBuffer = iwi.getBuffer();

		const Game::GfxImageFileHeader* iwiHeader = reinterpret_cast<const Game::GfxImageFileHeader*>(iwiBuffer.data());
		
		image->mapType = 3;
		image->dataLen1 = iwiHeader->fileSizeForPicmip[0] - 32;
		image->dataLen2 = iwiHeader->fileSizeForPicmip[0] - 32;

		if (std::memcmp(iwiHeader->tag, "IWi", 3))
		{
			Components::Logger::Error("Image is not a valid IWi!");
			return;
		}

		std::memcpy(image->loadDef->dimensions, iwiHeader->dimensions, 6);
		image->loadDef->flags = 0;
		image->loadDef->mipLevels = 0;

		switch (iwiHeader->format)
		{
			case Game::IWI_COMPRESSION::IWI_ARGB:
			{
				image->loadDef->format = 21;
				break;
			}

			case Game::IWI_COMPRESSION::IWI_RGB8:
			{
				image->loadDef->format = 20;
				break;
			}

			case Game::IWI_COMPRESSION::IWI_DXT1:
			{
				image->loadDef->format = 0x31545844;
				break;
			}

			case Game::IWI_COMPRESSION::IWI_DXT3:
			{
				image->loadDef->format = 0x33545844;
				break;
			}

			case Game::IWI_COMPRESSION::IWI_DXT5:
			{
				image->loadDef->format = 0x35545844;
				break;
			}
		}

		header->image = image;
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

		if (asset->texture)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			
			Game::GfxImageLoadDef* destTexture = buffer->dest<Game::GfxImageLoadDef>();
			buffer->save(asset->loadDef, 16);

			builder->incrementExternalSize(asset->loadDef->dataSize);

			// Zero the size!
			destTexture->dataSize = 0;

			if (destTexture->dataSize > 0)
			{
				buffer->save(asset->loadDef->data, asset->loadDef->dataSize);
			}

			Utils::Stream::ClearPointer(&dest->loadDef);
		}

		buffer->popBlock();
		buffer->popBlock();
	}
}
