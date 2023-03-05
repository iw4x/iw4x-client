#include <STDInclude.hpp>
#include "IclipMap_t.hpp"

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
				else
				{
					AssertUnreachable;
					Components::Logger::Error(Game::ERR_FATAL, "node {} of clipmap {} has no plane!", i, asset->name);
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

		builder->loadAsset(Game::XAssetType::ASSET_TYPE_MAP_ENTS, asset->mapEnts);
	}

	void IclipMap_t::load(Game::XAssetHeader* header, const std::string& _name, Components::ZoneBuilder::Zone* builder)
	{
		header->clipMap = builder->getIW4OfApi()->read<Game::clipMap_t>(Game::XAssetType::ASSET_TYPE_CLIPMAP_MP, _name);
		assert(header->data);
	}
}
