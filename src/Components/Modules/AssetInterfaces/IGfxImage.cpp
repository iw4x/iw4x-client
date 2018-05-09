#include "STDInclude.hpp"

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
		image->picmip.platform[0] = 0;
		image->picmip.platform[1] = 0;
		image->noPicmip = 0;
		image->track = 0;

		const char* tempName = image->name;
		if (tempName[0] == '*') tempName++;

		Components::FileSystem::File imageFile(Utils::String::VA("images/%s.iw4xImage", tempName));
		if (imageFile.exists())
		{
			Utils::Stream::Reader reader(builder->getAllocator(), imageFile.getBuffer());

			__int64 magic = reader.read<__int64>();
			if (std::memcmp(&magic, "IW4xImg" IW4X_IMG_VERSION, 8))
			{
				Components::Logger::Error(0, "Reading image '%s' failed, header is invalid!", name.data());
			}

			image->mapType = reader.read<char>();
			image->semantic = reader.read<char>();
			image->category = reader.read<char>();

			image->cardMemory.platform[0] = reader.read<int>();
			image->cardMemory.platform[1] = image->cardMemory.platform[0];

			image->texture.loadDef = reinterpret_cast<Game::GfxImageLoadDef*>(reader.readArray<char>(image->cardMemory.platform[0] + 16));

			// TODO: Fix that, this is clearly wrong
			auto dimensions = reinterpret_cast<unsigned short*>(&image->texture.loadDef->pad[2]);
			image->height = dimensions[0];
			image->width = dimensions[1];
			image->depth = dimensions[2];

			image->delayLoadPixels = true;
			image->texture.loadDef->flags = 0;

			if (image->texture.loadDef->resourceSize != image->cardMemory.platform[0])
			{
				Components::Logger::Error("Resource size doesn't match the data length (%s)!\n", name.data());
			}

			if (Utils::String::StartsWith(name, "*lightmap"))
			{
				dimensions[0] = 0;
				dimensions[1] = 2;
				dimensions[2] = 0;
			}

			header->image = image;
		}
		else if (name[0] != '*')
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
			//image->dataLen1 = iwiHeader->fileSizeForPicmip[0] - 32;
			//image->dataLen2 = iwiHeader->fileSizeForPicmip[0] - 32;

			image->texture.loadDef = builder->getAllocator()->allocate<Game::GfxImageLoadDef>();
			if (!image->texture.loadDef)
			{
				Components::Logger::Error("Failed to allocate GfxImageLoadDef structure!");
				return;
			}

			image->texture.loadDef->flags = 0;
			image->texture.loadDef->levelCount = 0;

			image->height = iwiHeader->dimensions[0];
			image->width = iwiHeader->dimensions[1];
			image->depth = iwiHeader->dimensions[2];

			switch (iwiHeader->format)
			{
			case Game::IMG_FORMAT_BITMAP_RGBA:
			{
				image->texture.loadDef->format = 21;
				break;
			}

			case Game::IMG_FORMAT_BITMAP_RGB:
			{
				image->texture.loadDef->format = 20;
				break;
			}

			case Game::IMG_FORMAT_DXT1:
			{
				image->texture.loadDef->format = 0x31545844;
				break;
			}

			case Game::IMG_FORMAT_DXT3:
			{
				image->texture.loadDef->format = 0x33545844;
				break;
			}

			case Game::IMG_FORMAT_DXT5:
			{
				image->texture.loadDef->format = 0x35545844;
				break;
			}

			default:
			{
				break;
			}
			}

			header->image = image;
		}
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
