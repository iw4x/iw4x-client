#include <STDInclude.hpp>
#include "IXModel.hpp"

#define IW4X_MODEL_VERSION 8

namespace Assets
{
	void IXModel::loadXSurfaceCollisionTree(Game::XSurfaceCollisionTree* entry, Utils::Stream::Reader* reader)
	{
		if (entry->nodes)
		{
			entry->nodes = reader->readArrayOnce<Game::XSurfaceCollisionNode>(entry->nodeCount);
		}

		if (entry->leafs)
		{
			entry->leafs = reader->readArrayOnce<Game::XSurfaceCollisionLeaf>(entry->leafCount);
		}
	}

	void IXModel::loadXSurface(Game::XSurface* surf, Utils::Stream::Reader* reader, [[maybe_unused]] Components::ZoneBuilder::Zone* builder)
	{
		if (surf->vertInfo.vertsBlend)
		{
			surf->vertInfo.vertsBlend = reader->readArrayOnce<unsigned short>(surf->vertInfo.vertCount[0] + (surf->vertInfo.vertCount[1] * 3) + (surf->vertInfo.vertCount[2] * 5) + (surf->vertInfo.vertCount[3] * 7));
		}

		// Access vertex block
		if (surf->verts0)
		{
			surf->verts0 = reader->readArrayOnce<Game::GfxPackedVertex>(surf->vertCount);
		}

		// Save_XRigidVertListArray
		if (surf->vertList)
		{
			surf->vertList = reader->readArrayOnce<Game::XRigidVertList>(surf->vertListCount);

			for (unsigned int i = 0; i < surf->vertListCount; ++i)
			{
				Game::XRigidVertList* rigidVertList = &surf->vertList[i];

				if (rigidVertList->collisionTree)
				{
					rigidVertList->collisionTree = reader->readObject<Game::XSurfaceCollisionTree>();
					this->loadXSurfaceCollisionTree(rigidVertList->collisionTree, reader);
				}
			}
		}

		// Access index block
		if (surf->triIndices)
		{
			surf->triIndices = reader->readArrayOnce<unsigned short>(surf->triCount * 3);
		}
	}

	void IXModel::loadXModelSurfs(Game::XModelSurfs* asset, Utils::Stream::Reader* reader, Components::ZoneBuilder::Zone* builder)
	{
		if (asset->name)
		{
			asset->name = reader->readCString();
		}

		if (asset->surfs)
		{
			asset->surfs = reader->readArrayOnce<Game::XSurface>(asset->numsurfs);

			for (int i = 0; i < asset->numsurfs; ++i)
			{
				this->loadXSurface(&asset->surfs[i], reader, builder);
			}
		}
	}

	void IXModel::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		Components::FileSystem::File modelFile(std::format("xmodel/{}.iw4xModel", name));

		if (!builder->isPrimaryAsset() && (!Components::ZoneBuilder::ZBPreferDiskAssets.get<bool>() || !modelFile.exists()))
		{
			header->model = Components::AssetHandler::FindOriginalAsset(this->getType(), name.data()).model;
			if (header->model) return;
		}


		if (modelFile.exists())
		{
			Utils::Stream::Reader reader(builder->getAllocator(), modelFile.getBuffer());

			__int64 magic = reader.read<__int64>();
			if (std::memcmp(&magic, "IW4xModl", 8))
			{
				Components::Logger::Error(Game::ERR_FATAL, "Reading model '{}' failed, header is invalid!", name);
			}

			int version = reader.read<int>();
			if (version != IW4X_MODEL_VERSION)
			{
				Components::Logger::Error(Game::ERR_FATAL, "Reading model '{}' failed, expected version is {}, but it was {}!", name, IW4X_MODEL_VERSION, version);
			}

			Game::XModel* asset = reader.readObject<Game::XModel>();

			if (asset->name)
			{
				asset->name = reader.readCString();
			}

			if (asset->boneNames)
			{
				asset->boneNames = builder->getAllocator()->allocateArray<unsigned short>(asset->numBones);

				for (char i = 0; i < asset->numBones; ++i)
				{
					asset->boneNames[i] = static_cast<std::uint16_t>(Game::SL_GetString(reader.readCString(), 0));
				}
			}

			if (asset->parentList)
			{
				asset->parentList = reader.readArrayOnce<unsigned char>(asset->numBones - asset->numRootBones);
			}

			if (asset->quats)
			{
				asset->quats = reader.readArrayOnce<short>((asset->numBones - asset->numRootBones) * 4);
			}

			if (asset->trans)
			{
				asset->trans = reader.readArrayOnce<float>((asset->numBones - asset->numRootBones) * 3);
			}

			if (asset->partClassification)
			{
				asset->partClassification = reader.readArrayOnce<unsigned char>(asset->numBones);
			}

			if (asset->baseMat)
			{
				asset->baseMat = reader.readArrayOnce<Game::DObjAnimMat>(asset->numBones);
			}

			if (asset->materialHandles)
			{
				asset->materialHandles = reader.readArray<Game::Material*>(asset->numsurfs);

				for (unsigned char i = 0; i < asset->numsurfs; ++i)
				{
					if (asset->materialHandles[i])
					{
						asset->materialHandles[i] = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_MATERIAL, reader.readString(), builder).material;
					}
				}
			}

			// Save_XModelLodInfoArray
			{
				for (unsigned int i = 0; i < 4; ++i)
				{
					if (asset->lodInfo[i].modelSurfs)
					{
						asset->lodInfo[i].modelSurfs = reader.readObject<Game::XModelSurfs>();
						this->loadXModelSurfs(asset->lodInfo[i].modelSurfs, &reader, builder);
						Components::AssetHandler::StoreTemporaryAsset(Game::XAssetType::ASSET_TYPE_XMODEL_SURFS, { asset->lodInfo[i].modelSurfs });

						asset->lodInfo[i].surfs = asset->lodInfo[i].modelSurfs->surfs;

						// Zero that for now, it breaks the models.
						// TODO: Figure out how that can be converted
						asset->lodInfo[i].smcBaseIndexPlusOne = 0;
						asset->lodInfo[i].smcSubIndexMask = 0;
						asset->lodInfo[i].smcBucket = 0;
					}
				}
			}

			// Save_XModelCollSurfArray
			if (asset->collSurfs)
			{
				asset->collSurfs = reader.readArray<Game::XModelCollSurf_s>(asset->numCollSurfs);

				for (int i = 0; i < asset->numCollSurfs; ++i)
				{
					Game::XModelCollSurf_s* collSurf = &asset->collSurfs[i];

					if (collSurf->collTris)
					{
						collSurf->collTris = reader.readArray<Game::XModelCollTri_s>(collSurf->numCollTris);
					}
				}
			}

			if (asset->boneInfo)
			{
				asset->boneInfo = reader.readArray<Game::XBoneInfo>(asset->numBones);
			}

			if (asset->physPreset)
			{
				asset->physPreset = reader.readObject<Game::PhysPreset>();

				if (asset->physPreset->name)
				{
					asset->physPreset->name = reader.readCString();
				}

				if (asset->physPreset->sndAliasPrefix)
				{
					asset->physPreset->sndAliasPrefix = reader.readCString();
				}

				// This is an experiment, ak74 fails though
				if (asset->name == "weapon_ak74u"s)
				{
					asset->physPreset = nullptr;
				}
				else
				{
					Game::PhysPreset* preset = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_PHYSPRESET, asset->physPreset->name, builder).physPreset;
					if (preset)
					{
						asset->physPreset = preset;
					}
					else
					{
						Components::AssetHandler::StoreTemporaryAsset(Game::XAssetType::ASSET_TYPE_PHYSPRESET, { asset->physPreset });
					}
				}
			}

			if (asset->physCollmap)
			{
				Game::PhysCollmap* collmap = reader.readObject<Game::PhysCollmap>();
				asset->physCollmap = collmap;

				if (collmap->name)
				{
					collmap->name = reader.readCString();
				}

				if (collmap->geoms)
				{
					collmap->geoms = reader.readArray<Game::PhysGeomInfo>(collmap->count);

					for (unsigned int i = 0; i < collmap->count; ++i)
					{
						Game::PhysGeomInfo* geom = &collmap->geoms[i];

						if (geom->brushWrapper)
						{
							Game::BrushWrapper* brush = reader.readObject<Game::BrushWrapper>();
							geom->brushWrapper = brush;
							{
								if (brush->brush.sides)
								{
									brush->brush.sides = reader.readArray<Game::cbrushside_t>(brush->brush.numsides);
									for (unsigned short j = 0; j < brush->brush.numsides; ++j)
									{
										Game::cbrushside_t* side = &brush->brush.sides[j];

										// TODO: Add pointer support
										if (side->plane)
										{
											side->plane = reader.readObject<Game::cplane_s>();
										}
									}
								}

								if (brush->brush.baseAdjacentSide)
								{
									brush->brush.baseAdjacentSide = reader.readArray<char>(brush->totalEdgeCount);
								}
							}

							// TODO: Add pointer support
							if (brush->planes)
							{
								brush->planes = reader.readArray<Game::cplane_s>(brush->brush.numsides);
							}
						}
					}

					Components::AssetHandler::StoreTemporaryAsset(Game::XAssetType::ASSET_TYPE_PHYSCOLLMAP, { asset->physCollmap });
					// asset->physCollmap = nullptr;
				}
			}

			if (!reader.end())
			{
				Components::Logger::Error(Game::ERR_FATAL, "Reading model '{}' failed, remaining raw data found!", name);
			}

			header->model = asset;
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
