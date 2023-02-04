#include <STDInclude.hpp>
#include "IclipMap_t.hpp"
#include "Utils/Json.hpp"

#define IW4X_CLIPMAP_VERSION 3

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
				for (size_t i = 0; i < asset->planeCount; ++i)
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

			for (size_t i = 0; i < asset->numLeafBrushes; i++)
			{
				builder->storePointer(&asset->leafbrushes[i]);
				buffer->saveObject(asset->leafbrushes[i]);
			}

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
						assert(node[i].data.leaf.brushes >= asset->leafbrushes);
						assert(node[i].data.leaf.brushes < asset->leafbrushes + sizeof(unsigned short) * asset->numLeafBrushes);

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

			for (size_t i = 0; i < asset->borderCount; ++i)
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
		if (!header->data) loadFromJSON(header, _name, builder);
		if (!header->data) loadBinary(header, _name, builder);
		assert(header->data);
	}

	void IclipMap_t::loadBinary(Game::XAssetHeader* header, const std::string& _name, Components::ZoneBuilder::Zone* builder)
	{
		std::string name = _name;
		Utils::String::Replace(name, "maps/mp/", "");
		Utils::String::Replace(name, ".d3dbsp", "");

		Components::FileSystem::File clipFile(std::format("clipmap/{}.iw4xClipMap", name));
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
			Components::Logger::Error(Game::ERR_FATAL, "Reading clipMap_t '{}' failed, header is invalid!", name);
		}

		int version = reader.read<int>();
		if (version > IW4X_CLIPMAP_VERSION)
		{
			Components::Logger::Error(Game::ERR_FATAL, "Reading clipmap '{}' failed, expected version is {}, but it was {}!", name, IW4X_CLIPMAP_VERSION, version);
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
				auto planeIndex = reader.read<unsigned int>();
				if (planeIndex < 0 || planeIndex >= clipMap->planeCount)
				{
					Components::Logger::Error(Game::ERR_FATAL, "invalid plane index");
					return;
				}
				clipMap->brushsides[i].plane = &clipMap->planes[planeIndex];
				clipMap->brushsides[i].materialNum = static_cast<unsigned short>(reader.read<int>()); // materialNum

				assert(clipMap->brushsides[i].materialNum < clipMap->numMaterials);

				clipMap->brushsides[i].firstAdjacentSideOffset = static_cast<char>(reader.read<short>()); // firstAdjacentSide
				clipMap->brushsides[i].edgeCount = reader.read<unsigned char>(); // edgeCount
			}
		}

		if (clipMap->numBrushEdges)
		{
			clipMap->brushEdges = reader.readArray<unsigned char>(clipMap->numBrushEdges);
		}

		if (clipMap->numNodes)
		{
			clipMap->nodes = builder->getAllocator()->allocateArray<Game::cNode_t>(clipMap->numNodes);
			for (unsigned int i = 0; i < clipMap->numNodes; ++i)
			{
				auto planeIndex = reader.read<unsigned int>();
				if (planeIndex < 0 || planeIndex >= clipMap->planeCount)
				{
					Components::Logger::Error(Game::ERR_FATAL, "invalid plane index\n");
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
					clipMap->leafbrushNodes[i].data.leaf.brushes = reader.readArrayOnce<unsigned short>(clipMap->leafbrushNodes[i].leafBrushCount);
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
			clipMap->triEdgeIsWalkable = reader.readArray<unsigned char>(4 * ((3 * clipMap->triCount + 31) >> 5));

			// check
			for (size_t i = 0; i < clipMap->triCount; i++)
			{
				for (size_t x = 0; x < 3; x++)
				{
					auto vIndex = clipMap->triIndices[i * 3 + x];
					assert(clipMap->vertCount > vIndex);
					assert(vIndex >= 0);
					assert(vIndex != clipMap->triIndices[i * 3 + ((x + 1) % 3)]);
					assert(vIndex != clipMap->triIndices[i * 3 + ((x + 2) % 3)]);
					(void)vIndex;
				}
			}
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
				clipMap->partitions[i].triCount = reader.read<unsigned char>();
				clipMap->partitions[i].borderCount = reader.read<unsigned char>();
				clipMap->partitions[i].firstTri = reader.read<int>();

				if (clipMap->partitions[i].borderCount > 0)
				{
					auto index = reader.read<unsigned int>();
					if (index < 0 || index > clipMap->borderCount)
					{
						Components::Logger::Error(Game::ERR_FATAL, "invalid border index\n");
						return;
					}
					clipMap->partitions[i].borders = &clipMap->borders[index];
				}

				for (size_t j = 0; j < clipMap->partitions[i].triCount; j++)
				{
					assert(clipMap->partitions[i].firstTri + j >= 0);
					assert(clipMap->partitions[i].firstTri + j < clipMap->triCount);
				}
			}
		}

		if (clipMap->aabbTreeCount)
		{
			clipMap->aabbTrees = reader.readArray<Game::CollisionAabbTree>(clipMap->aabbTreeCount);

			// Check
			for (size_t i = 0; i < clipMap->aabbTreeCount; i++)
			{
				assert(clipMap->aabbTrees->materialIndex >= 0);
				assert(clipMap->aabbTrees->materialIndex < clipMap->numMaterials);
				assert(clipMap->materials[clipMap->aabbTrees->materialIndex].contents);
			}

		}

		if (clipMap->numSubModels)
		{
			clipMap->cmodels = reader.readArray<Game::cmodel_t>(clipMap->numSubModels);

			// Check
			for (size_t i = 0; i < clipMap->numSubModels; i++)
			{
				auto cmodel = clipMap->cmodels[i];

				for (size_t j = 0; j < cmodel.leaf.collAabbCount; j++)
				{
					auto index = cmodel.leaf.firstCollAabbIndex + j;
					assert(index >= 0);
					assert(index < clipMap->aabbTreeCount);
					assert(cmodel.leaf.brushContents);
					(void)index;
				}
			}
		}

		if (clipMap->numBrushes)
		{
			clipMap->brushes = builder->getAllocator()->allocateArray<Game::cbrush_t>(clipMap->numBrushes);
			memset(clipMap->brushes, 0, sizeof(Game::cbrush_t) * clipMap->numBrushes);
			for (int i = 0; i < clipMap->numBrushes; ++i)
			{
				clipMap->brushes[i].numsides = reader.read<unsigned int>() & 0xFFFF; // todo: check for overflow here

				if (version >= 3)
				{
					clipMap->brushes[i].glassPieceIndex = reader.read<unsigned short>();
				}

				if (clipMap->brushes[i].numsides > 0)
				{
					auto index = reader.read<unsigned int>();
					if (index < 0 || index > clipMap->numBrushSides)
					{
						Components::Logger::Error(Game::ERR_FATAL, "invalid side index\n");
						return;
					}
					clipMap->brushes[i].sides = &clipMap->brushsides[index];
				}
				else
				{
					clipMap->brushes[i].sides = nullptr;
				}

				auto index = reader.read<unsigned int>();

				if (index == -1)
				{
					clipMap->brushes[i].baseAdjacentSide = nullptr; // Happens
				}
				else
				{
					if (index > clipMap->numBrushEdges)
					{
						Components::Logger::Error(Game::ERR_FATAL, "invalid edge index\n");
						return;
					}

					clipMap->brushes[i].baseAdjacentSide = &clipMap->brushEdges[index];
					assert(*clipMap->brushes[i].baseAdjacentSide >= 0);
					assert(*clipMap->brushes[i].baseAdjacentSide < clipMap->numBrushSides);
				}

				for (size_t x = 0; x < 2; x++)
				{
					for (size_t y = 0; y < 3; y++)
					{
						auto material = reader.read<unsigned short>();

						assert(material >= 0);
						assert(material < clipMap->numMaterials);
						assert(clipMap->materials[material].contents);

						clipMap->brushes[i].axialMaterialNum[x][y] = material;
					}
				}

				//todo check for overflow
				for (int r = 0; r < 2; ++r)
				{
					for (int c = 0; c < 3; ++c)
					{
						clipMap->brushes[i].firstAdjacentSideOffsets[r][c] = reader.read<short>() & 0xFF;
					}
				}

				for (int r = 0; r < 2; ++r)
				{
					for (int c = 0; c < 3; ++c)
					{
						clipMap->brushes[i].edgeCount[r][c] = reader.read<unsigned char>();
					}
				}
			}

			clipMap->brushBounds = reader.readArray<Game::Bounds>(clipMap->numBrushes);
			clipMap->brushContents = reader.readArray<int>(clipMap->numBrushes);

			for (size_t i = 0; i < clipMap->numBrushes; i++)
			{
				assert(clipMap->brushContents[i]);
			}

			assert(clipMap->brushContents);
		}

		for (int x = 0; x < 2; ++x)
		{
			if (clipMap->dynEntCount[x])
			{
				clipMap->dynEntDefList[x] = builder->getAllocator()->allocateArray<Game::DynEntityDef>(clipMap->dynEntCount[x]);
				for (int i = 0; i < clipMap->dynEntCount[x]; ++i)
				{
					clipMap->dynEntDefList[x][i].type = reader.read<Game::DynEntityType>();
					assert(clipMap->dynEntDefList[x][i].type != Game::DynEntityType::DYNENT_TYPE_INVALID);

					clipMap->dynEntDefList[x][i].pose = reader.read<Game::GfxPlacement>();
					std::string tempName = reader.readString();
					if (tempName != "NONE"s)
					{
						auto xModel = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_XMODEL, tempName, builder).model;
						clipMap->dynEntDefList[x][i].xModel = xModel;
						assert(xModel);
					}

					clipMap->dynEntDefList[x][i].brushModel = reader.read<short>();
					clipMap->dynEntDefList[x][i].physicsBrushModel = reader.read<short>();

					tempName = reader.readString();
					if (tempName != "NONE"s)
					{
						auto fx = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_FX, tempName, builder).fx;
						clipMap->dynEntDefList[x][i].destroyFx = fx;
						assert(fx);
					}

					tempName = reader.readString();
					if (tempName != "NONE"s)
					{
						auto preset = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_PHYSPRESET, tempName, builder).physPreset;
						clipMap->dynEntDefList[x][i].physPreset = preset;
						assert(preset);
					}

					clipMap->dynEntDefList[x][i].health = reader.read<int>();
					clipMap->dynEntDefList[x][i].mass = reader.read<Game::PhysMass>();
					clipMap->dynEntDefList[x][i].contents = reader.read<int>();
				}
			}
		}

		clipMap->smodelNodeCount = reader.read<unsigned short>();
		clipMap->smodelNodes = reader.readArray<Game::SModelAabbNode>(clipMap->smodelNodeCount);

		clipMap->mapEnts = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_MAP_ENTS, std::format("maps/mp/{}.d3dbsp", name), builder).mapEnts;

		// add triggers to mapEnts
		if (version >= 3)
		{
			auto trigger = &clipMap->mapEnts->trigger;
			trigger->count = reader.read<unsigned int>();
			trigger->models = builder->getAllocator()->allocateArray<Game::TriggerModel>(trigger->count);
			for (size_t i = 0; i < trigger->count; i++)
			{
				trigger->models[i] = reader.read<Game::TriggerModel>();
			}

			trigger->hullCount = reader.read<unsigned int>();
			trigger->hulls = builder->getAllocator()->allocateArray<Game::TriggerHull>(trigger->hullCount);
			for (size_t i = 0; i < trigger->hullCount; i++)
			{
				trigger->hulls[i] = reader.read<Game::TriggerHull>();
			}

			trigger->slabCount = reader.read<unsigned int>();
			trigger->slabs = builder->getAllocator()->allocateArray<Game::TriggerSlab>(trigger->slabCount);
			for (size_t i = 0; i < trigger->slabCount; i++)
			{
				trigger->slabs[i] = reader.read<Game::TriggerSlab>();
			}

			// Check
#if DEBUG
			for (size_t i = 0; i < trigger->count; i++)
			{
				assert(trigger->models[i].firstHull < trigger->hullCount);
			}

			for (size_t i = 0; i < trigger->hullCount; i++)
			{
				assert(trigger->hulls[i].firstSlab < trigger->slabCount);
			}
#endif
		}
		// This code is wrong but we keep it for backwards compatability with old versions of iw3xport
		else if (version == 2)
		{
			if (clipMap->numSubModels > 0)
			{
				clipMap->mapEnts->trigger.count = clipMap->numSubModels;
				clipMap->mapEnts->trigger.hullCount = clipMap->numSubModels;

				auto* hulls = builder->getAllocator()->allocateArray<Game::TriggerHull>(clipMap->mapEnts->trigger.hullCount);
				auto* models = builder->getAllocator()->allocateArray<Game::TriggerModel>(clipMap->mapEnts->trigger.count);

				for (unsigned int i = 0; i < clipMap->numSubModels; ++i)
				{
					models[i] = reader.read<Game::TriggerModel>();
					hulls[i] = reader.read<Game::TriggerHull>();
				}

				auto slabCount = reader.read<size_t>();
				clipMap->mapEnts->trigger.slabCount = slabCount;
				auto* slabs = builder->getAllocator()->allocateArray<Game::TriggerSlab>(clipMap->mapEnts->trigger.slabCount);
				for (unsigned int i = 0; i < clipMap->mapEnts->trigger.slabCount; i++)
				{
					slabs[i] = reader.read<Game::TriggerSlab>();
				}

				clipMap->mapEnts->trigger.models = &models[0];
				clipMap->mapEnts->trigger.hulls = &hulls[0];
				clipMap->mapEnts->trigger.slabs = &slabs[0];
			}
		}

		clipMap->checksum = reader.read<int>();

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
			Components::Logger::Error(Game::ERR_FATAL, "Clipmap data left!");
		}

		header->clipMap = clipMap;
	}


	void IclipMap_t::loadFromJSON(Game::XAssetHeader* header, const std::string& _name, Components::ZoneBuilder::Zone* builder)
	{
		std::string name = _name;
		Utils::String::Replace(name, "maps/mp/", "");
		Utils::String::Replace(name, ".d3dbsp", "");

		Components::FileSystem::File clipMapFile(std::format("clipmap/{}.iw4x.json", name));

		if (!clipMapFile.exists()) return;


		nlohmann::json clipMapJson;
		try
		{
			clipMapJson = nlohmann::json::parse(clipMapFile.getBuffer());
		}
		catch (const std::exception& e)
		{
			Components::Logger::PrintError(Game::CON_CHANNEL_ERROR, "Invalid JSON for clipmap {}! {}", name, e.what());
			return;
		}

		auto clipMap = builder->getAllocator()->allocate<Game::clipMap_t>();

		try
		{
			assert(clipMapJson["version"].get<int>() <= IW4X_CLIPMAP_VERSION);

			clipMap->name = builder->getAllocator()->duplicateString(clipMapJson["name"].get<std::string>());
			clipMap->isInUse = clipMapJson["isInUse"];

			// planes
			clipMap->planeCount = clipMapJson["planes"].size();
			clipMap->planes = clipMap->planeCount == 0 ? nullptr : builder->getAllocator()->allocateArray<Game::cplane_s>(clipMap->planeCount);

			for (size_t i = 0; i < clipMap->planeCount; i++)
			{
				auto plane = &clipMap->planes[i];
				Utils::Json::CopyArray(plane->normal, clipMapJson["planes"][i]["normal"], 3);
				plane->dist = clipMapJson["planes"][i]["dist"];
				plane->type = clipMapJson["planes"][i]["type"].get<unsigned char>();
			}

			// Smodel list
			clipMap->numStaticModels = clipMapJson["staticModelList"].size();
			clipMap->staticModelList = clipMap->numStaticModels == 0 ? nullptr : builder->getAllocator()->allocateArray<Game::cStaticModel_s>(clipMap->numStaticModels);

			for (size_t i = 0; i < clipMap->numStaticModels; i++)
			{
				auto model = &clipMap->staticModelList[i];
				auto jsonModel = clipMapJson["staticModelList"][i];

				model->xmodel = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_XMODEL, jsonModel["xmodel"], builder).model;

				Utils::Json::CopyArray(model->origin, jsonModel["origin"], 3);
				for (size_t j = 0; j < 3; j++)
				{
					Utils::Json::CopyArray(model->invScaledAxis[j], jsonModel["invScaledAxis"][j], 3);
				}

				model->absBounds = Utils::Json::ReadBounds(jsonModel["absBounds"]);
			}

			clipMap->numMaterials = clipMapJson["materials"].size();
			clipMap->materials = clipMap->numMaterials == 0 ? nullptr : builder->getAllocator()->allocateArray<Game::ClipMaterial>(clipMap->numMaterials);

			for (size_t i = 0; i < clipMap->numMaterials; i++)
			{
				auto material = &clipMap->materials[i];
				auto json_material = clipMapJson["materials"][i];

				material->name = builder->getAllocator()->duplicateString(json_material["name"].get<std::string>());
				material->surfaceFlags = json_material["surfaceFlags"];
				material->contents = json_material["contents"];
			}


			clipMap->numBrushSides = clipMapJson["brushsides"].size();
			clipMap->brushsides = clipMap->numBrushSides == 0 ? nullptr : builder->getAllocator()->allocateArray<Game::cbrushside_t>(clipMap->numBrushSides);

			for (size_t i = 0; i < clipMap->numBrushSides; i++)
			{
				auto* brushside = &clipMap->brushsides[i];
				auto json_brushside = clipMapJson["brushsides"][i];

				assert(clipMap->planes);
				brushside->plane = &clipMap->planes[std::stoi(json_brushside["plane"].get<std::string>().substr(1))];
				brushside->materialNum = json_brushside["materialNum"];
				brushside->firstAdjacentSideOffset = json_brushside["firstAdjacentSideOffset"].get<char>();
				brushside->edgeCount = json_brushside["edgeCount"].get<char>();
			}

			clipMap->numBrushEdges = clipMapJson["brushEdges"].size();
			clipMap->brushEdges = clipMap->numBrushEdges == 0 ? nullptr : builder->getAllocator()->allocateArray<unsigned char>(clipMap->numBrushEdges);
			Utils::Json::CopyArray(clipMap->brushEdges, clipMapJson["brushEdges"]);

			clipMap->numNodes = clipMapJson["nodes"].size();
			clipMap->nodes = clipMap->numNodes == 0 ? nullptr : builder->getAllocator()->allocateArray<Game::cNode_t>(clipMap->numNodes);

			for (size_t i = 0; i < clipMap->numNodes; i++)
			{
				auto node = &clipMap->nodes[i];
				auto jsonNode = clipMapJson["nodes"][i];

				assert(clipMap->planes);
				node->plane = &clipMap->planes[std::stoi(jsonNode["plane"].get<std::string>().substr(1))];
				node->children[0] = jsonNode["children"][0];
				node->children[1] = jsonNode["children"][1];
			}


			// LEaves
			clipMap->numLeafs = clipMapJson["leafs"].size();
			clipMap->leafs = clipMap->numLeafs == 0 ? nullptr : builder->getAllocator()->allocateArray<Game::cLeaf_t>(clipMap->numLeafs);

			for (size_t i = 0; i < clipMap->numLeafs; i++)
			{
				auto leaf = &clipMap->leafs[i];
				auto jsonLeaf = clipMapJson["leafs"][i];

				leaf->bounds = Utils::Json::ReadBounds(jsonLeaf["bounds"]);
				leaf->firstCollAabbIndex = jsonLeaf["firstCollAabbIndex"];
				leaf->collAabbCount = jsonLeaf["collAabbCount"];
				leaf->brushContents = jsonLeaf["brushContents"];
				leaf->terrainContents = jsonLeaf["terrainContents"];
				leaf->leafBrushNode = jsonLeaf["leafBrushNode"];
			}

			// Leafbrushnodes
			clipMap->leafbrushNodesCount = clipMapJson["leafbrushNodes"].size();
			clipMap->leafbrushNodes = clipMap->leafbrushNodesCount == 0 ? nullptr : builder->getAllocator()->allocateArray<Game::cLeafBrushNode_s>(clipMap->leafbrushNodesCount);

			// Leafbrushes
			clipMap->numLeafBrushes = clipMapJson["leafbrushes"].size();
			clipMap->leafbrushes = clipMap->numLeafBrushes == 0 ? nullptr : builder->getAllocator()->allocateArray<unsigned short>(clipMap->numLeafBrushes);
			Utils::Json::CopyArray(clipMap->leafbrushes, clipMapJson["leafbrushes"]);

			clipMap->numLeafSurfaces = clipMapJson["leafsurfaces"].size();
			clipMap->leafsurfaces = clipMap->numLeafSurfaces == 0 ? nullptr : builder->getAllocator()->allocateArray<unsigned int>(clipMap->numLeafSurfaces);
			Utils::Json::CopyArray(clipMap->leafsurfaces, clipMapJson["leafsurfaces"]);

			clipMap->vertCount = clipMapJson["verts"].size();
			clipMap->verts = clipMap->vertCount == 0 ? nullptr : builder->getAllocator()->allocateArray<Game::vec3_t>(clipMap->vertCount);
			for (size_t i = 0; i < clipMap->vertCount; i++)
			{
				Utils::Json::CopyArray(clipMap->verts[i], clipMapJson["verts"][i]);
			}

			for (size_t i = 0; i < clipMap->leafbrushNodesCount; i++)
			{
				auto* lbn = &clipMap->leafbrushNodes[i];
				auto jsonLbn = clipMapJson["leafbrushNodes"][i];

				lbn->axis = jsonLbn["axis"];
				lbn->leafBrushCount = jsonLbn["leafBrushCount"];
				lbn->contents = jsonLbn["contents"];

				if (lbn->leafBrushCount > 0)
				{
					int index = std::stoi(jsonLbn["data"].get<std::string>().substr(1));

					assert(index < clipMap->numLeafBrushes);
					assert(index >= 0);

					lbn->data.leaf.brushes = &clipMap->leafbrushes[index];
					assert(lbn->data.leaf.brushes);
				}
				else
				{
					lbn->data.children.dist = jsonLbn["data"]["dist"];
					lbn->data.children.range = jsonLbn["data"]["range"];
					Utils::Json::CopyArray(lbn->data.children.childOffset, jsonLbn["data"]["childOffset"]);
				}
			}

			// Tri indices
			auto indiceCountJson = clipMapJson["triIndices"].size();
			clipMap->triCount = indiceCountJson;
			clipMap->triIndices = clipMap->triCount == 0 ? nullptr : builder->getAllocator()->allocateArray<unsigned short>(clipMap->triCount * 3);

			for (size_t i = 0; i < clipMap->triCount * 3; i +=3)
			{
				Utils::Json::CopyArray(&clipMap->triIndices[i], clipMapJson["triIndices"][i/3]);
			}

			// Walkable
			auto walkableCount = 4 * ((3 * clipMap->triCount + 31) >> 5) * 3;
			assert(clipMapJson["triEdgeIsWalkable"].size() == walkableCount);
			clipMap->triEdgeIsWalkable = walkableCount == 0 ? nullptr : builder->getAllocator()->allocateArray<unsigned char>(walkableCount);
			Utils::Json::CopyArray(clipMap->triEdgeIsWalkable, clipMapJson["triEdgeIsWalkable"]);

			// Borders
			clipMap->borderCount = clipMapJson["borders"].size();
			clipMap->borders = clipMap->borderCount == 0 ? nullptr : builder->getAllocator()->allocateArray<Game::CollisionBorder>(clipMap->borderCount);
			for (size_t i = 0; i < clipMap->borderCount; i++)
			{
				auto border = &clipMap->borders[i];
				auto json_border = clipMapJson["borders"][i];

				Utils::Json::CopyArray(border->distEq, json_border["distEq"]);
				border->zBase = json_border["zBase"];
				border->zSlope = json_border["zSlope"];
				border->start = json_border["start"];
				border->length = json_border["length"];
			}


			// Collision partitions
			clipMap->partitionCount = clipMapJson["partitions"].size();
			clipMap->partitions = clipMap->partitionCount == 0 ? nullptr : builder->getAllocator()->allocateArray<Game::CollisionPartition>(clipMap->partitionCount);
			for (auto i = 0; i < clipMap->partitionCount; i++)
			{
				auto partition = &clipMap->partitions[i];
				auto json_partition = clipMapJson["partitions"][i];

				partition->triCount = json_partition["triCount"];
				partition->firstVertSegment = json_partition["firstVertSegment"];
				partition->firstTri = json_partition["firstTri"];
				partition->borderCount = json_partition["borderCount"];

				if (partition->borderCount > 0)
				{
					// They're always consecutive (I checked)
					auto index = std::stoi(json_partition["firstBorder"].get<std::string>().substr(1));
					assert(clipMap->borders);
					partition->borders = &clipMap->borders[index];
				}
			}

			// Tree
			clipMap->aabbTreeCount = clipMapJson["aabbTrees"].size();
			clipMap->aabbTrees = clipMap->aabbTreeCount == 0 ? nullptr : builder->getAllocator()->allocateArray<Game::CollisionAabbTree>(clipMap->aabbTreeCount);
			for (size_t i = 0; i < clipMap->aabbTreeCount; i++)
			{
				auto tree = &clipMap->aabbTrees[i];
				auto json_tree = clipMapJson["aabbTrees"][i];

				Utils::Json::CopyArray(tree->midPoint, json_tree["midPoint"]);
				Utils::Json::CopyArray(tree->halfSize, json_tree["halfSize"]);
				tree->materialIndex = json_tree["materialIndex"];
				tree->childCount = json_tree["childCount"];
				tree->u.firstChildIndex = json_tree["u"];
			}

			// CModels
			clipMap->numSubModels = clipMapJson["cmodels"].size();
			clipMap->cmodels = clipMap->numSubModels == 0 ? nullptr : builder->getAllocator()->allocateArray<Game::cmodel_t>(clipMap->numSubModels);
			for (size_t i = 0; i < clipMap->numSubModels; i++)
			{
				auto cmodel = &clipMap->cmodels[i];
				auto json_cmodel = clipMapJson["cmodels"][i];

				cmodel->bounds = Utils::Json::ReadBounds(json_cmodel["bounds"]);
				cmodel->radius = json_cmodel["radius"];

				auto leaf = &cmodel->leaf;
				auto json_leaf = json_cmodel["leaf"];

				leaf->firstCollAabbIndex = json_leaf["firstCollAabbIndex"];
				leaf->collAabbCount = json_leaf["collAabbCount"];
				leaf->brushContents = json_leaf["brushContents"];
				leaf->terrainContents = json_leaf["terrainContents"];
				leaf->leafBrushNode = json_leaf["leafBrushNode"];
				leaf->bounds = Utils::Json::ReadBounds(json_leaf["bounds"]);
				printf("");
			}

			// Brushes
			clipMap->numBrushes = static_cast<unsigned short>(clipMapJson["brushes"].size());
			clipMap->brushes = clipMap->numBrushes == 0 ? nullptr : builder->getAllocator()->allocateArray < Game::cbrush_t >(clipMap->numBrushes);
			for (size_t i = 0; i < clipMap->numBrushes; i++)
			{
				auto brush = &clipMap->brushes[i];
				auto json_brush = clipMapJson["brushes"][i];

				brush->glassPieceIndex = json_brush["glassPieceIndex"];
				brush->numsides = json_brush["numsides"];

				if (brush->numsides)
				{
					// Always consecutive
					auto index = std::stoi(json_brush["firstSide"].get<std::string>().substr(1));
					assert(clipMap->brushsides);
					brush->sides = &clipMap->brushsides[index];
				}

				if (json_brush["baseAdjacentSide"].is_string()) // Not null that means
				{
					auto index = std::stoi(json_brush["baseAdjacentSide"].get<std::string>().substr(1));
					assert(clipMap->brushEdges);
					brush->baseAdjacentSide = &clipMap->brushEdges[index];
				}

				for (size_t x = 0; x < 2; x++)
				{
					Utils::Json::CopyArray(brush->axialMaterialNum[x], json_brush["axialMaterialNum"][x]);
					Utils::Json::CopyArray(brush->firstAdjacentSideOffsets[x], json_brush["firstAdjacentSideOffsets"][x]);
					Utils::Json::CopyArray(brush->edgeCount[x], json_brush["edgeCount"][x]);
				}
			}

			assert(clipMapJson["brushes"].size() == clipMapJson["brushBounds"].size());
			clipMap->brushBounds = builder->getAllocator()->allocateArray<Game::Bounds>(clipMap->numBrushes);
			for (size_t i = 0; i < clipMap->numBrushes; i++)
			{
				clipMap->brushBounds[i] = Utils::Json::ReadBounds(clipMapJson["brushBounds"][i]);
			}

			assert(clipMapJson["brushes"].size() == clipMapJson["brushContents"].size());
			clipMap->brushContents = builder->getAllocator()->allocateArray<int>(clipMap->numBrushes);
			Utils::Json::CopyArray(clipMap->brushContents, clipMapJson["brushContents"]);

			auto json_ents= clipMapJson["mapEnts"];
			if (!json_ents.is_null())
			{
				auto ents_name = json_ents["name"];
				clipMap->mapEnts = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_MAP_ENTS, ents_name, builder).mapEnts;

				auto json_trigger = json_ents["trigger"];
				auto trigger = &clipMap->mapEnts->trigger;
				trigger->count = json_trigger["models"].size();
				trigger->models = builder->getAllocator()->allocateArray<Game::TriggerModel>(trigger->count);

				for (size_t i = 0; i < trigger->count; i++)
				{
					trigger->models[i].contents = json_trigger["models"][i]["contents"];
					trigger->models[i].hullCount = json_trigger["models"][i]["hullCount"];
					trigger->models[i].firstHull = json_trigger["models"][i]["firstHull"];
				}

				trigger->hullCount = json_trigger["hulls"].size();
				trigger->hulls = builder->getAllocator()->allocateArray<Game::TriggerHull>(trigger->hullCount);
				for (size_t i = 0; i < trigger->hullCount; i++)
				{
					trigger->hulls[i].bounds = Utils::Json::ReadBounds(json_trigger["hulls"][i]["bounds"]);

					trigger->hulls[i].contents = json_trigger["hulls"][i]["contents"];
					trigger->hulls[i].firstSlab = json_trigger["hulls"][i]["firstSlab"];
					trigger->hulls[i].slabCount = json_trigger["hulls"][i]["slabCount"];
				}

				trigger->slabCount = json_trigger["slabs"].size();
				trigger->slabs = builder->getAllocator()->allocateArray<Game::TriggerSlab>(trigger->slabCount);
				for (size_t i = 0; i < trigger->slabCount; i++)
				{
					Utils::Json::CopyArray(trigger->slabs[i].dir, json_trigger["slabs"][i]["dir"]);
					trigger->slabs[i].midPoint = json_trigger["slabs"][i]["midPoint"];
					trigger->slabs[i].halfSize = json_trigger["slabs"][i]["halfSize"];
				}

				// Stages
				clipMap->mapEnts->stageCount = static_cast<char>(json_ents["stages"].size());
				clipMap->mapEnts->stages = builder->getAllocator()->allocateArray <Game::Stage>(clipMap->mapEnts->stageCount);
				for (auto i = 0; i < clipMap->mapEnts->stageCount; i++)
				{
					auto stage = &clipMap->mapEnts->stages[i];
					auto json_stage = json_ents["stages"][i];

					stage->name = builder->getAllocator()->duplicateString(json_stage["name"]);
					Utils::Json::CopyArray(stage->origin, json_stage["origin"]);
					stage->triggerIndex = json_stage["triggerIndex"];
					stage->sunPrimaryLightIndex = json_stage["sunPrimaryLightIndex"].get<char>();
				}
			}

			// SmodelNodes
			clipMap->smodelNodeCount = static_cast<unsigned short>(clipMapJson["smodelNodes"].size());
			clipMap->smodelNodes = clipMap->smodelNodeCount == 0 ? nullptr : builder->getAllocator()->allocateArray<Game::SModelAabbNode>(clipMap->smodelNodeCount);
			for (size_t i = 0; i < clipMap->smodelNodeCount; i++)
			{
				auto json_node = clipMapJson["smodelNodes"][i];
				auto node = &clipMap->smodelNodes[i];
				
				node->bounds = Utils::Json::ReadBounds(json_node["bounds"]);
				node->firstChild = json_node["firstChild"];
				node->childCount = json_node["childCount"];
			}

			for (size_t i = 0; i < 2; i++)
			{
				clipMap->dynEntCount[i] = static_cast<unsigned short>(clipMapJson["dynEntities"][i].size());

				auto json_entities = clipMapJson["dynEntities"][i];
				clipMap->dynEntClientList[i] = clipMap->dynEntCount[i] == 0 ? nullptr : builder->getAllocator()->allocateArray<Game::DynEntityClient>(clipMap->dynEntCount[i]);
				clipMap->dynEntCollList[i] = clipMap->dynEntCount[i] == 0 ? nullptr : builder->getAllocator()->allocateArray<Game::DynEntityColl>(clipMap->dynEntCount[i]);
				clipMap->dynEntPoseList[i] = clipMap->dynEntCount[i] == 0 ? nullptr : builder->getAllocator()->allocateArray<Game::DynEntityPose>(clipMap->dynEntCount[i]);
				clipMap->dynEntDefList[i] = clipMap->dynEntCount[i] == 0 ? nullptr : builder->getAllocator()->allocateArray<Game::DynEntityDef>(clipMap->dynEntCount[i]);

				for (size_t j = 0; j < clipMap->dynEntCount[i]; j++)
				{
					auto json_entity = json_entities[j]["dynEntityDef"];
					auto entity = &clipMap->dynEntDefList[i][j];

					entity->type = json_entity["type"];
					Utils::Json::CopyArray(entity->pose.quat, json_entity["pose"]["quat"]);
					Utils::Json::CopyArray(entity->pose.origin, json_entity["pose"]["origin"]);

					entity->xModel = json_entity["xModel"].is_string() ? Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_XMODEL, json_entity["xModel"], builder).model : nullptr;
					entity->brushModel = json_entity["brushModel"];
					entity->physicsBrushModel = json_entity["physicsBrushModel"];
					entity->destroyFx = json_entity["destroyFx"].is_string() ? Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_FX, json_entity["destroyFx"], builder).fx : nullptr;
					entity->physPreset = json_entity["physPreset"].is_string() ? Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_PHYSPRESET, json_entity["physPreset"], builder).physPreset : nullptr;
					entity->health = json_entity["health"];
					
					Utils::Json::CopyArray(entity->mass.centerOfMass, json_entity["mass"]["centerOfMass"]);
					Utils::Json::CopyArray(entity->mass.momentsOfInertia, json_entity["mass"]["momentsOfInertia"]);
					Utils::Json::CopyArray(entity->mass.productsOfInertia, json_entity["mass"]["productsOfInertia"]);

					entity->contents = json_entity["contents"];
				}
			}

			clipMap->checksum = clipMapJson["checksum"];
		}
		catch (const std::exception& e)
		{
			Components::Logger::PrintError(Game::CON_CHANNEL_ERROR, "Malformed JSON for clipmap {}! {}", name, e.what());
			return;
		}

		header->clipMap = clipMap;
	}

	void IclipMap_t::dump(Game::XAssetHeader header)
	{
		assert(header.clipMap);

		static_assert(sizeof Game::clipMap_t == 256);
		static_assert(sizeof Game::ClipMaterial == 12);
		static_assert(sizeof Game::cbrushside_t == 8);
		static_assert(sizeof Game::cNode_t == 8);
		static_assert(sizeof Game::cLeaf_t == 40);
		static_assert(sizeof Game::cLeafBrushNode_s == 20);
		static_assert(sizeof Game::CollisionBorder == 28);
		static_assert(sizeof Game::CollisionPartition == 12);
		static_assert(sizeof Game::CollisionAabbTree == 32);
		static_assert(sizeof Game::cmodel_t == 68);
		static_assert(sizeof Game::SModelAabbNode == 28);


		const auto clipMap = header.clipMap;

		if (!clipMap) return;

		std::unordered_map<Game::cplane_s*, int> planes;
		std::unordered_map<Game::cbrushside_t*, int> brush_sides;
		std::unordered_map<unsigned char*, int> brush_edges;
		std::unordered_map<unsigned short*, int> leaf_brushes;
		std::unordered_map<Game::CollisionBorder*, int> borders;

		Utils::Memory::Allocator strDuplicator;
		nlohmann::json output;

		auto float_array_to_json = [](const float* array, int size)
		{
			auto arr = nlohmann::json::array();
			for (auto i = 0; i < size; ++i)
			{
				arr.push_back(array[i]);
			}

			return arr;
		};

		auto ushort_to_array = [](const unsigned short* array, int size)
		{
			auto arr = nlohmann::json::array();
			for (auto i = 0; i < size; ++i)
			{
				arr.push_back(array[i]);
			}

			return arr;
		};

		auto uchar_to_array = [](const unsigned char* array, int size)
		{
			auto arr = nlohmann::json::array();
			for (auto i = 0; i < size; ++i)
			{
				arr.push_back(array[i]);
			}

			return arr;
		};

		auto bounds_to_json = [&float_array_to_json](const Game::Bounds& bounds)
		{
			auto bounds_json= nlohmann::json::object();

			bounds_json.emplace("midPoint", float_array_to_json(bounds.midPoint, 3));
			bounds_json.emplace("halfSize", float_array_to_json(bounds.halfSize, 3));

			return bounds_json;
		};

		auto placement_to_json = [&float_array_to_json](const Game::GfxPlacement& placement)
		{
			auto placement_json= nlohmann::json::object();

			placement_json.emplace("quat", float_array_to_json(placement.quat, 4));
			placement_json.emplace("origin", float_array_to_json(placement.origin, 3));

			return placement_json;
		};

		auto leaf_to_json = [&bounds_to_json](const Game::cLeaf_t& leaf)
		{
			auto json_leave= nlohmann::json::object();

			json_leave.emplace("firstCollAabbIndex", leaf.firstCollAabbIndex);
			json_leave.emplace("collAabbCount", leaf.collAabbCount);
			json_leave.emplace("brushContents", leaf.brushContents);
			json_leave.emplace("terrainContents", leaf.terrainContents);
			json_leave.emplace("leafBrushNode", leaf.leafBrushNode);
			json_leave.emplace("bounds", bounds_to_json(leaf.bounds));

			return json_leave;
		};

		output.emplace("version", IW4X_CLIPMAP_VERSION);
		output.emplace("name", (clipMap->name));
		output.emplace("isInUse", clipMap->isInUse);

		// Planes
		auto json_planes = nlohmann::json::array();
		for (size_t i = 0; i < clipMap->planeCount; i++)
		{
			auto json_plane= nlohmann::json::object();
			auto plane = &clipMap->planes[i];

			json_plane.emplace(
				"normal",
				float_array_to_json(plane->normal, 3)
			);

			json_plane.emplace("dist", plane->dist);
			json_plane.emplace("type", plane->type);

			json_planes.push_back(json_plane);

			planes[plane] = i;
		}

		output.emplace("planes", json_planes);

		// Static models
		auto json_static_models = nlohmann::json::array();
		for (size_t i = 0; i < clipMap->numStaticModels; i++)
		{
			auto json_static_model= nlohmann::json::object();
			auto static_model = &clipMap->staticModelList[i];

			json_static_model.emplace(
				"xmodel", (static_model->xmodel->name)
			);

			json_static_model.emplace(
				"origin",
				float_array_to_json(static_model->origin, 3)
			);

			auto inv_scaled_axis = nlohmann::json::array();
			for (size_t j = 0; j < ARRAYSIZE(static_model->invScaledAxis); j++)
			{
				inv_scaled_axis.push_back(
					float_array_to_json(static_model->invScaledAxis[j], 3)
				);
			}

			json_static_model.emplace("invScaledAxis", inv_scaled_axis);

			json_static_model.emplace(
				"absBounds",
				bounds_to_json(static_model->absBounds)
			);

			json_static_models.push_back(json_static_model);
		}

		output.emplace("staticModelList", json_static_models);

		// Materials
		auto json_materials = nlohmann::json::array();
		for (size_t i = 0; i < clipMap->numMaterials; i++)
		{
			auto json_material= nlohmann::json::object();
			auto material = &clipMap->materials[i];

			json_material.emplace(
				"name",
				(material->name)
			);

			json_material.emplace("surfaceFlags", material->surfaceFlags);
			json_material.emplace("contents", material->contents);

			json_materials.push_back(json_material);
		}

		output.emplace("materials", json_materials);

		// Brushsides
		auto json_brush_sides = nlohmann::json::array();
		for (size_t i = 0; i < clipMap->numBrushSides; i++)
		{
			auto json_brush_side= nlohmann::json::object();
			auto brush_side = &clipMap->brushsides[i];

			auto index = planes[brush_side->plane];
			auto plane_index = std::format("#{}", index);
			json_brush_side.emplace(
				"plane",
				(strDuplicator.duplicateString(plane_index))
			);

			json_brush_side.emplace("materialNum", brush_side->materialNum);
			json_brush_side.emplace("firstAdjacentSideOffset", brush_side->firstAdjacentSideOffset);
			json_brush_side.emplace("edgeCount", brush_side->edgeCount);

			json_brush_sides.push_back(json_brush_side);

			brush_sides[brush_side] = i;
		}

		output.emplace("brushsides", json_brush_sides);

		// Brush edges
		auto json_brush_edges = nlohmann::json::array();
		for (size_t i = 0; i < clipMap->numBrushEdges; i++)
		{
			json_brush_edges.push_back(clipMap->brushEdges[i]);
			brush_edges[&clipMap->brushEdges[i]] = i;
		}

		output.emplace("brushEdges", json_brush_edges);

		// Brush nodes
		auto json_nodes = nlohmann::json::array();
		for (size_t i = 0; i < clipMap->numNodes; i++)
		{
			auto node = &clipMap->nodes[i];

			auto json_node= nlohmann::json::object();

			auto index = planes[node->plane];
			auto plane_index = std::format("#{}", index);
			json_node.emplace("plane", (strDuplicator.duplicateString(plane_index)));

			auto children = nlohmann::json::array();
			for (size_t j = 0; j < ARRAYSIZE(node->children); j++)
			{
				children.push_back(node->children[j]);
			}

			json_node.emplace("children", children);

			json_nodes.push_back(json_node);
		}

		output.emplace("nodes", json_nodes);

		// Leaves
		auto json_leaves = nlohmann::json::array();
		for (size_t i = 0; i < clipMap->numLeafs; i++)
		{
			auto leaf = &clipMap->leafs[i];

			json_leaves.push_back(leaf_to_json(*leaf));
		}

		output.emplace("leafs", json_leaves);

		// Brush leafs
		auto json_brush_leafs = nlohmann::json::array();
		for (size_t i = 0; i < clipMap->numLeafBrushes; i++)
		{
			json_brush_leafs.push_back(clipMap->leafbrushes[i]);
			leaf_brushes[&clipMap->leafbrushes[i]] = i;
		}

		output.emplace("leafbrushes", json_brush_leafs);


		// LeafBrushNodes
		auto json_leaf_brush_nodes = nlohmann::json::array();
		for (size_t i = 0; i < clipMap->leafbrushNodesCount; i++)
		{
			auto json_leaf_brush_node= nlohmann::json::object();
			auto leaf_brush_node = &clipMap->leafbrushNodes[i];

			json_leaf_brush_node.emplace("axis", leaf_brush_node->axis);
			json_leaf_brush_node.emplace("leafBrushCount", leaf_brush_node->leafBrushCount);
			json_leaf_brush_node.emplace("contents", leaf_brush_node->contents);

			if (leaf_brush_node->leafBrushCount > 0)
			{
				assert(leaf_brushes.contains(leaf_brush_node->data.leaf.brushes));
				auto index_str = std::format("#{}", leaf_brushes[leaf_brush_node->data.leaf.brushes]);
				json_leaf_brush_node.emplace("data", strDuplicator.duplicateString(index_str));
			}
			else
			{
				auto data= nlohmann::json::object();

				data.emplace("dist", leaf_brush_node->data.children.dist);
				data.emplace("range", leaf_brush_node->data.children.range);

				auto child_offset = nlohmann::json::array();
				for (size_t x = 0; x < 2; x++)
				{
					child_offset.push_back(leaf_brush_node->data.children.childOffset[x]);
				}

				data.emplace("childOffset", child_offset);

				json_leaf_brush_node.emplace("data", data);
			}

			json_leaf_brush_nodes.push_back(json_leaf_brush_node);
		}

		output.emplace("leafbrushNodes", json_leaf_brush_nodes);


		// leafsurfaces (unused)
		auto json_leaf_surfaces = nlohmann::json::array();
		for (size_t i = 0; i < clipMap->numLeafSurfaces; i++)
		{
			json_leaf_surfaces.push_back(clipMap->leafsurfaces[i]);
		}

		output.emplace("leafsurfaces", json_leaf_surfaces);

		// vertices
		auto json_vertices = nlohmann::json::array();
		for (size_t i = 0; i < clipMap->vertCount; i++)
		{
			json_vertices.push_back(float_array_to_json(clipMap->verts[i], 3));
		}

		output.emplace("verts", json_vertices);

		// tris
		auto json_tris = nlohmann::json::array();
		for (size_t i = 0; i < clipMap->triCount * 3; i += 3)
		{
			json_tris.push_back(ushort_to_array(&clipMap->triIndices[i], 3));
		}

		output.emplace("triIndices", json_tris);

		auto json_tris_walkable = nlohmann::json::array();
		auto walkable_count = 4 * ((3 * clipMap->triCount + 31) >> 5);
		for (size_t i = 0; i < walkable_count * 3; i++)
		{
			json_tris_walkable.push_back(static_cast<uint8_t>(clipMap->triEdgeIsWalkable[i]));
		}

		output.emplace("triEdgeIsWalkable", json_tris_walkable);

		// Collision borders
		auto json_collision_borders = nlohmann::json::array();
		for (size_t i = 0; i < clipMap->borderCount; i++)
		{
			auto collision_border = &clipMap->borders[i];

			auto json_collision_border= nlohmann::json::object();

			json_collision_border.emplace("distEq", float_array_to_json(collision_border->distEq, 3));
			json_collision_border.emplace("zBase", collision_border->zBase);
			json_collision_border.emplace("zSlope", collision_border->zSlope);
			json_collision_border.emplace("start", collision_border->start);
			json_collision_border.emplace("length", collision_border->length);

			json_collision_borders.push_back(json_collision_border);

			borders[collision_border] = i;
		}

		output.emplace("borders", json_collision_borders);

		// Collision partitions
		auto json_collision_partitions = nlohmann::json::array();
		for (auto i = 0; i < clipMap->partitionCount; i++)
		{
			auto collision_partition = &clipMap->partitions[i];

			auto json_collision_partition= nlohmann::json::object();

			json_collision_partition.emplace("triCount", collision_partition->triCount);
			json_collision_partition.emplace("firstVertSegment", collision_partition->firstVertSegment);
			json_collision_partition.emplace("firstTri", collision_partition->firstTri);
			json_collision_partition.emplace("borderCount", collision_partition->borderCount);

			if (collision_partition->borderCount)
			{
				auto index_str = strDuplicator.duplicateString(std::format("#{}", borders[&collision_partition->borders[0]]));
				json_collision_partition.emplace("firstBorder", (index_str));
			}

			json_collision_partitions.push_back(json_collision_partition);
		}

		output.emplace("partitions", json_collision_partitions);

		// Trees
		auto json_aabbtrees = nlohmann::json::array();
		for (size_t i = 0; i < clipMap->aabbTreeCount; i++)
		{
			auto aabbtree = &clipMap->aabbTrees[i];

			auto json_aabbtree= nlohmann::json::object();

			json_aabbtree.emplace("midPoint", float_array_to_json(aabbtree->midPoint, 3));
			json_aabbtree.emplace("halfSize", float_array_to_json(aabbtree->halfSize, 3));
			json_aabbtree.emplace("materialIndex", aabbtree->materialIndex);
			json_aabbtree.emplace("childCount", aabbtree->childCount);
			json_aabbtree.emplace("u", aabbtree->u.firstChildIndex);

			json_aabbtrees.push_back(json_aabbtree);
		}

		output.emplace("aabbTrees", json_aabbtrees);

		// Cmodels
		auto json_cmodels = nlohmann::json::array();
		for (size_t i = 0; i < clipMap->numSubModels; i++)
		{
			auto cmodel = &clipMap->cmodels[i];

			auto json_cmodel= nlohmann::json::object();

			json_cmodel.emplace("bounds", bounds_to_json(cmodel->bounds));
			json_cmodel.emplace("radius", cmodel->radius);
			json_cmodel.emplace("leaf", leaf_to_json(cmodel->leaf));

			json_cmodels.push_back(json_cmodel);
		}

		output.emplace("cmodels", json_cmodels);

		// Brushes
		auto json_brushes = nlohmann::json::array();
		for (size_t i = 0; i < clipMap->numBrushes; i++)
		{
			auto brush = &clipMap->brushes[i];

			auto json_brush= nlohmann::json::object();

			json_brush.emplace("glassPieceIndex", brush->glassPieceIndex);
			json_brush.emplace("numsides", brush->numsides);

			// Sides
			if (brush->numsides > 0)
			{
				auto index_str = strDuplicator.duplicateString(std::format("#{}", brush_sides[brush->sides]));
				json_brush.emplace("firstSide", (index_str));
			}

			if (brush->baseAdjacentSide)
			{
				auto index_str = strDuplicator.duplicateString(std::format("#{}", brush_edges[brush->baseAdjacentSide]));
				json_brush.emplace("baseAdjacentSide", (index_str));
			}
			else
			{
				json_brush.emplace("baseAdjacentSide", nlohmann::json());
			}

			auto axial_material_num = nlohmann::json::array();
			for (size_t x = 0; x < 2; x++)
			{
				axial_material_num.push_back(ushort_to_array(brush->axialMaterialNum[x], 3));
			}
			json_brush.emplace("axialMaterialNum", axial_material_num);

			auto first_adjacent_side_offsets = nlohmann::json::array();
			for (size_t x = 0; x < 2; x++)
			{
				first_adjacent_side_offsets.push_back(uchar_to_array(brush->firstAdjacentSideOffsets[x], 3));
			}
			json_brush.emplace("firstAdjacentSideOffsets", first_adjacent_side_offsets);

			auto edge_count = nlohmann::json::array();
			for (size_t x = 0; x < 2; x++)
			{
				edge_count.push_back(uchar_to_array(brush->edgeCount[x], 3));
			}
			json_brush.emplace("edgeCount", edge_count);

			json_brushes.push_back(json_brush);
		}

		output.emplace("brushes", json_brushes);

		// Brushbounds
		auto json_brush_bounds = nlohmann::json::array();
		for (size_t i = 0; i < clipMap->numBrushes; i++)
		{
			json_brush_bounds.push_back(bounds_to_json(clipMap->brushBounds[i]));
		}

		output.emplace("brushBounds", json_brush_bounds);

		// Brush contents
		auto json_brush_contents = nlohmann::json::array();
		for (size_t i = 0; i < clipMap->numBrushes; i++)
		{
			json_brush_contents.push_back(clipMap->brushContents[i]);
		}

		output.emplace("brushContents", json_brush_contents);

		// ENTITIES
		if (clipMap->mapEnts)
		{
			static_assert(sizeof Game::TriggerSlab == 20);
			static_assert(sizeof Game::TriggerModel == 8);
			static_assert(sizeof Game::TriggerHull == 32);

			auto json_map_ents= nlohmann::json::object();
			const auto ents = clipMap->mapEnts;

			json_map_ents.emplace("name", (ents->name));
		
			auto json_trigger= nlohmann::json::object();

			auto json_trigger_models = nlohmann::json::array();
			for (size_t i = 0; i < ents->trigger.count; i++)
			{
				auto json_trigger_model= nlohmann::json::object();
				json_trigger_model.emplace("contents", ents->trigger.models[i].contents);
				json_trigger_model.emplace("hullCount", ents->trigger.models[i].hullCount);
				json_trigger_model.emplace("firstHull", ents->trigger.models[i].firstHull);

				json_trigger_models.push_back(json_trigger_model);
			}

			json_trigger.emplace("models", json_trigger_models);

			auto json_trigger_hulls = nlohmann::json::array();
			for (size_t i = 0; i < ents->trigger.hullCount; i++)
			{
				auto json_trigger_hull= nlohmann::json::object();
				json_trigger_hull.emplace("bounds", bounds_to_json(ents->trigger.hulls[i].bounds));
				json_trigger_hull.emplace("contents", ents->trigger.hulls[i].contents);
				json_trigger_hull.emplace("slabCount", ents->trigger.hulls[i].slabCount);
				json_trigger_hull.emplace("firstSlab", ents->trigger.hulls[i].firstSlab);

				json_trigger_hulls.push_back(json_trigger_hull);
			}

			json_trigger.emplace("hulls", json_trigger_hulls);

			auto json_trigger_slabs = nlohmann::json::array();
			for (size_t i = 0; i < ents->trigger.slabCount; i++)
			{
				auto json_trigger_slab= nlohmann::json::object();
				json_trigger_slab.emplace("dir", float_array_to_json(ents->trigger.slabs[i].dir, 3));
				json_trigger_slab.emplace("midPoint", ents->trigger.slabs[i].midPoint);
				json_trigger_slab.emplace("halfSize", ents->trigger.slabs[i].halfSize);

				json_trigger_slabs.push_back(json_trigger_slab);
			}

			json_trigger.emplace("slabs", json_trigger_slabs);
			json_map_ents.emplace("trigger", json_trigger);

			auto json_stages = nlohmann::json::array();
			for (auto i = 0; i < ents->stageCount; i++)
			{
				auto stage = &ents->stages[i];
				auto json_stage= nlohmann::json::object();
				json_stage.emplace("name", (stage->name));
				json_stage.emplace("origin", float_array_to_json(stage->origin, 3));
				json_stage.emplace("triggerIndex", stage->triggerIndex);
				json_stage.emplace("sunPrimaryLightIndex", stage->sunPrimaryLightIndex);

				json_stages.push_back(json_stage);
			}

			json_map_ents.emplace("stages", json_stages);

			output.emplace("mapEnts", json_map_ents);
		}
		else
		{
			output.emplace("mapEnts", nlohmann::json());
		}

		//Smodel nodes
		auto json_smodelnodes = nlohmann::json::array();
		for (size_t i = 0; i < clipMap->smodelNodeCount; i++)
		{
			auto smodelNode = &clipMap->smodelNodes[i];

			auto json_smodelnode = nlohmann::json::object();

			json_smodelnode.emplace("bounds", bounds_to_json(smodelNode->bounds));
			json_smodelnode.emplace("firstChild", smodelNode->firstChild);
			json_smodelnode.emplace("childCount", smodelNode->childCount);

			json_smodelnodes.push_back(json_smodelnode);
		}

		output.emplace("smodelNodes", json_smodelnodes);

		// Dynent
		auto json_dyn_entities = nlohmann::json::array();
		for (size_t i = 0; i < ARRAYSIZE(clipMap->dynEntCount); i++)
		{
			auto def_list = clipMap->dynEntDefList[i];
			if (def_list)
			{
				auto json_dyn_entity_def_list = nlohmann::json::array();

				for (size_t j = 0; j < clipMap->dynEntCount[i]; j++)
				{
					auto json_dyn_entity_def_pack= nlohmann::json::object();
					auto json_dyn_entity_def= nlohmann::json::object();
					auto def = &def_list[j];

					json_dyn_entity_def.emplace("type", def->type);
					json_dyn_entity_def.emplace("pose", placement_to_json(def->pose));
					json_dyn_entity_def.emplace("xModel", def->xModel ? (def->xModel->name) : nlohmann::json());
					json_dyn_entity_def.emplace("brushModel", def->brushModel);
					json_dyn_entity_def.emplace("physicsBrushModel", def->physicsBrushModel);
					json_dyn_entity_def.emplace("destroyFx", def->destroyFx ? (def->destroyFx->name) : nlohmann::json());
					json_dyn_entity_def.emplace("physPreset", def->physPreset ? (def->physPreset->name) : nlohmann::json());
					json_dyn_entity_def.emplace("health", def->health);

					auto json_mass= nlohmann::json::object();
					json_mass.emplace("centerOfMass", float_array_to_json(def->mass.centerOfMass, 3));
					json_mass.emplace("momentsOfInertia", float_array_to_json(def->mass.momentsOfInertia, 3));
					json_mass.emplace("productsOfInertia", float_array_to_json(def->mass.productsOfInertia, 3));
					json_dyn_entity_def.emplace("mass", json_mass);

					json_dyn_entity_def.emplace("contents", def->contents);

					json_dyn_entity_def_pack.emplace("dynEntityDef", json_dyn_entity_def);

					/// All that follows is garbage data
					//auto json_dyn_entity_pose= nlohmann::json::object();
					//auto pose = &clipMap->dynEntPoseList[i][j];
					//json_dyn_entity_pose.emplace("pose", placement_to_json(pose->pose));
					//json_dyn_entity_pose.emplace("radius", pose->radius);
					//json_dyn_entity_def_pack.emplace("dynEntPose", json_dyn_entity_pose);

					//auto json_dyn_entity_client= nlohmann::json::object();
					//auto client = &clipMap->dynEntClientList[i][j];
					//json_dyn_entity_client.emplace("physObjId", client->physObjId);
					//json_dyn_entity_client.emplace("flags", client->flags);
					//json_dyn_entity_client.emplace("lightingHandle", client->lightingHandle);
					//json_dyn_entity_client.emplace("health", client->health);
					//json_dyn_entity_def_pack.emplace("dynEntClient", json_dyn_entity_client);

					//auto json_dyn_entity_coll= nlohmann::json::object();
					//auto coll = &clipMap->dynEntCollList[i][j];
					//json_dyn_entity_coll.emplace("sector", coll->sector);
					//json_dyn_entity_coll.emplace("nextEntInSector", coll->nextEntInSector);
					//json_dyn_entity_coll.emplace("linkMins", float_array_to_json(coll->linkMins, 2));
					//json_dyn_entity_coll.emplace("linkMaxs", float_array_to_json(coll->linkMaxs, 2));
					//json_dyn_entity_def_pack.emplace("dynEntColl", json_dyn_entity_coll);
					//

					json_dyn_entity_def_list.push_back(json_dyn_entity_def_pack);
				}

				json_dyn_entities.push_back(json_dyn_entity_def_list);
			}
			else
			{

				json_dyn_entities.push_back(nlohmann::json());
			}
		}
		output.emplace("dynEntities", json_dyn_entities);

		// Checksum
		output.emplace("checksum", clipMap->checksum);

		// Write to disk
		constexpr auto* prefix = "maps/mp/";
		constexpr auto* suffix = ".d3dbsp";
		Utils::IO::WriteFile(std::format("raw/clipmap/{}{}{}.iw4x.json", prefix, header.clipMap->name, suffix), output.dump(4));

	}
}
