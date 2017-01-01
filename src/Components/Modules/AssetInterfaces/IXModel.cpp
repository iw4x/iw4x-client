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

			model->name = reader.readCString();
			model->numBones = reader.readByte();
			model->numRootBones = reader.readByte();
			model->numSurfaces = reader.read<unsigned char>();
			model->numColSurfs = reader.read<int>();
			model->numLods = static_cast<char>(reader.read<short>());
			model->collLod = reader.read<short>();

			// Read bone names
			model->boneNames = builder->getAllocator()->allocateArray<short>(model->numBones);
			for (int i = 0; i < model->numBones; ++i)
			{
				model->boneNames[i] = Game::SL_GetString(reader.readCString(), 0);
			}

			// Bone count
			int boneCount = (model->numBones - model->numRootBones);

			// Read bone data
			model->parentList = reader.readArray<char>(boneCount);
			model->tagAngles = reader.readArray<Game::XModelAngle>(boneCount);
			model->tagPositions = reader.readArray<Game::XModelTagPos>(boneCount);
			model->partClassification = reader.readArray<char>(boneCount);
			model->animMatrix = reader.readArray<Game::DObjAnimMat>(boneCount);

			// Prepare surfaces
			Game::XModelSurfs surf;
			Utils::Memory::Allocator allocator;
			Game::XSurface* baseSurface = &baseModel->lods[0].modelSurfs[0].surfaces[0];

			std::memcpy(&surf, baseModel->lods[0].modelSurfs, sizeof(Game::XModelSurfs));
			surf.surfaces = allocator.allocateArray<Game::XSurface>(model->numSurfaces);
			surf.numSurfaces = model->numSurfaces;

			for (int i = 0; i < 4; ++i)
			{
				model->lods[i].dist = reader.read<float>();
				model->lods[i].numsurfs = reader.read<unsigned short>();
				model->lods[i].surfIndex = reader.read<unsigned short>();

				model->lods[i].partBits[0] = reader.read<int>();
				model->lods[i].partBits[1] = reader.read<int>();
				model->lods[i].partBits[2] = reader.read<int>();
				model->lods[i].partBits[3] = reader.read<int>();
				model->lods[i].partBits[4] = 0;
				model->lods[i].partBits[5] = 0;
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

				realSurf->numSurfaces = model->lods[i].numsurfs;
				realSurf->surfaces = builder->getAllocator()->allocateArray<Game::XSurface>(realSurf->numSurfaces);

				std::memcpy(realSurf->surfaces, &surf.surfaces[model->lods[i].surfIndex], sizeof(Game::XSurface) * realSurf->numSurfaces);
				std::memcpy(realSurf->partBits, model->lods[i].partBits, sizeof(realSurf->partBits));

				model->lods[i].modelSurfs = realSurf;
				model->lods[i].surfs = realSurf->surfaces;

				// Store surfs for later writing
				Components::AssetHandler::StoreTemporaryAsset(Game::XAssetType::ASSET_TYPE_XMODELSURFS, { realSurf });
			}

			// Read materials
			model->materials = builder->getAllocator()->allocateArray<Game::Material*>(model->numSurfaces);
			for (unsigned char i = 0; i < model->numSurfaces; ++i)
			{
				model->materials[i] = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_MATERIAL, reader.readString(), builder).material;
			}

			// Read collision surfaces
			if (reader.readByte())
			{
				model->colSurf = reader.readArray<Game::XModelCollSurf>(model->numColSurfs);

				for (int i = 0; i < model->numColSurfs; ++i)
				{
					if (model->colSurf[i].tris)
					{
						model->colSurf[i].tris = reader.read(48, model->colSurf[i].count);
					}
				}
			}
			else
			{
				model->colSurf = nullptr;
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

		if (asset->materials)
		{
			for (unsigned char i = 0; i < asset->numSurfaces; ++i)
			{
				if (asset->materials[i])
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->materials[i]);
				}
			}
		}

		for (int i = 0; i < 4; ++i)
		{
			if (asset->lods[i].modelSurfs)
			{
				builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODELSURFS, asset->lods[i].modelSurfs);
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

		if (asset->tagAngles)
		{
			AssertSize(Game::XModelAngle, 8);

			buffer->align(Utils::Stream::ALIGN_2);
			buffer->saveArray(asset->tagAngles, asset->numBones - asset->numRootBones);
			Utils::Stream::ClearPointer(&dest->tagAngles);
		}

		if (asset->tagPositions)
		{
			AssertSize(Game::XModelTagPos, 12);

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->tagPositions, asset->numBones - asset->numRootBones);
			Utils::Stream::ClearPointer(&dest->tagPositions);
		}

		if (asset->partClassification)
		{
			buffer->save(asset->partClassification, asset->numBones);
			Utils::Stream::ClearPointer(&dest->partClassification);
		}

		if (asset->animMatrix)
		{
			AssertSize(Game::DObjAnimMat, 32);

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->animMatrix, asset->numBones);
			Utils::Stream::ClearPointer(&dest->animMatrix);
		}

		if (asset->materials)
		{
			buffer->align(Utils::Stream::ALIGN_4);

			Game::Material** destMaterials = buffer->dest<Game::Material*>();
			buffer->saveArray(asset->materials, asset->numSurfaces);

			for (unsigned char i = 0; i < asset->numSurfaces; ++i)
			{
				if (asset->materials[i])
				{
					destMaterials[i] = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->materials[i]).material;
				}
			}

			Utils::Stream::ClearPointer(&dest->materials);
		}

		// Save_XModelLodInfoArray
		{
			AssertSize(Game::XModelLodInfo, 44);

			for (int i = 0; i < 4; ++i)
			{
				if (asset->lods[i].modelSurfs)
				{
					dest->lods[i].modelSurfs = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_XMODELSURFS, asset->lods[i].modelSurfs).surfaces;
				}
			}
		}

		// Save_XModelCollSurfArray
		if (asset->colSurf)
		{
			AssertSize(Game::XModelCollSurf, 44);

			buffer->align(Utils::Stream::ALIGN_4);

			Game::XModelCollSurf* destColSurfs = buffer->dest<Game::XModelCollSurf>();
			buffer->saveArray(asset->colSurf, asset->numColSurfs);

			for (int i = 0; i < asset->numColSurfs; ++i)
			{
				Game::XModelCollSurf* destColSurf = &destColSurfs[i];
				Game::XModelCollSurf* colSurf = &asset->colSurf[i];

				if (colSurf->tris)
				{
					buffer->align(Utils::Stream::ALIGN_4);

					buffer->save(colSurf->tris, 48, colSurf->count);
					Utils::Stream::ClearPointer(&destColSurf->tris);
				}
			}

			Utils::Stream::ClearPointer(&dest->colSurf);
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
