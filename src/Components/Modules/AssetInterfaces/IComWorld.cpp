#include "STDInclude.hpp"

namespace Assets
{
	void IComWorld::mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::ComWorld* asset = header.comWorld;
		if (asset->lights)
		{
			for (int i = 0; i < asset->lightCount; ++i)
			{
				if (asset->lights[i].name)
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_LIGHT_DEF, std::string(asset->lights[i].name), false);
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

		if (asset->lights)
		{
			AssertSize(Game::ComPrimaryLight, 68);
			buffer->align(Utils::Stream::ALIGN_4);

			Game::ComPrimaryLight* destLights = buffer->dest<Game::ComPrimaryLight>();
			buffer->saveArray(asset->lights, asset->lightCount);

			for (int i = 0; i < asset->lightCount; ++i)
			{
				Game::ComPrimaryLight* destLight = &destLights[i];
				Game::ComPrimaryLight* light = &asset->lights[i];

				if (light->name)
				{
					buffer->saveString(light->name);
					Utils::Stream::ClearPointer(&destLight->name);
				}
			}

			Utils::Stream::ClearPointer(&dest->lights);
		}

		buffer->popBlock();
	}
}
