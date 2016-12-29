#include <STDInclude.hpp>

#define IW4X_IMG_VERSION "0"

namespace Assets
{
	void IGfxImage::load(Game::XAssetHeader* header, std::string name, Components::ZoneBuilder::Zone* builder)
	{
		Game::GfxImage* image = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_IMAGE, name.data()).image;
		if (image && name[0] != '*') return;

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

		const char* tempName = image->name;
		if (tempName[0] == '*') tempName++;

		Components::FileSystem::File imageFile(fmt::sprintf("images/%s.iw4xImage", tempName));
		if (imageFile.exists())
		{
			Utils::Stream::Reader reader(builder->getAllocator(), imageFile.getBuffer());

			if (reader.read<__int64>() != *reinterpret_cast<__int64*>("IW4xImg" IW4X_IMG_VERSION))
			{
				Components::Logger::Error(0, "Reading image '%s' failed, header is invalid!", name.data());
			}

			AssertSize(Game::MapType, 1);
			image->mapType = reader.read<Game::MapType>();
			image->semantic = reader.read<char>();
			image->category = reader.read<char>();

			image->dataLen1 = reader.read<int>();
			image->dataLen2 = image->dataLen1;

			image->loadDef = reinterpret_cast<Game::GfxImageLoadDef*>(reader.readArray<char>(image->dataLen1 + 16));

			image->height = image->loadDef->dimensions[0];
			image->width = image->loadDef->dimensions[1];
			image->depth = image->loadDef->dimensions[2];

			image->loaded = true;
		}
		else
		{
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

			if (std::memcmp(iwiHeader->tag, "IWi", 3) && iwiHeader->version == 8)
			{
				Components::Logger::Error("Image is not a valid IWi!");
				return;
			}

			image->mapType = Game::MAPTYPE_2D;
			image->dataLen1 = iwiHeader->fileSizeForPicmip[0] - 32;
			image->dataLen2 = iwiHeader->fileSizeForPicmip[0] - 32;

			image->loadDef = builder->getAllocator()->allocate<Game::GfxImageLoadDef>();
			if (!image->loadDef)
			{
				Components::Logger::Error("Failed to allocate GfxImageLoadDef structure!");
				return;
			}

			std::memcpy(image->loadDef->dimensions, iwiHeader->dimensions, 6);
			image->loadDef->flags = 0;
			image->loadDef->levelCount = 0;

			image->height = image->loadDef->dimensions[0];
			image->width = image->loadDef->dimensions[1];
			image->depth = image->loadDef->dimensions[2];

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
		}

		header->image = image;
	}

	void IGfxImage::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::GfxImage, 32);
		AssertSize(Game::MapType, 1);

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

		if (asset->loadDef)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			
			Game::GfxImageLoadDef* destTexture = buffer->dest<Game::GfxImageLoadDef>();
			buffer->save(asset->loadDef, 16, 1);

			builder->incrementExternalSize(asset->loadDef->resourceSize);

			if (destTexture->resourceSize > 0)
			{
				buffer->save(asset->loadDef->data, asset->loadDef->resourceSize);
			}

			Utils::Stream::ClearPointer(&dest->loadDef);
		}

		buffer->popBlock();
		buffer->popBlock();
	}
}
