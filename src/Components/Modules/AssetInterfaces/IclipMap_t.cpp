#include <StdInclude.hpp>

#define IW4X_CLIPMAP_VERSION 1

namespace Assets
{
	void IclipMap_t::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::clipMap_t, 256);
		SaveLogEnter("clipMap_t");

		Utils::Stream* buffer = builder->getBuffer();
		Game::clipMap_t* asset = header.clipMap;
		Game::clipMap_t* dest = buffer->dest<Game::clipMap_t>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		if (asset->cPlanes)
		{
			AssertSize(Game::cplane_t, 20);
			SaveLogEnter("cplane_t");

			if (builder->hasPointer(asset->cPlanes))
			{
				dest->cPlanes = builder->getPointer(asset->cPlanes);
			}
			else
			{
				buffer->align(Utils::Stream::ALIGN_4);

				// not sure if this is neede but both brushside and brushedge need it and it can't hurt
				for (int i = 0; i < asset->numCPlanes; i++)
				{
					builder->storePointer(&asset->cPlanes[i]);
					buffer->save(&asset->cPlanes[i]);
				}
				Utils::Stream::ClearPointer(&dest->cPlanes);
			}

			SaveLogExit();
		}

		if (asset->staticModelList)
		{

			AssertSize(Game::cStaticModel_t, 76);
			SaveLogEnter("cStaticModel_t");

			// xmodel is already stored
			buffer->align(Utils::Stream::ALIGN_4);
			Game::cStaticModel_t* destStaticModelList = buffer->dest<Game::cStaticModel_t>();
			buffer->saveArray(asset->staticModelList, asset->numStaticModels);

			for (int i = 0; i < asset->numStaticModels; ++i)
			{
				if (asset->staticModelList[i].xmodel)
				{
					destStaticModelList[i].xmodel = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_XMODEL, asset->staticModelList[i].xmodel).model;
				}
			}

			Utils::Stream::ClearPointer(&dest->staticModelList);
			SaveLogExit();
		}

		if (asset->materials)
		{
			AssertSize(Game::ClipMaterial, 12);
			SaveLogEnter("ClipMaterial");

			buffer->align(Utils::Stream::ALIGN_4);
			Game::ClipMaterial* mats = buffer->dest<Game::ClipMaterial>();
			buffer->saveArray(asset->materials, asset->numMaterials);

			for (int i = 0; i < asset->numMaterials; ++i)
			{
				buffer->saveString(asset->materials[i].name);
				Utils::Stream::ClearPointer(&mats[i].name);
			}

			Utils::Stream::ClearPointer(&dest->materials);
			SaveLogExit();
		}

		if (asset->cBrushSides)
		{
			AssertSize(Game::cbrushside_t, 8);
			SaveLogEnter("cbrushside_t");

			buffer->align(Utils::Stream::ALIGN_4);
			Game::cbrushside_t* sides = buffer->dest<Game::cbrushside_t>();
			// we need the pointer to each of these to be stored so we can't write them all at once
			for (int i = 0; i < asset->numCBrushSides; ++i)
			{
				builder->storePointer(&asset->cBrushSides[i]); // for reference in cBrush
				buffer->save(&asset->cBrushSides[i]);
			}

			for (int i = 0; i < asset->numCBrushSides; ++i)
			{
				if (sides[i].side)
				{
					AssertSize(Game::cplane_t, 20);

					if (builder->hasPointer(sides[i].side))
					{
						sides[i].side = builder->getPointer(sides[i].side);
					}
					else
					{
						buffer->align(Utils::Stream::ALIGN_4);
						builder->storePointer(sides[i].side);

						buffer->save(sides[i].side);
						Utils::Stream::ClearPointer(&sides[i].side);
					}
				}
			}

			Utils::Stream::ClearPointer(&dest->cBrushSides);
			SaveLogExit();
		}

		if (asset->cBrushEdges)
		{
			SaveLogEnter("cBrushEdge");

			// no align for char
			for (int i = 0; i < asset->numCBrushEdges; ++i)
			{
				builder->storePointer(&asset->cBrushEdges[i]); // for reference in cBrush
				buffer->save(&asset->cBrushEdges[i]);
			}
			Utils::Stream::ClearPointer(&dest->cBrushEdges);

			SaveLogExit();
		}

		if (asset->cNodes)
		{
			AssertSize(Game::cNode_t, 8);
			SaveLogEnter("cNode_t");

			buffer->align(Utils::Stream::ALIGN_4);
			Game::cNode_t* nodes = buffer->dest<Game::cNode_t>();
			buffer->saveArray(asset->cNodes, asset->numCNodes);

			for (int i = 0; i < asset->numCNodes; ++i)
			{
				if (nodes[i].plane)
				{
					if (builder->hasPointer(nodes[i].plane))
					{
						nodes[i].plane = builder->getPointer(nodes[i].plane);
					}
					else
					{
						buffer->align(Utils::Stream::ALIGN_4);
						builder->storePointer(nodes[i].plane);

						buffer->save(nodes[i].plane);
						Utils::Stream::ClearPointer(&nodes[i].plane);
					}
				}
			}

			Utils::Stream::ClearPointer(&dest->cNodes);
			SaveLogExit();
		}

		if (asset->cLeaf)
		{
			AssertSize(Game::cLeaf_t, 40);
			SaveLogEnter("cLeaf_t");

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->cLeaf, asset->numCLeaf);
			Utils::Stream::ClearPointer(&dest->cLeaf);
			SaveLogExit();
		}

		if (asset->leafBrushes)
		{
			SaveLogEnter("cLeafBrush_t");

			buffer->align(Utils::Stream::ALIGN_2);
			buffer->saveArray(asset->leafBrushes, asset->numLeafBrushes);
			Utils::Stream::ClearPointer(&dest->leafBrushes);

			SaveLogExit();
		}

		if (asset->cLeafBrushNodes)
		{
			AssertSize(Game::cLeafBrushNode_t, 20);
			SaveLogEnter("cLeafBrushNode_t");

			buffer->align(Utils::Stream::ALIGN_4);
			Game::cLeafBrushNode_t* node = buffer->dest<Game::cLeafBrushNode_t>();
			buffer->saveArray(asset->cLeafBrushNodes, asset->numCLeafBrushNodes);

			for (int i = 0; i < asset->numCLeafBrushNodes; ++i)
			{
				if (node[i].leafBrushCount > 0)
				{
					if (node[i].data.brushes)
					{
						if (builder->hasPointer(node[i].data.brushes))
						{
							node[i].data.brushes = builder->getPointer(node[i].data.brushes);
						}
						else
						{
							buffer->align(Utils::Stream::ALIGN_2);

							for (short j = 0; j < node[i].leafBrushCount; ++j)
							{
								builder->storePointer(&node[i].data.brushes[j]);
								buffer->save(&node[i].data.brushes[j]);
							}

							Utils::Stream::ClearPointer(&node[i].data.brushes);
						}
					}
				}
			}

			Utils::Stream::ClearPointer(&dest->cLeafBrushNodes);
			SaveLogExit();
		}

		if (asset->leafSurfaces)
		{
			SaveLogEnter("cLeafSurface_t");

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->leafSurfaces, asset->numLeafSurfaces);
			Utils::Stream::ClearPointer(&dest->leafSurfaces);

			SaveLogExit();
		}

		if (asset->verts)
		{
			AssertSize(Game::vec3_t, 12);

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->verts, asset->numVerts);
			Utils::Stream::ClearPointer(&dest->verts);
		}

		if (asset->triIndices)
		{
			buffer->align(Utils::Stream::ALIGN_2);
			buffer->save(asset->triIndices, 6, asset->numTriIndices);
			Utils::Stream::ClearPointer(&dest->triIndices);
		}

		if (asset->triEdgeIsWalkable)
		{
			// no align for char
			buffer->save(asset->triEdgeIsWalkable, 1, 4 * ((3 * asset->numTriIndices + 31) >> 5));
			Utils::Stream::ClearPointer(&dest->triEdgeIsWalkable);
		}

		if (asset->collisionBorders)
		{
			AssertSize(Game::CollisionBorder, 28);
			SaveLogEnter("CollisionBorder");

			buffer->align(Utils::Stream::ALIGN_4);

			for (int i = 0; i < asset->numCollisionBorders; ++i)
			{
				builder->storePointer(&asset->collisionBorders[i]);
				buffer->save(&asset->collisionBorders[i]);
			}

			Utils::Stream::ClearPointer(&dest->collisionBorders);
			SaveLogExit();
		}

		if (asset->collisionPartitions)
		{
			AssertSize(Game::CollisionPartition, 12);
			SaveLogEnter("CollisionPartition");

			buffer->align(Utils::Stream::ALIGN_4);
			Game::CollisionPartition* destPartitions = buffer->dest<Game::CollisionPartition>();
			buffer->saveArray(asset->collisionPartitions, asset->numCollisionPartitions);

			for (int i = 0; i < asset->numCollisionPartitions; ++i)
			{
				Game::CollisionPartition* destPartition = &destPartitions[i];
				Game::CollisionPartition* partition = &asset->collisionPartitions[i];

				if (partition->borders)
				{
					if (builder->hasPointer(partition->borders))
					{
						destPartition->borders = builder->getPointer(partition->borders);
					}
					else
					{
						buffer->align(Utils::Stream::ALIGN_4);
						builder->storePointer(partition->borders);
						buffer->save(partition->borders);
						Utils::Stream::ClearPointer(&destPartition->borders);
					}
				}
			}

			Utils::Stream::ClearPointer(&dest->collisionPartitions);
			SaveLogExit();
		}

		if (asset->collisionAABBTrees)
		{
			AssertSize(Game::CollisionAabbTree, 32);
			SaveLogEnter("CollisionAabbTree");

			buffer->align(Utils::Stream::ALIGN_16);
			buffer->saveArray(asset->collisionAABBTrees, asset->numCollisionAABBTrees);
			Utils::Stream::ClearPointer(&dest->collisionAABBTrees);

			SaveLogExit();
		}

		if (asset->cModels)
		{
			AssertSize(Game::cmodel_t, 68);
			SaveLogEnter("cmodel_t");

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->cModels, asset->numCModels);
			Utils::Stream::ClearPointer(&dest->cModels);

			SaveLogExit();
		}

		if (asset->cBrushes)
		{
			AssertSize(Game::cbrush_t, 36);
			SaveLogEnter("cbrush_t");

			buffer->align(Utils::Stream::ALIGN_128);
			Game::cbrush_t* destBrushes = buffer->dest<Game::cbrush_t>();
			buffer->saveArray(asset->cBrushes, asset->numCBrushes);

			for (short i = 0; i < asset->numCBrushes; ++i)
			{
				Game::cbrush_t* destBrush = &destBrushes[i];
				Game::cbrush_t* brush = &asset->cBrushes[i];

				if (brush->sides)
				{
					if (builder->hasPointer(brush->sides))
					{
						destBrush->sides = builder->getPointer(brush->sides);
					}
					else
					{
						AssertSize(Game::cbrushside_t, 8);

						MessageBoxA(0, "BrushSide shouldn't be written in cBrush!", "WARNING", MB_ICONEXCLAMATION);

						buffer->align(Utils::Stream::ALIGN_4);
						builder->storePointer(brush->sides);

						Game::cbrushside_t* side = buffer->dest<Game::cbrushside_t>();
						buffer->save(brush->sides);

						if (brush->sides->side)
						{
							if (builder->hasPointer(brush->sides->side))
							{
								side->side = builder->getPointer(brush->sides->side);
							}
							else
							{
								buffer->align(Utils::Stream::ALIGN_4);
								builder->storePointer(brush->sides->side);
								buffer->save(brush->sides->side);
								Utils::Stream::ClearPointer(&side->side);
							}
						}

						Utils::Stream::ClearPointer(&destBrush->sides);
					}
				}

				if (brush->baseAdjacentSide)
				{
					if (builder->hasPointer(brush->baseAdjacentSide))
					{
						destBrush->baseAdjacentSide = builder->getPointer(brush->baseAdjacentSide);
					}
					else
					{
						builder->storePointer(brush->baseAdjacentSide);
						buffer->save(brush->baseAdjacentSide);
						Utils::Stream::ClearPointer(&destBrush->baseAdjacentSide);
					}
				}
			}

			Utils::Stream::ClearPointer(&dest->cBrushes);
			SaveLogExit();
		}

		if (asset->cBrushBounds)
		{
			AssertSize(Game::Bounds, 24);
			SaveLogEnter("Bounds");

			buffer->align(Utils::Stream::ALIGN_128);
			buffer->saveArray(asset->cBrushBounds, asset->numCBrushes);
			Utils::Stream::ClearPointer(&dest->cBrushBounds);

			SaveLogExit();
		}

		if (asset->cBrushContents)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->cBrushContents, asset->numCBrushes);
			Utils::Stream::ClearPointer(&dest->cBrushContents);
		}

		if (asset->unknown4)
		{
			AssertSize(Game::SModelAabbNode, 28);
			SaveLogEnter("SModelAabbNode");

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->unknown4, asset->unkCount4);
			Utils::Stream::ClearPointer(&dest->unknown4);

			SaveLogExit();
		}

		if (asset->mapEnts)
		{
			dest->mapEnts = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MAP_ENTS, asset->mapEnts).mapEnts;
		}

		for (int i = 0; i < 2; ++i)
		{
			if (asset->dynEntDefList[i])
			{
				AssertSize(Game::DynEntityDef, 92);

				buffer->align(Utils::Stream::ALIGN_4);
				Game::DynEntityDef* dynEntDest = buffer->dest<Game::DynEntityDef>();
				buffer->saveArray(asset->dynEntDefList[i], asset->dynEntCount[i]);

				Game::DynEntityDef* dynEnt = asset->dynEntDefList[i];
				for (int j = 0; j < asset->dynEntCount[i]; ++j)
				{
					if (dynEnt[j].xModel)
					{
						dynEntDest[j].xModel = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_XMODEL, dynEnt[j].xModel).model;
					}

					if (dynEnt[j].destroyFx)
					{
						dynEntDest[j].destroyFx = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_FX, dynEnt[j].destroyFx).fx;
					}

					if (dynEnt[j].physPreset)
					{
						dynEntDest[j].physPreset = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_PHYSPRESET, dynEnt[j].physPreset).physPreset;
					}
				}

				Utils::Stream::ClearPointer(&dest->dynEntDefList[i]);
			}
		}

		buffer->pushBlock(Game::XFILE_BLOCK_RUNTIME);

		for (int i = 0; i < 2; ++i)
		{
			if (asset->dynEntPoseList[i])
			{
				AssertSize(Game::DynEntityPose, 32);

				buffer->align(Utils::Stream::ALIGN_4);
				buffer->saveArray(asset->dynEntPoseList[i], asset->dynEntCount[i]);
				Utils::Stream::ClearPointer(&dest->dynEntPoseList[i]);

			}
		}

		for (int i = 0; i < 2; ++i)
		{
			if (asset->dynEntClientList[i])
			{
				AssertSize(Game::DynEntityClient, 12);

				buffer->align(Utils::Stream::ALIGN_4);
				buffer->saveArray(asset->dynEntClientList[i], asset->dynEntCount[i]);
				Utils::Stream::ClearPointer(&dest->dynEntClientList[i]);
			}
		}

		for (int i = 0; i < 2; ++i)
		{
			if (asset->dynEntCollList[i])
			{
				AssertSize(Game::DynEntityColl, 20);

				buffer->align(Utils::Stream::ALIGN_4);
				buffer->saveArray(asset->dynEntCollList[i], asset->dynEntCount[i]);
				Utils::Stream::ClearPointer(&dest->dynEntCollList[i]);
			}
		}

		buffer->popBlock();
		buffer->popBlock();

		SaveLogExit();
	}

	void IclipMap_t::mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::clipMap_t* asset = header.clipMap;
		for (int i = 0; i < asset->numStaticModels; ++i)
		{
			Game::XModel* m = asset->staticModelList[i].xmodel;
			if (m)
			{
				builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL, m);
			}
		}

		for (int j = 0; j < 2; ++j)
		{
			Game::DynEntityDef* def = asset->dynEntDefList[j];

			for (int i = 0; i < asset->dynEntCount[j]; ++i)
			{
				if (def[i].xModel)
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL, def[i].xModel);
				}

				if (def[i].destroyFx)
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_FX, def[i].destroyFx);
				}

				if (def[i].physPreset)
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_PHYSPRESET, def[i].physPreset);
				}
			}
		}
		builder->loadAsset(Game::XAssetType::ASSET_TYPE_MAP_ENTS, asset);
	}

	void IclipMap_t::load(Game::XAssetHeader* header, std::string name, Components::ZoneBuilder::Zone* builder)
	{
		Utils::String::Replace(name, "maps/mp/", "");
		Utils::String::Replace(name, ".d3dbsp", "");

		Components::FileSystem::File clipFile(fmt::sprintf("clipmap/%s.iw4xClipMap", name.data()));
		if (!clipFile.exists())
		{
			return;
		}

		Game::clipMap_t* clipMap = builder->getAllocator()->allocate<Game::clipMap_t>();
		if (!clipMap)
		{
			Components::Logger::Print("Error allocating memory for clipMap_t structure!\n");
			return;
		}

		Utils::Stream::Reader reader(builder->getAllocator(), clipFile.getBuffer());

		if (reader.read<__int64>() != *reinterpret_cast<__int64*>("IW4xClip"))
		{
			Components::Logger::Error(0, "Reading clipMap_t '%s' failed, header is invalid!", name.data());
		}

		int version = reader.read<int>();
		if (version != IW4X_CLIPMAP_VERSION)
		{
			Components::Logger::Error(0, "Reading clipmap '%s' failed, expected version is %d, but it was %d!", name.data(), IW4X_CLIPMAP_VERSION, version);
		}

		clipMap->name = reader.readCString();

		clipMap->numCPlanes = reader.read<int>();
		clipMap->numStaticModels = reader.read<int>();
		clipMap->numMaterials = reader.read<int>();
		clipMap->numCBrushSides = reader.read<int>();
		clipMap->numCBrushEdges = reader.read<int>();
		clipMap->numCNodes = reader.read<int>();
		clipMap->numCLeaf = reader.read<int>();
		clipMap->numCLeafBrushNodes = reader.read<int>();
		clipMap->numLeafBrushes = reader.read<int>();
		clipMap->numLeafSurfaces = reader.read<int>();
		clipMap->numVerts = reader.read<int>();
		clipMap->numTriIndices = reader.read<int>();
		clipMap->numCollisionBorders = reader.read<int>();
		clipMap->numCollisionPartitions = reader.read<int>();
		clipMap->numCollisionAABBTrees = reader.read<int>();
		clipMap->numCModels = reader.read<int>();
		clipMap->numCBrushes = reader.read<short>();
		clipMap->dynEntCount[0] = reader.read<unsigned __int16>();
		clipMap->dynEntCount[1] = reader.read<unsigned __int16>();

		if (clipMap->numCPlanes)
		{
			clipMap->cPlanes = reader.readArray<Game::cplane_t>(clipMap->numCPlanes);
		}

		if (clipMap->numStaticModels)
		{
			clipMap->staticModelList = builder->getAllocator()->allocateArray<Game::cStaticModel_t>(clipMap->numStaticModels);
			for (int i = 0; i < clipMap->numStaticModels; ++i)
			{
				std::string modelName = reader.readString();
				if (modelName != "NONE"s)
				{
					clipMap->staticModelList[i].xmodel = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_XMODEL, modelName, builder).model;
				}
				float* buf = reader.readArray<float>(18);
				memcpy(&clipMap->staticModelList[i].origin, buf, sizeof(float) * 18);
			}
		}

		if (clipMap->numMaterials)
		{
			clipMap->materials = builder->getAllocator()->allocateArray<Game::ClipMaterial>(clipMap->numMaterials);
			for (int j = 0; j < clipMap->numMaterials; ++j)
			{
				clipMap->materials[j].name = reader.readArray<char>(64);
				clipMap->materials[j].unk = reader.read<int>();
				clipMap->materials[j].unk2 = reader.read<int>();
			}
		}

		if (clipMap->numCBrushSides)
		{
			clipMap->cBrushSides = builder->getAllocator()->allocateArray<Game::cbrushside_t>(clipMap->numCBrushSides);
			for (int i = 0; i < clipMap->numCBrushSides; ++i)
			{
				int planeIndex = reader.read<int>();
				if (planeIndex < 0 || planeIndex > clipMap->numCBrushSides)
				{
					Components::Logger::Error("invalid plane index");
					return;
				}
				clipMap->cBrushSides[i].side = &clipMap->cPlanes[planeIndex];
				reader.read<int>(); // materialNum
				reader.read<short>(); // firstAdjacentSide
				reader.read<char>(); // edgeCount
				// not sure how to fill out texInfo and dispInfo
				// just leave zero for now
			}
		}

		if (clipMap->numCBrushEdges)
		{
			clipMap->cBrushEdges = reader.readArray<char>(clipMap->numCBrushEdges);
		}

		if (clipMap->numCNodes)
		{
			clipMap->cNodes = builder->getAllocator()->allocateArray<Game::cNode_t>(clipMap->numCNodes);
			for (int i = 0; i < clipMap->numCNodes; ++i)
			{
				int planeIndex = reader.read<int>();
				if (planeIndex < 0 || planeIndex > clipMap->numCPlanes)
				{
					Components::Logger::Error("invalid plane index\n");
					return;
				}
				clipMap->cNodes[i].plane = &clipMap->cPlanes[planeIndex];
				clipMap->cNodes[i].children[0] = reader.read<short>();
				clipMap->cNodes[i].children[1] = reader.read<short>();
			}
		}

		if (clipMap->numCLeaf)
		{
			clipMap->cLeaf = reader.readArray<Game::cLeaf_t>(clipMap->numCLeaf);
		}

		if (clipMap->numCLeafBrushNodes)
		{
			clipMap->cLeafBrushNodes = builder->getAllocator()->allocateArray<Game::cLeafBrushNode_t>(clipMap->numCLeafBrushNodes);
			for (int i = 0; i < clipMap->numCLeafBrushNodes; ++i)
			{
				Game::cLeafBrushNode_t tmp = reader.read<Game::cLeafBrushNode_t>();
				memcpy(&clipMap->cLeafBrushNodes[i], &tmp, sizeof(Game::cLeafBrushNode_t));

				if (tmp.leafBrushCount > 0)
				{
					clipMap->cLeafBrushNodes[i].data.brushes = reader.readArray<unsigned short>(tmp.leafBrushCount);
				}
			}
		}

		if (clipMap->numLeafBrushes)
		{
			clipMap->leafBrushes = reader.readArray<short>(clipMap->numLeafBrushes);
		}

		if (clipMap->numLeafSurfaces)
		{
			clipMap->leafSurfaces = reader.readArray<int>(clipMap->numLeafSurfaces);
		}

		if (clipMap->numVerts)
		{
			clipMap->verts = reader.readArray<Game::vec3_t>(clipMap->numVerts);
		}

		if (clipMap->numTriIndices)
		{
			clipMap->triIndices = reader.readArray<short>(clipMap->numTriIndices * 3);
			clipMap->triEdgeIsWalkable = reader.readArray<bool>(4 * ((3 * clipMap->numTriIndices + 31) >> 5));
		}

		if (clipMap->numCollisionBorders)
		{
			clipMap->collisionBorders = reader.readArray<Game::CollisionBorder>(clipMap->numCollisionBorders);
		}

		if (clipMap->numCollisionPartitions)
		{
			clipMap->collisionPartitions = builder->getAllocator()->allocateArray<Game::CollisionPartition>(clipMap->numCollisionPartitions);
			for (int i = 0; i < clipMap->numCollisionPartitions; ++i)
			{
				clipMap->collisionPartitions[i].triCount = reader.read<char>();
				clipMap->collisionPartitions[i].borderCount = reader.read<char>();
				clipMap->collisionPartitions[i].firstTri = reader.read<int>();

				if (clipMap->collisionPartitions[i].borderCount > 0)
				{
					int index = reader.read<int>();
					if (index < 0 || index > clipMap->numCollisionBorders)
					{
						Components::Logger::Error("invalid border index\n");
						return;
					}
					clipMap->collisionPartitions[i].borders = &clipMap->collisionBorders[index];
				}
			}
		}

		if (clipMap->numCollisionAABBTrees)
		{
			clipMap->collisionAABBTrees = reader.readArray<Game::CollisionAabbTree>(clipMap->numCollisionAABBTrees);
		}

		if (clipMap->numCModels)
		{
			clipMap->cModels = reader.readArray<Game::cmodel_t>(clipMap->numCModels);
		}

		if (clipMap->numCBrushes)
		{
			clipMap->cBrushes = builder->getAllocator()->allocateArray<Game::cbrush_t>(clipMap->numCBrushes);
			memset(clipMap->cBrushes, 0, sizeof(Game::cbrush_t) * clipMap->numCBrushes);
			for (int i = 0; i < clipMap->numCBrushes; ++i)
			{
				clipMap->cBrushes[i].numsides = reader.read<unsigned int>() & 0xFFFF; // todo: check for overflow here
				if (clipMap->cBrushes[i].numsides > 0)
				{
					int index = reader.read<int>();
					if (index < 0 || index > clipMap->numCBrushSides)
					{
						Components::Logger::Error("invalid side index\n");
						return;
					}
					clipMap->cBrushes[i].sides = &clipMap->cBrushSides[index];
				}
				else
				{
					clipMap->cBrushes[i].sides = nullptr;
				}

				int index = reader.read<int>();
				if (index > clipMap->numCBrushEdges)
				{
					Components::Logger::Error("invalid edge index\n");
					return;
				}
				clipMap->cBrushes[i].baseAdjacentSide = &clipMap->cBrushEdges[index];

				char* tmp = reader.readArray<char>(12);
				memcpy(&clipMap->cBrushes[i].axialMaterialNum, tmp, 12);

				//todo check for overflow
				for (int r = 0; r < 2; ++r)
				{
					for (int c = 0; c < 3; ++c)
					{
						clipMap->cBrushes[i].firstAdjacentSideOffsets[r][c] = reader.read<short>() & 0xFF;
					}
				}

				tmp = reader.readArray<char>(6);
				memcpy(&clipMap->cBrushes[i].edgeCount, tmp, 6);
			}

			clipMap->cBrushBounds = reader.readArray<Game::Bounds>(clipMap->numCBrushes);
			clipMap->cBrushContents = reader.readArray<int>(clipMap->numCBrushes);
		}

		for (int x = 0; x < 2; ++x)
		{
			if (clipMap->dynEntCount[x])
			{
				clipMap->dynEntDefList[x] = builder->getAllocator()->allocateArray<Game::DynEntityDef>(clipMap->dynEntCount[x]);
				for (int i = 0; i < clipMap->dynEntCount[x]; ++i)
				{
					clipMap->dynEntDefList[x][i].type = reader.read<Game::DynEntityType>();
					clipMap->dynEntDefList[x][i].pose = reader.read<Game::GfxPlacement>();
					std::string tempName = reader.readString();
					if (tempName != "NONE"s)
					{
						clipMap->dynEntDefList[x][i].xModel = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_XMODEL, tempName, builder).model;
					}

					clipMap->dynEntDefList[x][i].brushModel = reader.read<short>();
					clipMap->dynEntDefList[x][i].physicsBrushModel = reader.read<short>();

					tempName = reader.readString();
					if (tempName != "NONE"s)
					{
						clipMap->dynEntDefList[x][i].destroyFx = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_FX, tempName, builder).fx;
					}

					tempName = reader.readString();
					if (tempName != "NONE"s)
					{
						clipMap->dynEntDefList[x][i].physPreset = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_PHYSPRESET, tempName, builder).physPreset;
					}

					clipMap->dynEntDefList[x][i].health = reader.read<int>();
					clipMap->dynEntDefList[x][i].mass = reader.read<Game::PhysMass>();
					clipMap->dynEntDefList[x][i].contents = reader.read<int>();
				}
			}
		}

		clipMap->checksum = reader.read<int>();

		clipMap->mapEnts = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_MAP_ENTS, Utils::String::VA("maps/mp/%s.d3dbsp", name.c_str()), builder).mapEnts;

		// This mustn't be null and has to have at least 1 'valid' entry.
		if (!clipMap->unkCount4 || !clipMap->unknown4)
		{
			clipMap->unkCount4 = 1;
			clipMap->unknown4 = builder->getAllocator()->allocateArray<Game::SModelAabbNode>(1);
		}

		// These mustn't be null, but they don't need to be valid.
		for (int i = 0; i < 2; ++i)
		{
			Utils::Stream::ClearPointer(&clipMap->dynEntPoseList[i]);
			Utils::Stream::ClearPointer(&clipMap->dynEntClientList[i]);
			Utils::Stream::ClearPointer(&clipMap->dynEntCollList[i]);
		}

		header->clipMap = clipMap;
	}
}
