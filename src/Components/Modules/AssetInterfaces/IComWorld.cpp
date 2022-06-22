#include <STDInclude.hpp>
#include "IComWorld.hpp"

#define IW4X_COMMAP_VERSION 0

namespace Assets
{
	void IComWorld::load(Game::XAssetHeader* header, const std::string& _name, Components::ZoneBuilder::Zone* builder)
	{
		std::string name = _name;
		Utils::String::Replace(name, "maps/mp/", "");
		Utils::String::Replace(name, ".d3dbsp", "");

		Components::FileSystem::File mapFile(Utils::String::VA("comworld/%s.iw4xComWorld", name.data()));

		if (mapFile.exists())
		{
			Utils::Stream::Reader reader(builder->getAllocator(), mapFile.getBuffer());

			__int64 magic = reader.read<__int64>();
			if (std::memcmp(&magic, "IW4xComW", 8))
			{
				Components::Logger::Error(Game::ERR_FATAL, "Reading comworld '{}' failed, header is invalid!", name);
			}

			int version = reader.read<int>();
			if (version != IW4X_COMMAP_VERSION)
			{
				Components::Logger::Error(Game::ERR_FATAL, "Reading comworld '{}' failed, expected version is {}, but it was {}!", name, IW4X_COMMAP_VERSION, version);
			}

			Game::ComWorld* asset = reader.readObject<Game::ComWorld>();
			header->comWorld = asset;

			if (asset->name)
			{
				asset->name = reader.readCString();
			}

			if (asset->primaryLights)
			{
				asset->primaryLights = reader.readArray<Game::ComPrimaryLight>(asset->primaryLightCount);

				for (unsigned int i = 0; i < asset->primaryLightCount; ++i)
				{
					Game::ComPrimaryLight* light = &asset->primaryLights[i];

					if (light->defName)
					{
						light->defName = reader.readCString();
						Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_LIGHT_DEF, light->defName, builder);
					}
				}
			}
		}
	}

	void IComWorld::mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::ComWorld* asset = header.comWorld;

		if (asset->primaryLights)
		{
			for (unsigned int i = 0; i < asset->primaryLightCount; ++i)
			{
				if (asset->primaryLights[i].defName)
				{
					builder->loadAssetByName(Game::XAssetType::ASSET_TYPE_LIGHT_DEF, asset->primaryLights[i].defName, false);
				}
			}
		}
	}

	void IComWorld::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::ComWorld, 16);

		Utils::Stream* buffer = builder->getBuffer();
		Game::ComWorld* asset = header.comWorld;
		Game::ComWorld* dest = buffer->dest<Game::ComWorld>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		if (asset->primaryLights)
		{
			AssertSize(Game::ComPrimaryLight, 68);
			buffer->align(Utils::Stream::ALIGN_4);

			Game::ComPrimaryLight* destLights = buffer->dest<Game::ComPrimaryLight>();
			buffer->saveArray(asset->primaryLights, asset->primaryLightCount);

			for (unsigned int i = 0; i < asset->primaryLightCount; ++i)
			{
				Game::ComPrimaryLight* destLight = &destLights[i];
				Game::ComPrimaryLight* light = &asset->primaryLights[i];

				if (light->defName)
				{
					buffer->saveString(light->defName);
					Utils::Stream::ClearPointer(&destLight->defName);
				}
			}

			Utils::Stream::ClearPointer(&dest->primaryLights);
		}

		buffer->popBlock();
	}
}
