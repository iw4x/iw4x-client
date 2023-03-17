#include <STDInclude.hpp>
#include "IMapEnts.hpp"

namespace Assets
{
	void IMapEnts::load(Game::XAssetHeader* header, const std::string& _name, Components::ZoneBuilder::Zone* builder)
	{
		header->mapEnts = builder->getIW4OfApi()->read<Game::MapEnts>(Game::XAssetType::ASSET_TYPE_MAP_ENTS, _name);
	}

	void IMapEnts::mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Utils::Entities memEnts(header.mapEnts->entityString, header.mapEnts->numEntityChars);
		for (auto& model : memEnts.getModels())
		{
			builder->loadAssetByName(Game::XAssetType::ASSET_TYPE_XMODEL, model, false);
		}
	}

	void IMapEnts::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::MapEnts, 44);

		Utils::Stream* buffer = builder->getBuffer();
		Game::MapEnts* asset = header.mapEnts;
		Game::MapEnts* dest = buffer->dest<Game::MapEnts>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		if (asset->entityString)
		{
			buffer->save(asset->entityString, asset->numEntityChars);
			Utils::Stream::ClearPointer(&dest->entityString);
		}

		AssertSize(Game::MapTriggers, 24);

		if (asset->trigger.models)
		{
			AssertSize(Game::TriggerModel, 8);

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->trigger.models, asset->trigger.count);
			Utils::Stream::ClearPointer(&dest->trigger.models);
		}

		if (asset->trigger.hulls)
		{
			AssertSize(Game::TriggerHull, 32);

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->trigger.hulls, asset->trigger.hullCount);
			Utils::Stream::ClearPointer(&dest->trigger.hulls);
		}

		if (asset->trigger.slabs)
		{
			AssertSize(Game::TriggerSlab, 20);

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->trigger.slabs, asset->trigger.slabCount);
			Utils::Stream::ClearPointer(&dest->trigger.slabs);
		}

		if (asset->stages)
		{
			AssertSize(Game::Stage, 20);

			buffer->align(Utils::Stream::ALIGN_4);

			Game::Stage* destStages = buffer->dest<Game::Stage>();
			buffer->saveArray(asset->stages, asset->stageCount);

			for (char i = 0; i < asset->stageCount; ++i)
			{
				Game::Stage* destStage = &destStages[i];
				Game::Stage* stage = &asset->stages[i];

				if (stage->name)
				{
					buffer->saveString(stage->name);
					Utils::Stream::ClearPointer(&destStage->name);
				}
			}

			Utils::Stream::ClearPointer(&dest->stages);
		}

		buffer->popBlock();
	}
}
