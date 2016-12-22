#include <StdInclude.hpp>

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
				builder->storePointer(asset->cPlanes);

				buffer->saveArray(asset->cPlanes, asset->numCPlanes);
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
					destStaticModelList[i].xmodel = builder->requireAsset(Game::XAssetType::ASSET_TYPE_XMODEL, asset->staticModelList[i].xmodel->name).model;
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
			buffer->saveArray(asset->cBrushSides, asset->numCBrushSides);

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
			buffer->saveArray(asset->cBrushEdges, asset->numCBrushEdges);
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
							builder->storePointer(node[i].data.brushes);

							buffer->saveArray(node[i].data.brushes, node[i].leafBrushCount);
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
			buffer->saveArray(&asset->collisionBorders, asset->numCollisionBorders);
			Utils::Stream::ClearPointer(&dest->collisionBorders);
			SaveLogExit();
		}

		if (asset->collisionPartitions)
		{
			AssertSize(Game::CollisionPartition, 12);
			SaveLogEnter("CollisionPartition");

			buffer->align(Utils::Stream::ALIGN_4);
			Game::CollisionPartition* border = buffer->dest<Game::CollisionPartition>();
			buffer->saveArray(asset->collisionPartitions, asset->numCollisionPartitions);

			for (int i = 0; i < asset->numCollisionPartitions; ++i)
			{
				if (border[i].borders)
				{
					if (builder->hasPointer(border[i].borders))
					{
						border[i].borders = builder->getPointer(border[i].borders);
					}
					else
					{
						buffer->align(Utils::Stream::ALIGN_4);
						builder->storePointer(border[i].borders);

						buffer->save(border[i].borders);
						Utils::Stream::ClearPointer(&border[i].borders);
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
			Game::cbrush_t* brushes = buffer->dest<Game::cbrush_t>();
			buffer->saveArray(asset->cBrushes, asset->numCBrushes);

			for (short i = 0; i < asset->numCBrushes; ++i)
			{
				if (brushes[i].brushSide)
				{
					if (builder->hasPointer(brushes[i].brushSide))
					{
						brushes[i].brushSide = builder->getPointer(brushes[i].brushSide);
					}
					else
					{
						AssertSize(Game::cbrushside_t, 8);

						buffer->align(Utils::Stream::ALIGN_4);
						builder->storePointer(brushes[i].brushSide);

						Game::cbrushside_t* side = buffer->dest<Game::cbrushside_t>();
						buffer->save(brushes[i].brushSide);

						if (brushes[i].brushSide->side)
						{
							if (builder->hasPointer(brushes[i].brushSide->side))
							{
								brushes[i].brushSide->side = builder->getPointer(brushes[i].brushSide->side);
							}
							else
							{
								buffer->align(Utils::Stream::ALIGN_4);
								builder->storePointer(brushes[i].brushSide->side);

								buffer->save(brushes[i].brushSide->side);
								Utils::Stream::ClearPointer(&side->side);
							}
						}

						Utils::Stream::ClearPointer(&brushes[i].brushSide);
					}
				}

				if (brushes[i].brushEdge)
				{
					if (builder->hasPointer(brushes[i].brushEdge))
					{
						brushes[i].brushEdge = builder->getPointer(brushes[i].brushEdge);
					}
					else
					{
						builder->storePointer(brushes[i].brushEdge);

						buffer->save(brushes[i].brushEdge);
						Utils::Stream::ClearPointer(&brushes[i].brushEdge);
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
			dest->mapEnts = builder->requireAsset(Game::XAssetType::ASSET_TYPE_MAP_ENTS, asset->name).mapEnts;
		}

		for (int i = 0; i < 2; ++i)
		{
			if (asset->dynEntDefList[i])
			{
				AssertSize(Game::DynEntityDef, 92);

				buffer->align(Utils::Stream::ALIGN_4);
				Game::DynEntityDef* dynEntDest = buffer->dest<Game::DynEntityDef>();
				buffer->saveArray(asset->dynEntDefList[i], asset->dynEntCount[i]);

				for (int j = 0; j < asset->dynEntCount[i]; ++j)
				{
					Game::XModel* m = asset->dynEntDefList[i][j].xModel;
					Game::FxEffectDef* fx = asset->dynEntDefList[i][j].destroyFx;
					Game::PhysPreset* p = asset->dynEntDefList[i][j].physPreset;

					if (m)
					{
						dynEntDest[j].xModel = builder->requireAsset(Game::XAssetType::ASSET_TYPE_XMODEL, m->name).model;
					}

					if (fx)
					{
						dynEntDest[j].destroyFx = builder->requireAsset(Game::XAssetType::ASSET_TYPE_FX, fx->name).fx;
					}

					if (p)
					{
						dynEntDest[j].physPreset = builder->requireAsset(Game::XAssetType::ASSET_TYPE_PHYSPRESET, p->name).physPreset;
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
				buffer->save(asset->dynEntClientList[i], asset->dynEntCount[i]);
				Utils::Stream::ClearPointer(&dest->dynEntClientList[i]);
			}
		}

		for (int i = 0; i < 2; ++i)
		{
			if (asset->dynEntCollList[i])
			{
				AssertSize(Game::DynEntityColl, 20);

				buffer->align(Utils::Stream::ALIGN_4);
				buffer->save(asset->dynEntCollList[i], asset->dynEntCount[i]);
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
			builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL, m->name);
		}

		for (int j = 0; j < 2; ++j)
		{
			Game::DynEntityDef* def = asset->dynEntDefList[j];

			for (int i = 0; i < asset->dynEntCount[j]; ++i)
			{
				if (def[i].xModel)
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL, def[i].xModel->name);
				}

				if (def[i].destroyFx)
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_FX, def[i].destroyFx->name);
				}

				if (def[i].physPreset)
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_PHYSPRESET, def[i].physPreset->name);
				}
			}
		}
		builder->loadAsset(Game::XAssetType::ASSET_TYPE_MAP_ENTS, asset->name);
	}

	void IclipMap_t::load(Game::XAssetHeader* /*header*/, std::string name, Components::ZoneBuilder::Zone* /*builder*/)
	{
		Game::clipMap_t* map = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_COL_MAP_MP, name.data()).clipMap;
		if (map) return;

		Components::Logger::Error("Missing clipMap_t %s... you can't make them yet you idiot.", name.data());
	}
}
