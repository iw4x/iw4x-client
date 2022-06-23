#include <STDInclude.hpp>
#include "IMapEnts.hpp"

namespace Assets
{
	void IMapEnts::load(Game::XAssetHeader* header, const std::string& _name, Components::ZoneBuilder::Zone* builder)
	{
		std::string name = _name;
		Utils::String::Replace(name, "maps/", "");
		Utils::String::Replace(name, "mp/", "");
		Utils::String::Replace(name, ".d3dbsp", "");

		Components::FileSystem::File ents(Utils::String::VA("mapents/%s.ents", name.data()));
		if (ents.exists())
		{
			Game::MapEnts* entites = builder->getAllocator()->allocate<Game::MapEnts>();
			Game::MapEnts* orgEnts = Components::AssetHandler::FindOriginalAsset(this->getType(), name.data()).mapEnts;

			// TODO: Get rid of that
			if (!orgEnts)
			{
				orgEnts = Components::AssetHandler::FindOriginalAsset(Game::XAssetType::ASSET_TYPE_MAP_ENTS, "maps/iw4_credits.d3dbsp").mapEnts;

				if (!orgEnts)
				{
					Game::DB_EnumXAssets(Game::XAssetType::ASSET_TYPE_MAP_ENTS, [](Game::XAssetHeader header, void* mapEnts)
					{
						if (!*reinterpret_cast<void**>(mapEnts))
						{
							*reinterpret_cast<Game::MapEnts**>(mapEnts) = header.mapEnts;
						}
					}, &orgEnts, false);
				}
			}

			if (orgEnts)
			{
				std::memcpy(entites, orgEnts, sizeof Game::MapEnts);
			}
			else
			{
				entites->stageCount = 1;
				entites->stages = builder->getAllocator()->allocate<Game::Stage>();
				entites->stages[0].name = "stage 0";
				entites->stages[0].triggerIndex = 0x400;
				entites->stages[0].sunPrimaryLightIndex = 0x1;
			}

			std::string entityString = ents.getBuffer();

			entites->name = builder->getAllocator()->duplicateString(Utils::String::VA("maps/mp/%s.d3dbsp", name.data()));
			entites->entityString = builder->getAllocator()->duplicateString(entityString);
			entites->numEntityChars = entityString.size() + 1;

			header->mapEnts = entites;
		}
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
