#include <STDInclude.hpp>

namespace Assets
{
	void IMapEnts::load(Game::XAssetHeader* header, std::string name, Components::ZoneBuilder::Zone* builder)
	{
		Components::FileSystem::File ents(name + ".ents");
		if (ents.exists())
		{
			Game::MapEnts* entites = builder->getAllocator()->allocate<Game::MapEnts>();
			Game::MapEnts* orgEnts = Components::AssetHandler::FindOriginalAsset(this->getType(), name.data()).mapEnts;

			if (orgEnts)
			{
				std::memcpy(entites, orgEnts, sizeof Game::MapEnts);
			}
			else
			{
				entites->name = builder->getAllocator()->duplicateString(name);
			}

			entites->entityString = builder->getAllocator()->duplicateString(ents.getBuffer());
			entites->numEntityChars = ents.getBuffer().size() + 1;

			header->mapEnts = entites;
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

		if (asset->trigger.models)
		{
			AssertSize(Game::TriggerModel, 8);
			buffer->align(Utils::Stream::ALIGN_4);

			buffer->saveArray(asset->trigger.models, asset->trigger.modelCount);

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

			for (int i = 0; i < asset->stageCount; ++i)
			{
				Game::Stage* destStage = &destStages[i];
				Game::Stage* stage = &asset->stages[i];

				buffer->saveString(stage->stageName);
				Utils::Stream::ClearPointer(&destStage->stageName);
			}

			Utils::Stream::ClearPointer(&dest->stages);
		}

		buffer->popBlock();
	}
}
