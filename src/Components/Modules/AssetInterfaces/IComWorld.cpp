#include <STDInclude.hpp>
#include "IComWorld.hpp"

#define IW4X_COMMAP_VERSION 0

namespace Assets
{
	void IComWorld::load(Game::XAssetHeader* header, const std::string& _name, Components::ZoneBuilder::Zone* builder)
	{
		header->comWorld = builder->getIW4OfApi()->read<Game::ComWorld>(Game::XAssetType::ASSET_TYPE_COMWORLD, _name);
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
