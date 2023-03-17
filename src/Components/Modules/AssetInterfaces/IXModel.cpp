#include <STDInclude.hpp>
#include "IXModel.hpp"

namespace Assets
{
	void IXModel::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		header->model = builder->getIW4OfApi()->read<Game::XModel>(Game::XAssetType::ASSET_TYPE_XMODEL, name);

		if (header->model) 
		{
			// ???
			if (header->model->physCollmap)
			{
				Components::AssetHandler::StoreTemporaryAsset(Game::XAssetType::ASSET_TYPE_PHYSCOLLMAP, { header->model->physCollmap });
			}

			if (header->model->physPreset)
			{
				Components::AssetHandler::StoreTemporaryAsset(Game::XAssetType::ASSET_TYPE_PHYSPRESET, { header->model->physPreset });
			}

			for (size_t i = 0; i < header->model->numLods; i++)
			{
				const auto& info = header->model->lodInfo[i];
				Components::AssetHandler::StoreTemporaryAsset(Game::XAssetType::ASSET_TYPE_XMODEL_SURFS, { info.modelSurfs });
			}
		}
	}

	void IXModel::mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::XModel* asset = header.model;

		if (asset->boneNames)
		{
			for (char i = 0; i < asset->numBones; ++i)
			{
				builder->addScriptString(asset->boneNames[i]);
			}
		}

		if (asset->materialHandles)
		{
			for (unsigned char i = 0; i < asset->numsurfs; ++i)
			{
				if (asset->materialHandles[i])
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->materialHandles[i]);
				}
			}
		}

		for (int i = 0; i < 4; ++i)
		{
			if (asset->lodInfo[i].modelSurfs)
			{
				builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL_SURFS, asset->lodInfo[i].modelSurfs);
			}
		}

		if (asset->physPreset)
		{
			builder->loadAsset(Game::XAssetType::ASSET_TYPE_PHYSPRESET, asset->physPreset);
		}

		if (asset->physCollmap)
		{
			builder->loadAsset(Game::XAssetType::ASSET_TYPE_PHYSCOLLMAP, asset->physCollmap);
		}
	}

	void IXModel::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::XModel, 304);

		Utils::Stream* buffer = builder->getBuffer();
		Game::XModel* asset = header.model;
		Game::XModel* dest = buffer->dest<Game::XModel>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		if (asset->boneNames)
		{
			buffer->align(Utils::Stream::ALIGN_2);

			unsigned short* destBoneNames = buffer->dest<unsigned short>();
			buffer->saveArray(asset->boneNames, asset->numBones);

			for (char i = 0; i < asset->numBones; ++i)
			{
				builder->mapScriptString(destBoneNames[i]);
			}

			Utils::Stream::ClearPointer(&dest->boneNames);
		}

		if (asset->parentList)
		{
			if (builder->hasPointer(asset->parentList))
			{
				dest->parentList = builder->getPointer(asset->parentList);
			}
			else
			{
				builder->storePointer(asset->parentList);
				buffer->save(asset->parentList, asset->numBones - asset->numRootBones);
				Utils::Stream::ClearPointer(&dest->parentList);
			}
		}

		if (asset->quats)
		{
			if (builder->hasPointer(asset->quats))
			{
				dest->quats = builder->getPointer(asset->quats);
			}
			else
			{
				buffer->align(Utils::Stream::ALIGN_2);
				builder->storePointer(asset->quats);
				buffer->saveArray(asset->quats, (asset->numBones - asset->numRootBones) * 4);
				Utils::Stream::ClearPointer(&dest->quats);
			}
		}

		if (asset->trans)
		{
			if (builder->hasPointer(asset->trans))
			{
				dest->trans = builder->getPointer(asset->trans);
			}
			else
			{
				buffer->align(Utils::Stream::ALIGN_4);
				builder->storePointer(asset->trans);
				buffer->saveArray(asset->trans, (asset->numBones - asset->numRootBones) * 3);
				Utils::Stream::ClearPointer(&dest->trans);
			}
		}

		if (asset->partClassification)
		{
			if (builder->hasPointer(asset->partClassification))
			{
				dest->partClassification = builder->getPointer(asset->partClassification);
			}
			else
			{
				builder->storePointer(asset->partClassification);
				buffer->save(asset->partClassification, asset->numBones);
				Utils::Stream::ClearPointer(&dest->partClassification);
			}
		}

		if (asset->baseMat)
		{
			AssertSize(Game::DObjAnimMat, 32);
			if (builder->hasPointer(asset->baseMat))
			{
				dest->baseMat = builder->getPointer(asset->baseMat);
			}
			else
			{
				buffer->align(Utils::Stream::ALIGN_4);
				builder->storePointer(asset->baseMat);
				buffer->saveArray(asset->baseMat, asset->numBones);
				Utils::Stream::ClearPointer(&dest->baseMat);
			}

		}

		if (asset->materialHandles)
		{
			buffer->align(Utils::Stream::ALIGN_4);

			Game::Material** destMaterials = buffer->dest<Game::Material*>();
			buffer->saveArray(asset->materialHandles, asset->numsurfs);

			for (unsigned char i = 0; i < asset->numsurfs; ++i)
			{
				if (asset->materialHandles[i])
				{
					destMaterials[i] = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->materialHandles[i]).material;
				}
			}

			Utils::Stream::ClearPointer(&dest->materialHandles);
		}

		// Save_XModelLodInfoArray
		{
			AssertSize(Game::XModelLodInfo, 44);

			for (int i = 0; i < 4; ++i)
			{
				if (asset->lodInfo[i].modelSurfs)
				{
					dest->lodInfo[i].modelSurfs = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_XMODEL_SURFS, asset->lodInfo[i].modelSurfs).modelSurfs;
				}
			}
		}

		// Save_XModelCollSurfArray
		if (asset->collSurfs)
		{
			AssertSize(Game::XModelCollSurf_s, 44);

			buffer->align(Utils::Stream::ALIGN_4);

			Game::XModelCollSurf_s* destColSurfs = buffer->dest<Game::XModelCollSurf_s>();
			buffer->saveArray(asset->collSurfs, asset->numCollSurfs);

			for (int i = 0; i < asset->numCollSurfs; ++i)
			{
				Game::XModelCollSurf_s* destCollSurf = &destColSurfs[i];
				Game::XModelCollSurf_s* collSurf = &asset->collSurfs[i];

				if (collSurf->collTris)
				{
					buffer->align(Utils::Stream::ALIGN_4);

					buffer->save(collSurf->collTris, 48, collSurf->numCollTris);
					Utils::Stream::ClearPointer(&destCollSurf->collTris);
				}
			}

			Utils::Stream::ClearPointer(&dest->collSurfs);
		}

		if (asset->boneInfo)
		{
			AssertSize(Game::XBoneInfo, 28);

			buffer->align(Utils::Stream::ALIGN_4);

			buffer->saveArray(asset->boneInfo, asset->numBones);
			Utils::Stream::ClearPointer(&dest->boneInfo);
		}

		if (asset->physPreset)
		{
			dest->physPreset = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_PHYSPRESET, asset->physPreset).physPreset;
		}

		if (asset->physCollmap)
		{
			dest->physCollmap = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_PHYSCOLLMAP, asset->physCollmap).physCollmap;
		}

		buffer->popBlock();
	}
}
