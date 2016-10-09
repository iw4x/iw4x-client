#include <STDInclude.hpp>

namespace Assets
{
	void IGfxImage::Load(Game::XAssetHeader* header, std::string name, Components::ZoneBuilder::Zone* builder)
	{
		Game::GfxImage* image = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_IMAGE, name.data()).image;
		if (image) return;

		image = builder->GetAllocator()->Allocate<Game::GfxImage>();
		if (!image)
		{
			Components::Logger::Error("Failed to allocate GfxImage structure!");
			return;
		}

		image->name = builder->GetAllocator()->DuplicateString(name);
		image->semantic = 2;
		image->category = 0;
		image->cardMemory = 0;

		image->loadDef = builder->GetAllocator()->Allocate<Game::GfxImageLoadDef>();
		if (!image->loadDef)
		{
			Components::Logger::Error("Failed to allocate GfxImageLoadDef structure!");
			return;
		}

		Components::FileSystem::File iwi(fmt::sprintf("images/%s.iwi", name.data()));

		if (!iwi.Exists())
		{
			Components::Logger::Error("Loading image '%s' failed!", iwi.GetName().data());
			return;
		}

		auto iwiBuffer = iwi.GetBuffer();

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

	void IGfxImage::Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Assert_Size(Game::GfxImage, 32);

		Utils::Stream* buffer = builder->GetBuffer();
		Game::GfxImage* asset = header.image;
		Game::GfxImage* dest = buffer->Dest<Game::GfxImage>();
		buffer->Save(asset);

		buffer->PushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->SaveString(builder->GetAssetName(this->GetType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		buffer->PushBlock(Game::XFILE_BLOCK_TEMP);

		if (asset->texture)
		{
			buffer->Align(Utils::Stream::ALIGN_4);
			
			Game::GfxImageLoadDef* destTexture = buffer->Dest<Game::GfxImageLoadDef>();
			buffer->Save(asset->texture, 16);

			// Zero the size!
			destTexture->dataSize = 0;

			Utils::Stream::ClearPointer(&dest->texture);
		}

		buffer->PopBlock();
		buffer->PopBlock();
	}
}
