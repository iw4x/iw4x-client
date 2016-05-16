#include <STDInclude.hpp>

namespace Assets
{
	void IXModel::Mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::XModel* asset = header.model;

		if (asset->boneNames)
		{
			for (char i = 0; i < asset->numBones; ++i)
			{
				builder->AddScriptString(asset->boneNames[i]);
			}
		}

		if (asset->materials)
		{
			for (char i = 0; i < asset->numSurfaces; ++i)
			{
				if (asset->materials[i])
				{
					//Components::Logger::Print("%s\n", asset->materials[i]->name);
					builder->LoadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->materials[i]->name);
				}
			}
		}

		for (int i = 0; i < 4; ++i)
		{
			if (asset->lods[i].surfaces)
			{
				// We're not supposed to include xmodelsurfs as standalone asset
				//builder->LoadAsset(Game::XAssetType::ASSET_TYPE_XMODELSURFS, asset->lods[i].surfaces->name);

				IXModelSurfs().Mark({ asset->lods[i].surfaces }, builder);
			}
		}

		if (asset->physPreset)
		{
			builder->LoadAsset(Game::XAssetType::ASSET_TYPE_PHYSPRESET, asset->physPreset->name);
		}

		if (asset->physCollmap)
		{
			builder->LoadAsset(Game::XAssetType::ASSET_TYPE_PHYS_COLLMAP, asset->physCollmap->name);
		}
	}

	void IXModel::Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Assert_Size(Game::XModel, 304);

		Utils::Stream* buffer = builder->GetBuffer();
		Game::XModel* asset = header.model;
		Game::XModel* dest = buffer->Dest<Game::XModel>();
		buffer->Save(asset, sizeof(Game::XModel));

		buffer->PushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->SaveString(builder->GetAssetName(this->GetType(), asset->name));
			dest->name = reinterpret_cast<char*>(-1);
		}

		if (asset->boneNames)
		{
			buffer->Align(Utils::Stream::ALIGN_2);

			unsigned short* destBoneNames = buffer->Dest<unsigned short>();
			buffer->SaveArray(asset->boneNames, asset->numBones);
			
			for (char i = 0; i < asset->numBones; ++i)
			{
				builder->MapScriptString(&destBoneNames[i]);
			}

			dest->boneNames = reinterpret_cast<short*>(-1);
		}

		if (asset->parentList)
		{
			buffer->Save(asset->parentList, asset->numBones - asset->numRootBones);
			dest->parentList = reinterpret_cast<char*>(-1);
		}

		if (asset->tagAngles)
		{
			Assert_Size(Game::XModelAngle, 8);

			buffer->Align(Utils::Stream::ALIGN_2);
			buffer->SaveArray(asset->tagAngles, asset->numBones - asset->numRootBones);
			dest->tagAngles = reinterpret_cast<Game::XModelAngle*>(-1);
		}

		if (asset->tagPositions)
		{
			Assert_Size(Game::XModelTagPos, 12);

			buffer->Align(Utils::Stream::ALIGN_4);
			buffer->SaveArray(asset->tagPositions, asset->numBones - asset->numRootBones);
			dest->tagPositions = reinterpret_cast<Game::XModelTagPos*>(-1);
		}

		if (asset->partClassification)
		{
			buffer->Save(asset->partClassification, asset->numBones);
			dest->partClassification = reinterpret_cast<char*>(-1);
		}

		if (asset->animMatrix)
		{
			Assert_Size(Game::DObjAnimMat, 32);

			buffer->Align(Utils::Stream::ALIGN_4);
			buffer->SaveArray(asset->animMatrix, asset->numBones);
			dest->animMatrix = reinterpret_cast<Game::DObjAnimMat*>(-1);
		}

		if (asset->materials)
		{
			buffer->Align(Utils::Stream::ALIGN_4);

			Game::Material** destMaterials = buffer->Dest<Game::Material*>();
			buffer->SaveArray(asset->materials, asset->numSurfaces);

			for (char i = 0; i < asset->numSurfaces; ++i)
			{
				if (asset->materials[i])
				{
					destMaterials[i] = builder->RequireAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->materials[i]->name).material;
				}
			}

			dest->materials = reinterpret_cast<Game::Material**>(-1);
		}

		// Save_XModelLodInfoArray
		{
			Assert_Size(Game::XModelLodInfo, 44);

			for (int i = 0; i < 4; ++i)
			{
				if (asset->lods[i].surfaces)
				{
					// Requiring this asset is not possible, it has to be loaded as part of the model
					//dest->lods[i].surfaces = builder->RequireAsset(Game::XAssetType::ASSET_TYPE_XMODELSURFS, asset->lods[i].surfaces->name).surfaces;

					buffer->PushBlock(Game::XFILE_BLOCK_TEMP);
					buffer->Align(Utils::Stream::ALIGN_4);

					IXModelSurfs().Save({ asset->lods[i].surfaces }, builder);
					dest->lods[i].surfaces = reinterpret_cast<Game::XModelSurfs*>(-1);

					buffer->PopBlock();
				}
			}
		}

		// Save_XModelCollSurfArray
		if (asset->colSurf)
		{
			Assert_Size(Game::XModelCollSurf, 44);

			buffer->Align(Utils::Stream::ALIGN_4);

			Game::XModelCollSurf* destColSurfs = buffer->Dest<Game::XModelCollSurf>();
			buffer->SaveArray(asset->colSurf, asset->numColSurfs);

			for (int i = 0; i < asset->numColSurfs; ++i)
			{
				Game::XModelCollSurf* destColSurf = &destColSurfs[i];
				Game::XModelCollSurf* colSurf = &asset->colSurf[i];

				if (colSurf->tris)
				{
					buffer->Align(Utils::Stream::ALIGN_4);
					buffer->Save(colSurf->tris, 48, colSurf->count);
					destColSurf->tris = reinterpret_cast<void*>(-1);
				}
			}

			dest->colSurf = reinterpret_cast<Game::XModelCollSurf*>(-1);
		}

		if (asset->boneInfo)
		{
			buffer->Align(Utils::Stream::ALIGN_4);

			buffer->Save(asset->boneInfo, 28, asset->numBones);
			dest->boneInfo = reinterpret_cast<char*>(-1);
		}

		if (asset->physPreset)
		{
			dest->physPreset = builder->RequireAsset(Game::XAssetType::ASSET_TYPE_PHYSPRESET, asset->physPreset->name).physPreset;
		}

		if (asset->physCollmap)
		{
			dest->physCollmap = builder->RequireAsset(Game::XAssetType::ASSET_TYPE_PHYS_COLLMAP, asset->physCollmap->name).physCollmap;
		}

		buffer->PopBlock();
	}
}
