#include <STDInclude.hpp>

#define IW4X_MODEL_VERSION 3

namespace Assets
{
	void IXModel::load(Game::XAssetHeader* header, std::string name, Components::ZoneBuilder::Zone* builder)
	{
		Components::FileSystem::File modelFile(fmt::sprintf("xmodel/%s.iw4xModel", name.data()));

		if (modelFile.exists())
		{
			Game::XModel* baseModel = Components::AssetHandler::FindOriginalAsset(Game::XAssetType::ASSET_TYPE_XMODEL, "viewmodel_mp5k").model;

			// Allocate new model and copy the base data to it
			Game::XModel* model = builder->getAllocator()->allocate<Game::XModel>();
			std::memcpy(model, baseModel, sizeof(Game::XModel));

			Utils::Stream::Reader reader(builder->getAllocator(), modelFile.getBuffer());

			if (reader.read<__int64>() != *reinterpret_cast<__int64*>("IW4xModl"))
			{
				Components::Logger::Error(0, "Reading model '%s' failed, header is invalid!", name.data());
			}

			int version = reader.read<int>();
			if (version != IW4X_MODEL_VERSION)
			{
				Components::Logger::Error(0, "Reading model '%s' failed, expected version is %d, but it was %d!", name.data(), IW4X_MODEL_VERSION, version);
			}

			ZeroMemory(model->noScalePartBits, sizeof model->noScalePartBits);

			model->name = reader.readCString();
			model->numBones = reader.readByte();
			model->numRootBones = reader.readByte();
			model->numsurfs = reader.read<unsigned char>();
			model->numCollSurfs = reader.read<int>();
			model->numLods = static_cast<char>(reader.read<short>());
			model->collLod = static_cast<char>(reader.read<short>());

			// Read bone names
			model->boneNames = builder->getAllocator()->allocateArray<unsigned short>(model->numBones);
			for (int i = 0; i < model->numBones; ++i)
			{
				model->boneNames[i] = Game::SL_GetString(reader.readCString(), 0);
			}

			// Bone count
			int boneCount = (model->numBones - model->numRootBones);

			// Read bone data
			model->parentList = reader.readArray<char>(boneCount);
			model->quats = reader.readArray<short>(boneCount * 4);
			model->trans = reader.readArray<float>(boneCount * 3);
			model->partClassification = reader.readArray<char>(boneCount);
			model->baseMat = reader.readArray<Game::DObjAnimMat>(boneCount);

			// Prepare surfaces
			Game::XModelSurfs surf;
			Utils::Memory::Allocator allocator;
			Game::XSurface* baseSurface = &baseModel->lodInfo[0].modelSurfs[0].surfaces[0];

			std::memcpy(&surf, baseModel->lodInfo[0].modelSurfs, sizeof(Game::XModelSurfs));
			surf.surfaces = allocator.allocateArray<Game::XSurface>(model->numsurfs);
			surf.numSurfaces = model->numsurfs;

			for (int i = 0; i < 4; ++i)
			{
				model->lodInfo[i].dist = reader.read<float>();
				model->lodInfo[i].numsurfs = reader.read<unsigned short>();
				model->lodInfo[i].surfIndex = reader.read<unsigned short>();

				model->lodInfo[i].partBits[0] = reader.read<int>();
				model->lodInfo[i].partBits[1] = reader.read<int>();
				model->lodInfo[i].partBits[2] = reader.read<int>();
				model->lodInfo[i].partBits[3] = reader.read<int>();
				model->lodInfo[i].partBits[4] = 0;
				model->lodInfo[i].partBits[5] = 0;
			}

			// Read surfaces
			for (int i = 0; i < surf.numSurfaces; ++i)
			{
				Game::XSurface* surface = &surf.surfaces[i];
				std::memcpy(surface, baseSurface, sizeof(Game::XSurface));

				surface->tileMode = reader.read<char>();
				surface->deformed = reader.read<char>();

				surface->streamHandle = reader.read<unsigned char>();
				surface->partBits[0] = reader.read<int>();
				surface->partBits[1] = reader.read<int>();
				surface->partBits[2] = reader.read<int>();
				surface->partBits[3] = reader.read<int>();
				surface->partBits[4] = 0;
				surface->partBits[5] = 0;

				surface->baseTriIndex = reader.read<unsigned __int16>();
				surface->baseVertIndex = reader.read<unsigned __int16>();

				surface->numVertices = reader.read<unsigned short>();
				surface->numPrimitives = reader.read<unsigned short>();
				surface->numCT = reader.read<int>();

				surface->blendNum1 = reader.read<short>();
				surface->blendNum2 = reader.read<short>();
				surface->blendNum3 = reader.read<short>();
				surface->blendNum4 = reader.read<short>();

				surface->blendInfo = reinterpret_cast<char*>(reader.read(2, surface->blendNum1 + (3 * surface->blendNum2) + (5 * surface->blendNum3) + (7 * surface->blendNum4)));

				surface->vertexBuffer = reader.readArray<Game::GfxPackedVertex>(surface->numVertices);
				surface->indexBuffer = reader.readArray<Game::Face>(surface->numPrimitives);

				// Read vert list
				if (reader.readByte())
				{
					surface->ct = reader.readArray<Game::XRigidVertList>(surface->numCT);

					for (int j = 0; j < surface->numCT; ++j)
					{
						Game::XRigidVertList* vertList = &surface->ct[j];

						vertList->entry = reader.readArray<Game::XSurfaceCollisionTree>();
						vertList->entry->node = reinterpret_cast<char*>(reader.read(16, vertList->entry->numNode));
						vertList->entry->leaf = reader.readArray<short>(vertList->entry->numLeaf);
					}
				}
				else
				{
					surface->ct = nullptr;
				}
			}

			// When all surfaces are loaded, split them up. 
			for (char i = 0; i < model->numLods; ++i)			
			{
				Game::XModelSurfs* realSurf = builder->getAllocator()->allocate<Game::XModelSurfs>();

				// Usually, a binary representation is used for the index, but meh.
				realSurf->name = builder->getAllocator()->duplicateString(fmt::sprintf("%s_lod%d", model->name, i & 0xFF));

				realSurf->numSurfaces = model->lodInfo[i].numsurfs;
				realSurf->surfaces = builder->getAllocator()->allocateArray<Game::XSurface>(realSurf->numSurfaces);

				std::memcpy(realSurf->surfaces, &surf.surfaces[model->lodInfo[i].surfIndex], sizeof(Game::XSurface) * realSurf->numSurfaces);
				std::memcpy(realSurf->partBits, model->lodInfo[i].partBits, sizeof(realSurf->partBits));

				model->lodInfo[i].modelSurfs = realSurf;
				model->lodInfo[i].surfs = realSurf->surfaces;

				// Store surfs for later writing
				Components::AssetHandler::StoreTemporaryAsset(Game::XAssetType::ASSET_TYPE_XMODELSURFS, { realSurf });
			}

			// Read materials
			model->materialHandles = builder->getAllocator()->allocateArray<Game::Material*>(model->numsurfs);
			for (char i = 0; i < model->numsurfs; ++i)
			{
				model->materialHandles[i] = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_MATERIAL, reader.readString(), builder).material;
			}

			// Read collision surfaces
			if (reader.readByte())
			{
				model->collSurfs = reader.readArray<Game::XModelCollSurf_s>(model->numCollSurfs);

				for (int i = 0; i < model->numCollSurfs; ++i)
				{
					if (model->collSurfs[i].collTris)
					{
						model->collSurfs[i].collTris = reader.readArray<Game::XModelCollTri_s>(model->collSurfs[i].numCollTris);
					}
				}
			}
			else
			{
				model->collSurfs = nullptr;
			}

			// Read bone info
			if (reader.readByte())
			{
				model->boneInfo = reader.readArray<Game::XBoneInfo>(model->numBones);
			}
			else
			{
				model->boneInfo = nullptr;
			}

			if (!reader.end())
			{
				Components::Logger::Error(0, "Reading model '%s' failed, remaining raw data found!", name.data());
			}

			header->model = model;
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
				builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODELSURFS, asset->lodInfo[i].modelSurfs);
			}
		}

		if (asset->physPreset)
		{
			builder->loadAsset(Game::XAssetType::ASSET_TYPE_PHYSPRESET, asset->physPreset);
		}

		if (asset->physCollmap)
		{
			builder->loadAsset(Game::XAssetType::ASSET_TYPE_PHYS_COLLMAP, asset->physCollmap);
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
				builder->mapScriptString(&destBoneNames[i]);
			}

			Utils::Stream::ClearPointer(&dest->boneNames);
		}

		if (asset->parentList)
		{
			buffer->save(asset->parentList, asset->numBones - asset->numRootBones);
			Utils::Stream::ClearPointer(&dest->parentList);
		}

		if (asset->quats)
		{
			buffer->align(Utils::Stream::ALIGN_2);
			buffer->saveArray(asset->quats, (asset->numBones - asset->numRootBones) * 4);
			Utils::Stream::ClearPointer(&dest->quats);
		}

		if (asset->trans)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->trans, (asset->numBones - asset->numRootBones) * 3);
			Utils::Stream::ClearPointer(&dest->trans);
		}

		if (asset->partClassification)
		{
			buffer->save(asset->partClassification, asset->numBones);
			Utils::Stream::ClearPointer(&dest->partClassification);
		}

		if (asset->baseMat)
		{
			AssertSize(Game::DObjAnimMat, 32);

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->baseMat, asset->numBones);
			Utils::Stream::ClearPointer(&dest->baseMat);
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
					dest->lodInfo[i].modelSurfs = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_XMODELSURFS, asset->lodInfo[i].modelSurfs).surfaces;
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
			dest->physCollmap = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_PHYS_COLLMAP, asset->physCollmap).physCollmap;
		}

		buffer->popBlock();
	}
}
