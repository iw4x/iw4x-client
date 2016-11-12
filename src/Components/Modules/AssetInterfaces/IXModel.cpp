#include <STDInclude.hpp>

namespace Assets
{
	void IXModel::Load(Game::XAssetHeader* header, std::string name, Components::ZoneBuilder::Zone* builder)
	{
		Components::FileSystem::File modelFile(fmt::sprintf("xmodel/%s.iw4xModel", name.data()));

		if (modelFile.Exists())
		{
			Game::XModel* baseModel = Components::AssetHandler::FindOriginalAsset(Game::XAssetType::ASSET_TYPE_XMODEL, "viewmodel_mp5k").model;

			// Allocate new model and copy the base data to it
			Game::XModel* model = builder->GetAllocator()->Allocate<Game::XModel>();
			std::memcpy(model, baseModel, sizeof(Game::XModel));

			Utils::Stream::Reader reader(builder->GetAllocator(), modelFile.GetBuffer());

			model->name = reader.ReadCString();
			model->numBones = reader.ReadByte();
			model->numRootBones = reader.ReadByte();
			model->numSurfaces = reader.ReadByte();
			model->numColSurfs = reader.Read<int>();

			// Read bone names
			model->boneNames = builder->GetAllocator()->AllocateArray<short>(model->numBones);
			for (int i = 0; i < model->numBones; ++i)
			{
				model->boneNames[i] = Game::SL_GetString(reader.ReadCString(), 0);
			}

			// Bone count
			int boneCount = (model->numBones - model->numRootBones);

			// Read bone data
			model->parentList = reader.ReadArray<char>(boneCount);
			model->tagAngles = reader.ReadArray<Game::XModelAngle>(boneCount);
			model->tagPositions = reader.ReadArray<Game::XModelTagPos>(boneCount);
			model->partClassification = reader.ReadArray<char>(boneCount);
			model->animMatrix = reader.ReadArray<Game::DObjAnimMat>(boneCount);

			// Prepare surfaces
			Game::XSurface* baseSurface = &baseModel->lods[0].surfaces[0].surfaces[0];
			Game::XModelSurfs* surf = builder->GetAllocator()->Allocate<Game::XModelSurfs>();

			std::memcpy(surf, baseModel->lods[0].surfaces, sizeof(Game::XModelSurfs));
			surf->name = builder->GetAllocator()->DuplicateString(fmt::sprintf("%s_lod1", model->name));
			surf->surfaces = builder->GetAllocator()->AllocateArray<Game::XSurface>(model->numSurfaces);
			surf->numSurfaces = model->numSurfaces;

			// Reset surfaces in remaining lods
			for (unsigned int i = 1; i < 4; ++i)
			{
				ZeroMemory(&model->lods[i], sizeof(Game::XModelLodInfo));
			}

			model->lods[0].dist = reader.Read<float>();
			model->lods[0].numSurfs = reader.Read<short>();
			model->lods[0].maxSurfs = reader.Read<short>();

			model->lods[0].partBits[0] = reader.Read<int>();
			model->lods[0].partBits[1] = reader.Read<int>();
			model->lods[0].partBits[2] = reader.Read<int>();
			model->lods[0].partBits[3] = reader.Read<int>();

			model->lods[0].numSurfs = model->numSurfaces; // This is needed in case we have more than 1 LOD
			model->lods[0].surfaces = surf;
			model->lods[0].surfs = surf->surfaces;
			model->numLods = 1;

			// Read surfaces
			for (int i = 0; i < surf->numSurfaces; ++i)
			{
				Game::XSurface* surface = &surf->surfaces[i];
				std::memcpy(surface, baseSurface, sizeof(Game::XSurface));

				surface->tileMode = reader.Read<char>();
				surface->deformed = reader.Read<char>();

				surface->streamHandle = reader.Read<unsigned char>();
				surface->flags = reader.Read<unsigned short>();
				surface->something = reader.Read<short>();
				surface->something2 = reader.Read<int>();

				surface->pad2 = reader.Read<char>();
				surface->pad3 = reader.Read<int>();

				surface->numVertices = reader.Read<unsigned short>();
				surface->numPrimitives = reader.Read<unsigned short>();
				surface->numCT = reader.Read<int>();

				surface->blendNum1 = reader.Read<short>();
				surface->blendNum2 = reader.Read<short>();
				surface->blendNum3 = reader.Read<short>();
				surface->blendNum4 = reader.Read<short>();

				surface->blendInfo = reinterpret_cast<char*>(reader.Read(2, surface->blendNum1 + (3 * surface->blendNum2) + (5 * surface->blendNum3) + (7 * surface->blendNum4)));

				surface->vertexBuffer = reader.ReadArray<Game::GfxPackedVertex>(surface->numVertices);
				surface->indexBuffer = reader.ReadArray<Game::Face>(surface->numPrimitives);

				// Read vert list
				if (reader.ReadByte())
				{
					surface->ct = reader.ReadArray<Game::XRigidVertList>(surface->numCT);

					for (int j = 0; j < surface->numCT; ++j)
					{
						Game::XRigidVertList* vertList = &surface->ct[j];

						vertList->entry = reader.ReadArray<Game::XSurfaceCollisionTree>();
						vertList->entry->node = reinterpret_cast<char*>(reader.Read(16, vertList->entry->numNode));
						vertList->entry->leaf = reader.ReadArray<short>(vertList->entry->numLeaf);
					}
				}
				else
				{
					surface->ct = nullptr;
				}
			}

			// Read materials
			model->materials = builder->GetAllocator()->AllocateArray<Game::Material*>(model->numSurfaces);
			for (char i = 0; i < model->numSurfaces; ++i)
			{
				model->materials[i] = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_MATERIAL, reader.ReadString(), builder).material;
			}

			// Read collision surfaces
			if (reader.ReadByte())
			{
				model->colSurf = reader.ReadArray<Game::XModelCollSurf>(model->numColSurfs);
				
				for (int i = 0; i < model->numColSurfs; ++i)
				{
					if (model->colSurf[i].tris)
					{
						model->colSurf[i].tris = reader.Read(48, model->colSurf[i].count);
					}
				}
			}
			else
			{
				model->colSurf = nullptr;
			}

			// Read bone info
			if (reader.ReadByte())
			{
				model->boneInfo = reader.ReadArray<Game::XBoneInfo>(model->numBones);
			}
			else
			{
				model->boneInfo = nullptr;
			}

			if (!reader.End())
			{
				Components::Logger::Error(0, "Reading model '%s' failed, remaining raw data found!", name.data());
			}

			header->model = model;
		}
	}

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
		buffer->Save(asset);

		buffer->PushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->SaveString(builder->GetAssetName(this->GetType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
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

			Utils::Stream::ClearPointer(&dest->boneNames);
		}

		if (asset->parentList)
		{
			buffer->Save(asset->parentList, asset->numBones - asset->numRootBones);
			Utils::Stream::ClearPointer(&dest->parentList);
		}

		if (asset->tagAngles)
		{
			Assert_Size(Game::XModelAngle, 8);

			buffer->Align(Utils::Stream::ALIGN_2);
			buffer->SaveArray(asset->tagAngles, asset->numBones - asset->numRootBones);
			Utils::Stream::ClearPointer(&dest->tagAngles);
		}

		if (asset->tagPositions)
		{
			Assert_Size(Game::XModelTagPos, 12);

			buffer->Align(Utils::Stream::ALIGN_4);
			buffer->SaveArray(asset->tagPositions, asset->numBones - asset->numRootBones);
			Utils::Stream::ClearPointer(&dest->tagPositions);
		}

		if (asset->partClassification)
		{
			buffer->Save(asset->partClassification, asset->numBones);
			Utils::Stream::ClearPointer(&dest->partClassification);
		}

		if (asset->animMatrix)
		{
			Assert_Size(Game::DObjAnimMat, 32);

			buffer->Align(Utils::Stream::ALIGN_4);
			buffer->SaveArray(asset->animMatrix, asset->numBones);
			Utils::Stream::ClearPointer(&dest->animMatrix);
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

			Utils::Stream::ClearPointer(&dest->materials);
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
					Utils::Stream::ClearPointer(&dest->lods[i].surfaces);

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
					Utils::Stream::ClearPointer(&destColSurf->tris);
				}
			}

			Utils::Stream::ClearPointer(&dest->colSurf);
		}

		if (asset->boneInfo)
		{
			Assert_Size(Game::XBoneInfo, 28);

			buffer->Align(Utils::Stream::ALIGN_4);

			buffer->SaveArray(asset->boneInfo, asset->numBones);
			Utils::Stream::ClearPointer(&dest->boneInfo);
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
