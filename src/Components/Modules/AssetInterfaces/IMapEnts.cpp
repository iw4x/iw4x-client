#include <STDInclude.hpp>

namespace Assets
{
	void IMapEnts::Load(Game::XAssetHeader* header, std::string name, Components::ZoneBuilder::Zone* builder)
	{
		Components::FileSystem::File ents(name + ".ents");
		if (ents.Exists())
		{
			Game::MapEnts* entites = builder->GetAllocator()->AllocateArray<Game::MapEnts>();
			Game::MapEnts* orgEnts = Components::AssetHandler::FindOriginalAsset(this->GetType(), name.data()).mapEnts;

			if (orgEnts)
			{
				memcpy(entites, orgEnts, sizeof Game::MapEnts);
			}
			else
			{
				entites->name = builder->GetAllocator()->DuplicateString(name);
			}

			entites->entityString = builder->GetAllocator()->DuplicateString(ents.GetBuffer());
			entites->numEntityChars = ents.GetBuffer().size() + 1;

			header->mapEnts = entites;
		}
	}

	void IMapEnts::Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Assert_Size(Game::MapEnts, 44);

		Utils::Stream* buffer = builder->GetBuffer();
		Game::MapEnts* asset = header.mapEnts;
		Game::MapEnts* dest = buffer->Dest<Game::MapEnts>();
		buffer->Save(asset, sizeof(Game::MapEnts));

		buffer->PushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->SaveString(builder->GetAssetName(this->GetType(), asset->name));
			dest->name = reinterpret_cast<char*>(-1);
		}

		if (asset->entityString)
		{
			buffer->Save(asset->entityString, asset->numEntityChars);
			dest->entityString = reinterpret_cast<char*>(-1);
		}

		if (asset->trigger.models)
		{
			Assert_Size(Game::TriggerModel, 8);
			buffer->Align(Utils::Stream::ALIGN_4);
			buffer->SaveArray(asset->trigger.models, asset->trigger.modelCount);
			dest->trigger.models = reinterpret_cast<Game::TriggerModel*>(-1);
		}

		if (asset->trigger.hulls)
		{
			Assert_Size(Game::TriggerHull, 32);
			buffer->Align(Utils::Stream::ALIGN_4);
			buffer->SaveArray(asset->trigger.hulls, asset->trigger.hullCount);
			dest->trigger.hulls = reinterpret_cast<Game::TriggerHull*>(-1);
		}

		if (asset->trigger.slabs)
		{
			Assert_Size(Game::TriggerSlab, 20);
			buffer->Align(Utils::Stream::ALIGN_4);
			buffer->SaveArray(asset->trigger.slabs, asset->trigger.slabCount);
			dest->trigger.slabs = reinterpret_cast<Game::TriggerSlab*>(-1);
		}

		if (asset->stages)
		{
			Assert_Size(Game::Stage, 20);

			buffer->Align(Utils::Stream::ALIGN_4);

			Game::Stage* destStages = buffer->Dest<Game::Stage>();
			buffer->SaveArray(asset->stages, asset->stageCount);

			for (int i = 0; i < asset->stageCount; ++i)
			{
				Game::Stage* destStage = &destStages[i];
				Game::Stage* stage = &asset->stages[i];

				buffer->SaveString(stage->stageName);
				destStage->stageName = reinterpret_cast<char*>(-1);
			}

			dest->stages = reinterpret_cast<Game::Stage*>(-1);
		}

		buffer->PopBlock();
	}
}
