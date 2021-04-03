#include "StdInclude.hpp"

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

		if (asset->planes)
		{
			AssertSize(Game::cplane_s, 20);
			SaveLogEnter("cplane_t");

			if (builder->hasPointer(asset->planes))
			{
				dest->planes = builder->getPointer(asset->planes);
			}
			else
			{
				buffer->align(Utils::Stream::ALIGN_4);

				// not sure if this is needed but both brushside and brushedge need it and it can't hurt
				for (int i = 0; i < asset->planeCount; ++i)
				{
					builder->storePointer(&asset->planes[i]);
					buffer->save(&asset->planes[i]);
				}
				Utils::Stream::ClearPointer(&dest->planes);
			}

			SaveLogExit();
		}

		if (asset->staticModelList)
		{

			AssertSize(Game::cStaticModel_s, 76);
			SaveLogEnter("cStaticModel_t");

			// xmodel is already stored
			buffer->align(Utils::Stream::ALIGN_4);
			Game::cStaticModel_s* destStaticModelList = buffer->dest<Game::cStaticModel_s>();
			buffer->saveArray(asset->staticModelList, asset->numStaticModels);

			for (unsigned int i = 0; i < asset->numStaticModels; ++i)
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

			for (unsigned int i = 0; i < asset->numMaterials; ++i)
			{
				buffer->saveString(asset->materials[i].name);
				Utils::Stream::ClearPointer(&mats[i].name);
			}

			Utils::Stream::ClearPointer(&dest->materials);
			SaveLogExit();
		}

		if (asset->brushsides)
		{
			AssertSize(Game::cbrushside_t, 8);
			SaveLogEnter("cbrushside_t");

			buffer->align(Utils::Stream::ALIGN_4);
			Game::cbrushside_t* sides = buffer->dest<Game::cbrushside_t>();
			// we need the pointer to each of these to be stored so we can't write them all at once
			for (unsigned int i = 0; i < asset->numBrushSides; ++i)
			{
				builder->storePointer(&asset->brushsides[i]); // for reference in cBrush
				buffer->save(&asset->brushsides[i]);
			}

			for (unsigned int i = 0; i < asset->numBrushSides; ++i)
			{
				if (sides[i].plane)
				{
					AssertSize(Game::cplane_s, 20);

					if (builder->hasPointer(sides[i].plane))
					{
						sides[i].plane = builder->getPointer(sides[i].plane);
					}
					else
					{
						buffer->align(Utils::Stream::ALIGN_4);
						builder->storePointer(sides[i].plane);

						buffer->save(sides[i].plane);
						Utils::Stream::ClearPointer(&sides[i].plane);
					}
				}
			}

			Utils::Stream::ClearPointer(&dest->brushsides);
			SaveLogExit();
		}

		if (asset->brushEdges)
		{
			SaveLogEnter("cBrushEdge");

			// no align for char
			for (unsigned int i = 0; i < asset->numBrushEdges; ++i)
			{
				builder->storePointer(&asset->brushEdges[i]); // for reference in cBrush
				buffer->save(&asset->brushEdges[i]);
			}
			Utils::Stream::ClearPointer(&dest->brushEdges);

			SaveLogExit();
		}

		if (asset->nodes)
		{
			AssertSize(Game::cNode_t, 8);
			SaveLogEnter("cNode_t");

			buffer->align(Utils::Stream::ALIGN_4);
			Game::cNode_t* nodes = buffer->dest<Game::cNode_t>();
			buffer->saveArray(asset->nodes, asset->numNodes);

			for (unsigned int i = 0; i < asset->numNodes; ++i)
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

			Utils::Stream::ClearPointer(&dest->nodes);
			SaveLogExit();
		}

		if (asset->leafs)
		{
			AssertSize(Game::cLeaf_t, 40);
			SaveLogEnter("cLeaf_t");

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->leafs, asset->numLeafs);
			Utils::Stream::ClearPointer(&dest->leafs);
			SaveLogExit();
		}

		if (asset->leafbrushes)
		{
			SaveLogEnter("cLeafBrush_t");

			buffer->align(Utils::Stream::ALIGN_2);
			buffer->saveArray(asset->leafbrushes, asset->numLeafBrushes);
			Utils::Stream::ClearPointer(&dest->leafbrushes);

			SaveLogExit();
		}

		if (asset->leafbrushNodes)
		{
			AssertSize(Game::cLeafBrushNode_s, 20);
			SaveLogEnter("cLeafBrushNode_t");

			buffer->align(Utils::Stream::ALIGN_4);
			Game::cLeafBrushNode_s* node = buffer->dest<Game::cLeafBrushNode_s>();
			buffer->saveArray(asset->leafbrushNodes, asset->leafbrushNodesCount);

			for (unsigned int i = 0; i < asset->leafbrushNodesCount; ++i)
			{
				if (node[i].leafBrushCount > 0)
				{
					if (node[i].data.leaf.brushes)
					{
						if (builder->hasPointer(node[i].data.leaf.brushes))
						{
							node[i].data.leaf.brushes = builder->getPointer(node[i].data.leaf.brushes);
						}
						else
						{
							buffer->align(Utils::Stream::ALIGN_2);

							for (short j = 0; j < node[i].leafBrushCount; ++j)
							{
								builder->storePointer(&node[i].data.leaf.brushes[j]);
								buffer->save(&node[i].data.leaf.brushes[j]);
							}

							Utils::Stream::ClearPointer(&node[i].data.leaf.brushes);
						}
					}
				}
			}

			Utils::Stream::ClearPointer(&dest->leafbrushNodes);
			SaveLogExit();
		}

		if (asset->leafsurfaces)
		{
			SaveLogEnter("cLeafSurface_t");

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->leafsurfaces, asset->numLeafSurfaces);
			Utils::Stream::ClearPointer(&dest->leafsurfaces);

			SaveLogExit();
		}

		if (asset->verts)
		{
			AssertSize(Game::vec3_t, 12);

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->verts, asset->vertCount);
			Utils::Stream::ClearPointer(&dest->verts);
		}

		if (asset->triIndices)
		{
			buffer->align(Utils::Stream::ALIGN_2);
			buffer->save(asset->triIndices, 6, asset->triCount);
			Utils::Stream::ClearPointer(&dest->triIndices);
		}

		if (asset->triEdgeIsWalkable)
		{
			// no align for char
			buffer->save(asset->triEdgeIsWalkable, 1, 4 * ((3 * asset->triCount + 31) >> 5));
			Utils::Stream::ClearPointer(&dest->triEdgeIsWalkable);
		}

		if (asset->borders)
		{
			AssertSize(Game::CollisionBorder, 28);
			SaveLogEnter("CollisionBorder");

			buffer->align(Utils::Stream::ALIGN_4);

			for (int i = 0; i < asset->borderCount; ++i)
			{
				builder->storePointer(&asset->borders[i]);
				buffer->save(&asset->borders[i]);
			}

			Utils::Stream::ClearPointer(&dest->borders);
			SaveLogExit();
		}

		if (asset->partitions)
		{
			AssertSize(Game::CollisionPartition, 12);
			SaveLogEnter("CollisionPartition");

			buffer->align(Utils::Stream::ALIGN_4);
			Game::CollisionPartition* destPartitions = buffer->dest<Game::CollisionPartition>();
			buffer->saveArray(asset->partitions, asset->partitionCount);

			for (int i = 0; i < asset->partitionCount; ++i)
			{
				Game::CollisionPartition* destPartition = &destPartitions[i];
				Game::CollisionPartition* partition = &asset->partitions[i];

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

			Utils::Stream::ClearPointer(&dest->partitions);
			SaveLogExit();
		}

		if (asset->aabbTrees)
		{
			AssertSize(Game::CollisionAabbTree, 32);
			SaveLogEnter("CollisionAabbTree");

			buffer->align(Utils::Stream::ALIGN_16);
			buffer->saveArray(asset->aabbTrees, asset->aabbTreeCount);
			Utils::Stream::ClearPointer(&dest->aabbTrees);

			SaveLogExit();
		}

		if (asset->cmodels)
		{
			AssertSize(Game::cmodel_t, 68);
			SaveLogEnter("cmodel_t");

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->cmodels, asset->numSubModels);
			Utils::Stream::ClearPointer(&dest->cmodels);

			SaveLogExit();
		}

		if (asset->brushes)
		{
			AssertSize(Game::cbrush_t, 36);
			SaveLogEnter("cbrush_t");

			buffer->align(Utils::Stream::ALIGN_128);
			Game::cbrush_t* destBrushes = buffer->dest<Game::cbrush_t>();
			buffer->saveArray(asset->brushes, asset->numBrushes);

			for (short i = 0; i < asset->numBrushes; ++i)
			{
				Game::cbrush_t* destBrush = &destBrushes[i];
				Game::cbrush_t* brush = &asset->brushes[i];

				if (brush->sides)
				{
					if (builder->hasPointer(brush->sides))
					{
						destBrush->sides = builder->getPointer(brush->sides);
					}
					else
					{
						AssertSize(Game::cbrushside_t, 8);

						MessageBoxA(nullptr, "BrushSide shouldn't be written in cBrush!", "WARNING", MB_ICONEXCLAMATION);

						buffer->align(Utils::Stream::ALIGN_4);
						builder->storePointer(brush->sides);

						Game::cbrushside_t* side = buffer->dest<Game::cbrushside_t>();
						buffer->save(brush->sides);

						if (brush->sides->plane)
						{
							if (builder->hasPointer(brush->sides->plane))
							{
								side->plane = builder->getPointer(brush->sides->plane);
							}
							else
							{
								buffer->align(Utils::Stream::ALIGN_4);
								builder->storePointer(brush->sides->plane);
								buffer->save(brush->sides->plane);
								Utils::Stream::ClearPointer(&side->plane);
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

			Utils::Stream::ClearPointer(&dest->brushes);
			SaveLogExit();
		}

		if (asset->brushBounds)
		{
			AssertSize(Game::Bounds, 24);
			SaveLogEnter("Bounds");

			buffer->align(Utils::Stream::ALIGN_128);
			buffer->saveArray(asset->brushBounds, asset->numBrushes);
			Utils::Stream::ClearPointer(&dest->brushBounds);

			SaveLogExit();
		}

		if (asset->brushContents)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->brushContents, asset->numBrushes);
			Utils::Stream::ClearPointer(&dest->brushContents);
		}

		if (asset->smodelNodes)
		{
			AssertSize(Game::SModelAabbNode, 28);
			SaveLogEnter("SModelAabbNode");

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->smodelNodes, asset->smodelNodeCount);
			Utils::Stream::ClearPointer(&dest->smodelNodes);

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
		for (unsigned int i = 0; i < asset->numStaticModels; ++i)
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

	void IclipMap_t::load(Game::XAssetHeader* header, const std::string& _name, Components::ZoneBuilder::Zone* builder)
	{
		std::string name = _name;
		Utils::String::Replace(name, "maps/mp/", "");
		Utils::String::Replace(name, ".d3dbsp", "");

		Components::FileSystem::File clipFile(Utils::String::VA("clipmap/%s.iw4xClipMap", name.data()));
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

		Game::clipMap_t* orgClipMap = nullptr;
		Game::DB_EnumXAssets(Game::XAssetType::ASSET_TYPE_CLIPMAP_MP, [](Game::XAssetHeader header, void* clipMap)
		{
			if (!*reinterpret_cast<void**>(clipMap))
			{
				*reinterpret_cast<Game::clipMap_t**>(clipMap) = header.clipMap;
			}
		}, &orgClipMap, false);

		if (orgClipMap) std::memcpy(clipMap, orgClipMap, sizeof Game::clipMap_t);

		Utils::Stream::Reader reader(builder->getAllocator(), clipFile.getBuffer());

		__int64 magic = reader.read<__int64>();
		if (std::memcmp(&magic, "IW4xClip", 8))
		{
			Components::Logger::Error(0, "Reading clipMap_t '%s' failed, header is invalid!", name.data());
		}

		int version = reader.read<int>();
		if (version != IW4X_CLIPMAP_VERSION)
		{
			Components::Logger::Error(0, "Reading clipmap '%s' failed, expected version is %d, but it was %d!", name.data(), IW4X_CLIPMAP_VERSION, version);
		}

		clipMap->name = reader.readCString();

		clipMap->planeCount = reader.read<int>();
		clipMap->numStaticModels = reader.read<int>();
		clipMap->numMaterials = reader.read<int>();
		clipMap->numBrushSides = reader.read<int>();
		clipMap->numBrushEdges = reader.read<int>();
		clipMap->numNodes = reader.read<int>();
		clipMap->numLeafs = reader.read<int>();
		clipMap->leafbrushNodesCount = reader.read<int>();
		clipMap->numLeafBrushes = reader.read<int>();
		clipMap->numLeafSurfaces = reader.read<int>();
		clipMap->vertCount = reader.read<int>();
		clipMap->triCount = reader.read<int>();
		clipMap->borderCount = reader.read<int>();
		clipMap->partitionCount = reader.read<int>();
		clipMap->aabbTreeCount = reader.read<int>();
		clipMap->numSubModels = reader.read<int>();
		clipMap->numBrushes = reader.read<short>();
		clipMap->dynEntCount[0] = reader.read<unsigned __int16>();
		clipMap->dynEntCount[1] = reader.read<unsigned __int16>();

		if (clipMap->planeCount)
		{
			void* oldPtr = reader.read<void*>();
			clipMap->planes = reader.readArray<Game::cplane_s>(clipMap->planeCount);

			if (builder->getAllocator()->isPointerMapped(oldPtr))
			{
				clipMap->planes = builder->getAllocator()->getPointer<Game::cplane_s>(oldPtr);
				Components::Logger::Print("ClipMap dpvs planes already mapped. This shouldn't happen. Make sure to load the ClipMap before the GfxWorld!\n");
			}
			else
			{
				builder->getAllocator()->mapPointer(oldPtr, clipMap->planes);
			}
		}

		if (clipMap->numStaticModels)
		{
			clipMap->staticModelList = builder->getAllocator()->allocateArray<Game::cStaticModel_s>(clipMap->numStaticModels);
			for (unsigned int i = 0; i < clipMap->numStaticModels; ++i)
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
			for (unsigned int j = 0; j < clipMap->numMaterials; ++j)
			{
				clipMap->materials[j].name = reader.readArray<char>(64);
				clipMap->materials[j].surfaceFlags = reader.read<int>();
				clipMap->materials[j].contents = reader.read<int>();
			}
		}

		if (clipMap->numBrushSides)
		{
			clipMap->brushsides = builder->getAllocator()->allocateArray<Game::cbrushside_t>(clipMap->numBrushSides);
			for (unsigned int i = 0; i < clipMap->numBrushSides; ++i)
			{
				int planeIndex = reader.read<int>();
				if (planeIndex < 0 || planeIndex >= clipMap->planeCount)
				{
					Components::Logger::Error("invalid plane index");
					return;
				}
				clipMap->brushsides[i].plane = &clipMap->planes[planeIndex];
				clipMap->brushsides[i].materialNum = static_cast<unsigned short>(reader.read<int>()); // materialNum
				clipMap->brushsides[i].firstAdjacentSideOffset = static_cast<char>(reader.read<short>()); // firstAdjacentSide
				clipMap->brushsides[i].edgeCount = reader.read<char>(); // edgeCount
			}
		}

		if (clipMap->numBrushEdges)
		{
			clipMap->brushEdges = reader.readArray<char>(clipMap->numBrushEdges);
		}

		if (clipMap->numNodes)
		{
			clipMap->nodes = builder->getAllocator()->allocateArray<Game::cNode_t>(clipMap->numNodes);
			for (unsigned int i = 0; i < clipMap->numNodes; ++i)
			{
				int planeIndex = reader.read<int>();
				if (planeIndex < 0 || planeIndex >= clipMap->planeCount)
				{
					Components::Logger::Error("invalid plane index\n");
					return;
				}
				clipMap->nodes[i].plane = &clipMap->planes[planeIndex];
				clipMap->nodes[i].children[0] = reader.read<short>();
				clipMap->nodes[i].children[1] = reader.read<short>();
			}
		}

		if (clipMap->numLeafs)
		{
			clipMap->leafs = reader.readArray<Game::cLeaf_t>(clipMap->numLeafs);
		}

		if (clipMap->leafbrushNodesCount)
		{
			clipMap->leafbrushNodes = builder->getAllocator()->allocateArray<Game::cLeafBrushNode_s>(clipMap->leafbrushNodesCount);
			for (unsigned int i = 0; i < clipMap->leafbrushNodesCount; ++i)
			{
				clipMap->leafbrushNodes[i] = reader.read<Game::cLeafBrushNode_s>();

				if (clipMap->leafbrushNodes[i].leafBrushCount > 0)
				{
					clipMap->leafbrushNodes[i].data.leaf.brushes = reader.readArray<unsigned short>(clipMap->leafbrushNodes[i].leafBrushCount);
				}
			}
		}

		if (clipMap->numLeafBrushes)
		{
			clipMap->leafbrushes = reader.readArray<unsigned short>(clipMap->numLeafBrushes);
		}

		if (clipMap->numLeafSurfaces)
		{
			clipMap->leafsurfaces = reader.readArray<unsigned int>(clipMap->numLeafSurfaces);
		}

		if (clipMap->vertCount)
		{
			clipMap->verts = reader.readArray<Game::vec3_t>(clipMap->vertCount);
		}

		if (clipMap->triCount)
		{
			clipMap->triIndices = reader.readArray<unsigned short>(clipMap->triCount * 3);
			clipMap->triEdgeIsWalkable = reader.readArray<char>(4 * ((3 * clipMap->triCount + 31) >> 5));
		}

		if (clipMap->borderCount)
		{
			clipMap->borders = reader.readArray<Game::CollisionBorder>(clipMap->borderCount);
		}

		if (clipMap->partitionCount)
		{
			clipMap->partitions = builder->getAllocator()->allocateArray<Game::CollisionPartition>(clipMap->partitionCount);
			for (int i = 0; i < clipMap->partitionCount; ++i)
			{
				clipMap->partitions[i].triCount = reader.read<char>();
				clipMap->partitions[i].borderCount = reader.read<char>();
				clipMap->partitions[i].firstTri = reader.read<int>();

				if (clipMap->partitions[i].borderCount > 0)
				{
					int index = reader.read<int>();
					if (index < 0 || index > clipMap->borderCount)
					{
						Components::Logger::Error("invalid border index\n");
						return;
					}
					clipMap->partitions[i].borders = &clipMap->borders[index];
				}
			}
		}

		if (clipMap->aabbTreeCount)
		{
			clipMap->aabbTrees = reader.readArray<Game::CollisionAabbTree>(clipMap->aabbTreeCount);
		}

		if (clipMap->numSubModels)
		{
			clipMap->cmodels = reader.readArray<Game::cmodel_t>(clipMap->numSubModels);
		}

		if (clipMap->numBrushes)
		{
			clipMap->brushes = builder->getAllocator()->allocateArray<Game::cbrush_t>(clipMap->numBrushes);
			memset(clipMap->brushes, 0, sizeof(Game::cbrush_t) * clipMap->numBrushes);
			for (int i = 0; i < clipMap->numBrushes; ++i)
			{
				clipMap->brushes[i].numsides = reader.read<unsigned int>() & 0xFFFF; // todo: check for overflow here
				if (clipMap->brushes[i].numsides > 0)
				{
					unsigned int index = reader.read<unsigned int>();
					if (index < 0 || index > clipMap->numBrushSides)
					{
						Components::Logger::Error("invalid side index\n");
						return;
					}
					clipMap->brushes[i].sides = &clipMap->brushsides[index];
				}
				else
				{
					clipMap->brushes[i].sides = nullptr;
				}

				unsigned int index = reader.read<unsigned int>();
				if (index > clipMap->numBrushEdges)
				{
					Components::Logger::Error("invalid edge index\n");
					return;
				}
				clipMap->brushes[i].baseAdjacentSide = &clipMap->brushEdges[index];

				char* tmp = reader.readArray<char>(12);
				memcpy(&clipMap->brushes[i].axialMaterialNum, tmp, 12);

				//todo check for overflow
				for (int r = 0; r < 2; ++r)
				{
					for (int c = 0; c < 3; ++c)
					{
						clipMap->brushes[i].firstAdjacentSideOffsets[r][c] = reader.read<short>() & 0xFF;
					}
				}

				tmp = reader.readArray<char>(6);
				memcpy(&clipMap->brushes[i].edgeCount, tmp, 6);
			}

			clipMap->brushBounds = reader.readArray<Game::Bounds>(clipMap->numBrushes);
			clipMap->brushContents = reader.readArray<int>(clipMap->numBrushes);
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

		clipMap->smodelNodeCount = reader.read<unsigned short>();
		clipMap->smodelNodes = reader.readArray<Game::SModelAabbNode>(clipMap->smodelNodeCount);

		clipMap->checksum = reader.read<int>();

		clipMap->mapEnts = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_MAP_ENTS, Utils::String::VA("maps/mp/%s.d3dbsp", name.data()), builder).mapEnts;

		// add triggers to mapEnts

		std::vector<Game::TriggerSlab> slabs;
		std::vector<Game::TriggerHull> hulls;
		std::vector<Game::TriggerModel> models;

		clipMap->mapEnts->trigger.count = clipMap->numSubModels;
		clipMap->mapEnts->trigger.hullCount = clipMap->numSubModels;
		clipMap->mapEnts->trigger.slabCount = 0;

		for (int i = 0; i < clipMap->numSubModels; ++i)
		{
			Game::TriggerHull trigHull = {};
			trigHull.bounds = clipMap->cmodels[i].bounds;
			trigHull.contents = clipMap->cmodels[i].leaf.brushContents | clipMap->cmodels[i].leaf.terrainContents;;

			Game::TriggerModel trigMod = {};
			trigMod.hullCount = 1;
			trigMod.firstHull = 0;
			trigMod.contents = clipMap->cmodels[i].leaf.brushContents | clipMap->cmodels[i].leaf.terrainContents;;

			auto* node = &clipMap->leafbrushNodes[clipMap->cmodels[i].leaf.leafBrushNode];

			if (!node->leafBrushCount) continue; // skip empty brushes

			int baseHull = hulls.size();
			for (int j = 0; j < node->leafBrushCount; ++j)
			{
				auto* brush = &clipMap->brushes[node->data.leaf.brushes[j]];

				auto baseSlab = slabs.size();
				for (int k = 0; k < brush->numsides; ++k)
				{
					Game::TriggerSlab curSlab;
					curSlab.dir[0] = brush->sides[k].plane->normal[0];
					curSlab.dir[1] = brush->sides[k].plane->normal[1];
					curSlab.dir[2] = brush->sides[k].plane->normal[2];
					curSlab.halfSize = brush->sides[k].plane->dist;
					curSlab.midPoint = 0.0f; // ??

					slabs.emplace_back(curSlab);
					clipMap->mapEnts->trigger.slabCount++;
				}

				trigHull.firstSlab = baseSlab;
				trigHull.slabCount = slabs.size() - baseSlab;
			}

			models.emplace_back(trigMod);
			hulls.emplace_back(trigHull);
		}

		clipMap->mapEnts->trigger.models = &models[0];
		clipMap->mapEnts->trigger.hulls = &hulls[0];
		clipMap->mapEnts->trigger.slabs = &slabs[0];


		// This mustn't be null and has to have at least 1 'valid' entry.
		if (!clipMap->smodelNodeCount || !clipMap->smodelNodes)
		{
			clipMap->smodelNodeCount = 1;
			clipMap->smodelNodes = builder->getAllocator()->allocateArray<Game::SModelAabbNode>(clipMap->smodelNodeCount);

			clipMap->smodelNodes[0].bounds.halfSize[0] = -131072.000f;
			clipMap->smodelNodes[0].bounds.halfSize[1] = -131072.000f;
			clipMap->smodelNodes[0].bounds.halfSize[2] = -131072.000f;
		}

		// These mustn't be null, but they don't need to be valid.
		for (int i = 0; i < 2 && clipMap->dynEntCount[i]; ++i)
		{
			Utils::Stream::ClearPointer(&clipMap->dynEntPoseList[i]);
			Utils::Stream::ClearPointer(&clipMap->dynEntClientList[i]);
			Utils::Stream::ClearPointer(&clipMap->dynEntCollList[i]);
		}

		if (!reader.end())
		{
			Components::Logger::Error("Clipmap data left!");
		}

		header->clipMap = clipMap;
	}
}
