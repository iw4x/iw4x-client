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
		buffer->Save(asset);

		buffer->PushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->SaveString(builder->GetAssetName(this->GetType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		if (asset->entityString)
		{
			buffer->Save(asset->entityString, asset->numEntityChars);
			Utils::Stream::ClearPointer(&dest->entityString);
		}

		if (asset->trigger.models)
		{
			Assert_Size(Game::TriggerModel, 8);
			buffer->Align(Utils::Stream::ALIGN_4);

			buffer->SaveArray(asset->trigger.models, asset->trigger.modelCount);

			Utils::Stream::ClearPointer(&dest->trigger.models);
		}

		if (asset->trigger.hulls)
		{
			Assert_Size(Game::TriggerHull, 32);
			buffer->Align(Utils::Stream::ALIGN_4);

			buffer->SaveArray(asset->trigger.hulls, asset->trigger.hullCount);

			Utils::Stream::ClearPointer(&dest->trigger.hulls);
		}

		if (asset->trigger.slabs)
		{
			Assert_Size(Game::TriggerSlab, 20);
			buffer->Align(Utils::Stream::ALIGN_4);

			buffer->SaveArray(asset->trigger.slabs, asset->trigger.slabCount);

			Utils::Stream::ClearPointer(&dest->trigger.slabs);
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

				Utils::Stream::ClearPointer(&destStage->stageName);
			}

			Utils::Stream::ClearPointer(&dest->stages);
		}

		buffer->PopBlock();
	}
}
