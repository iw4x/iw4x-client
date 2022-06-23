#include <STDInclude.hpp>
#include "IGfxLightDef.hpp"

#define IW4X_LIGHT_VERSION "0"

namespace Assets
{
	void IGfxLightDef::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		Components::FileSystem::File mapFile(Utils::String::VA("lights/%s.iw4xLight", name.data()));

		if (mapFile.exists())
		{
			Utils::Stream::Reader reader(builder->getAllocator(), mapFile.getBuffer());

			char* magic = reader.readArray<char>(7);
			if (std::memcmp(magic, "IW4xLit", 7))
			{
				Components::Logger::Error(Game::ERR_FATAL, "Reading light '{}' failed, header is invalid!", name);
			}

			std::string version;
			version.push_back(reader.read<char>());
			if (version != IW4X_LIGHT_VERSION)
			{
				Components::Logger::Error(Game::ERR_FATAL, "Reading light '{}' failed, expected version is {}, but it was {}!", name, IW4X_LIGHT_VERSION, version);
			}

			Game::GfxLightDef* asset = reader.readObject<Game::GfxLightDef>();
			header->lightDef = asset;

			if (asset->name)
			{
				asset->name = reader.readCString();
			}

			if (asset->attenuation.image)
			{
				asset->attenuation.image = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_IMAGE, reader.readString().data(), builder).image;
			}
		}
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
