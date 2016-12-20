#include <StdInclude.hpp>

namespace Assets
{
    void IclipMap_t::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
    {
        AssertSize(Game::clipMap_t, 256);

		Utils::Stream* buffer = builder->getBuffer();
		Game::clipMap_t* asset = header.clipMap;
		Game::clipMap_t* dest = buffer->dest<Game::clipMap_t>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

    	if (asset->cPlanes) // OffsetToPointer support
    	{
    		buffer->align(Utils::Stream::ALIGN_4);
    		buffer->save(asset->cPlanes, sizeof(Game::cPlane), asset->numCPlanes);
    		Utils::Stream::ClearPointer(&dest->cPlanes);
    	}

    	if (asset->staticModelList)
    	{
    		// xmodel is already stored
    		buffer->align(Utils::Stream::ALIGN_4);
            Game::cStaticModel* destStaticModelList = buffer->dest<Game::cStaticModel>();
    		buffer->save(asset->staticModelList, sizeof(Game::cStaticModel), asset->numStaticModels);
            for (int i = 0; i < asset->numStaticModels; i++) {
                if (asset->staticModelList[i].xmodel)
                {
                    destStaticModelList[i].xmodel = builder->requireAsset(Game::XAssetType::ASSET_TYPE_XMODEL, asset->staticModelList[i].xmodel->name).model;
                }
            }
    		Utils::Stream::ClearPointer(&dest->staticModelList);
    	}

    	if (asset->materials)
    	{
    		buffer->align(Utils::Stream::ALIGN_4);
    		Game::dMaterial* mats = buffer->dest<Game::dMaterial>();
    		buffer->save(asset->materials, sizeof(Game::dMaterial), asset->numMaterials);
    		for(int i=0; i<asset->numMaterials; i++)
    		{
    			buffer->save(asset->materials[i].name, strlen(asset->materials[i].name) + 1, 1);
    			Utils::Stream::ClearPointer(&mats[i].name);
    		}
    		Utils::Stream::ClearPointer(&dest->materials);
    	}

    	if (asset->cBrushSides)
    	{
    		buffer->align(Utils::Stream::ALIGN_4);
    		Game::cBrushSide* sides = buffer->dest<Game::cBrushSide>();
    		buffer->save(asset->cBrushSides, sizeof(Game::cBrushSide), asset->numCBrushSides);
    		for (int i = 0; i<asset->numCBrushSides; i++)
    		{
    			if (sides[i].side) // OffsetToPointer
    			{
    				buffer->align(Utils::Stream::ALIGN_4);
    				buffer->save(sides[i].side, sizeof(Game::cPlane), 1);
    				Utils::Stream::ClearPointer(&sides[i].side);
    			}
    		}
    		Utils::Stream::ClearPointer(&dest->cBrushSides);
    	}

    	if (asset->cBrushEdges)
    	{
    		// no align for char
    		buffer->save(asset->cBrushEdges, 1, asset->numCBrushEdges);
    		Utils::Stream::ClearPointer(&dest->cBrushEdges);
    	}

    	if (asset->cNodes)
    	{
    		buffer->align(Utils::Stream::ALIGN_4);
    		Game::cNode* nodes = buffer->dest<Game::cNode>();
    		buffer->save(asset->cNodes, sizeof(Game::cNode), asset->numCNodes);
    		for (int i = 0; i<asset->numCNodes; i++)
    		{
    			if (nodes[i].plane) // OffsetToPointer
    			{
    				buffer->align(Utils::Stream::ALIGN_4);
    				buffer->save(nodes[i].plane, sizeof(Game::cPlane), 1);
    				Utils::Stream::ClearPointer(&nodes[i].plane);
    			}
    		}
    		Utils::Stream::ClearPointer(&dest->cNodes);
    	}

    	if (asset->cLeaf)
    	{
            buffer->align(Utils::Stream::ALIGN_4);
    		buffer->save(asset->cLeaf, sizeof(Game::cLeaf), asset->numCLeaf);
    		Utils::Stream::ClearPointer(&dest->cLeaf);
    	}

    	if (asset->leafBrushes)
    	{
            buffer->align(Utils::Stream::ALIGN_2);
    		buffer->save(asset->leafBrushes, 2, asset->numLeafBrushes);
    		Utils::Stream::ClearPointer(&dest->leafBrushes);
    	}

    	if (asset->cLeafBrushNodes)
    	{
            buffer->align(Utils::Stream::ALIGN_4);
    		Game::cLeafBrushNode* node = buffer->dest<Game::cLeafBrushNode>();
    		buffer->save(asset->cLeafBrushNodes, sizeof(Game::cLeafBrushNode) , asset->numCLeafBrushNodes);
    		for(int i=0; i<asset->numCLeafBrushNodes; i++)
    		{
    			if(node[i].leafBrushCount > 0)
    			{
    				if(node[i].data.leaf.brushes) // OffsetToPointer
    				{
    					buffer->align(Utils::Stream::ALIGN_2);
    					buffer->save(node[i].data.leaf.brushes, 2, node[i].leafBrushCount);
    					Utils::Stream::ClearPointer(&node[i].data.leaf.brushes);
    				}
    			}
    		}
    		Utils::Stream::ClearPointer(&dest->cLeafBrushNodes);
    	}

    	if (asset->leafSurfaces)
    	{
    		buffer->align(Utils::Stream::ALIGN_4);
    		buffer->save(asset->leafSurfaces, sizeof(int), asset->numLeafSurfaces);
    		Utils::Stream::ClearPointer(&dest->leafSurfaces);
    	}

    	if (asset->verts)
    	{
    		buffer->align(Utils::Stream::ALIGN_4);
    		buffer->save(asset->verts, sizeof(Game::vec3_t), asset->numVerts);
    		Utils::Stream::ClearPointer(&dest->verts);
    	}

    	if (asset->triIndices)
    	{
    		buffer->align(Utils::Stream::ALIGN_2);
    		buffer->save(asset->triIndices, sizeof(short), 3 * asset->numTriIndices);
    		Utils::Stream::ClearPointer(&dest->triIndices);
    	}

    	if (asset->triEdgeIsWalkable)
    	{
    		// no align for char
    		buffer->save(asset->triEdgeIsWalkable, sizeof(char), ((3 * asset->numTriIndices + 31) >> 3) & 0xFFFFFFFC);
    		Utils::Stream::ClearPointer(&dest->triEdgeIsWalkable);
    	}

    	if (asset->collisionBorders)
    	{
    		buffer->align(Utils::Stream::ALIGN_4);
    		buffer->save(asset->collisionBorders, sizeof(Game::CollisionBorder), asset->numCollisionBorders);
    		Utils::Stream::ClearPointer(&dest->collisionBorders);
    	}

    	if (asset->collisionPartitions)
    	{
    		buffer->align(Utils::Stream::ALIGN_4);
    		Game::CollisionPartition* border = buffer->dest<Game::CollisionPartition>();
    		buffer->save(asset->collisionPartitions, sizeof(Game::CollisionPartition), asset->numCollisionPartitions);
    		for (int i = 0; i<asset->numCollisionPartitions; i++)
    		{
    			if (border[i].borders) // OffsetToPointer
    			{
    				buffer->align(Utils::Stream::ALIGN_4);
    				buffer->save(border[i].borders, sizeof(Game::CollisionBorder), 1);
    				Utils::Stream::ClearPointer(&border[i].borders);
    			}
    		}
    		Utils::Stream::ClearPointer(&dest->collisionPartitions);
    	}

    	if (asset->collisionAABBTrees)
    	{
    		buffer->align(Utils::Stream::ALIGN_16);
    		buffer->save(asset->collisionAABBTrees, sizeof(Game::CollisionAabbTree), asset->numCollisionAABBTrees);
    		Utils::Stream::ClearPointer(&dest->collisionAABBTrees);
    	}

    	if (asset->cModels)
    	{
    		buffer->align(Utils::Stream::ALIGN_4);
    		buffer->save(asset->cModels, sizeof(Game::cModel), asset->numCModels);
    		Utils::Stream::ClearPointer(&dest->cModels);
    	}

    	if (asset->cBrushes)
    	{
    		buffer->align(Utils::Stream::ALIGN_128);
    		Game::cBrush* brushes = buffer->dest<Game::cBrush>();
    		buffer->save(asset->cBrushes, sizeof(Game::cBrush), asset->numCBrushes);
    		for(int i=0; i<asset->numCBrushes; i++)
    		{
    			if(brushes[i].brushSide)
    			{
    				Game::cBrushSide* side = buffer->dest<Game::cBrushSide>();
    				buffer->save(brushes[i].brushSide, sizeof(Game::cBrushSide), 1);
    				if (brushes[i].brushSide->side)
    				{
    					buffer->align(Utils::Stream::ALIGN_4);
    					buffer->save(brushes[i].brushSide->side, sizeof(Game::cPlane), 1);
    					Utils::Stream::ClearPointer(&side->side);
    				}
    				Utils::Stream::ClearPointer(&brushes[i].brushSide);
    			}
    			if(brushes[i].brushEdge)
    			{
    				buffer->save(brushes[i].brushEdge, 1, 1);
    				Utils::Stream::ClearPointer(&brushes[i].brushEdge);
    			}
    		}
    		Utils::Stream::ClearPointer(&dest->cBrushes);
    	}

    	if (asset->cBrushBounds)
    	{
    		buffer->align(Utils::Stream::ALIGN_128);
    		buffer->save(asset->cBrushBounds, sizeof(Game::Bounds), asset->numCBrushes);
    		Utils::Stream::ClearPointer(&dest->cBrushBounds);
    	}

    	if (asset->cBrushContents)
    	{
    		buffer->align(Utils::Stream::ALIGN_4);
    		buffer->save(asset->cBrushContents, sizeof(int), asset->numCBrushes);
    		Utils::Stream::ClearPointer(&dest->cBrushContents);
    	}

    	if (asset->unknown4)
    	{
    		buffer->align(Utils::Stream::ALIGN_4);
    		buffer->save(asset->unknown4, 28, asset->unkCount4);
    		Utils::Stream::ClearPointer(&dest->unknown4);
    	}

        if (asset->mapEnts)
        {
            dest->mapEnts = builder->requireAsset(Game::XAssetType::ASSET_TYPE_MAP_ENTS, asset->name).mapEnts;
        }

    	for (int i = 0; i < 2; i++)
    	{
    		if (asset->dynEntDefList[i])
    		{
    			buffer->align(Utils::Stream::ALIGN_4);
                Game::DynEntityDef* dynEntDest = buffer->dest<Game::DynEntityDef>();
                buffer->save(asset->dynEntDefList[i], sizeof(Game::DynEntityDef), asset->dynEntCount[i]);
                for(int j = 0; j < asset->dynEntCount[i]; j++)
                {
                    Game::XModel* m = asset->dynEntDefList[j]->xModel;
                    Game::FxEffectDef* fx = asset->dynEntDefList[j]->destroyFx;
                    Game::PhysPreset* p = asset->dynEntDefList[j]->physPreset;

                    if (m)
                    {
                        dynEntDest->xModel = builder->requireAsset(Game::XAssetType::ASSET_TYPE_XMODEL, m->name).model;
                    }

                    if (fx)
                    {
                        dynEntDest->destroyFx = builder->requireAsset(Game::XAssetType::ASSET_TYPE_FX, fx->name).fx;
                    }

                    if (p)
                    {
                        dynEntDest->physPreset = builder->requireAsset(Game::XAssetType::ASSET_TYPE_PHYSPRESET, p->name).physPreset;
                    }
                }

    			Utils::Stream::ClearPointer(&dest->dynEntDefList[i]);
    		}
    	}

    	buffer->pushBlock(Game::XFILE_BLOCK_RUNTIME);

    	for (int i = 0; i < 2; i++)
    	{
    		if (asset->dynEntPoseList[i])
    		{
    			buffer->align(Utils::Stream::ALIGN_4);
    			buffer->save(asset->dynEntPoseList[i], sizeof(Game::DynEntityPose), asset->dynEntCount[i]);
    			Utils::Stream::ClearPointer(&dest->dynEntPoseList[i]);

    		}
    	}

    	for (int i = 0; i < 2; i++)
    	{
    		if (asset->dynEntClientList[i])
    		{
    			buffer->align(Utils::Stream::ALIGN_4);
    			buffer->save(asset->dynEntClientList[i], sizeof(Game::DynEntityClient), asset->dynEntCount[i]);
    			Utils::Stream::ClearPointer(&dest->dynEntClientList[i]);
    		}
    	}

    	for (int i = 0; i < 2; i++)
    	{
    		if (asset->dynEntCollList[i])
    		{
    			buffer->align(Utils::Stream::ALIGN_4);
    			buffer->save(asset->dynEntCollList[i], sizeof(Game::DynEntityColl), asset->dynEntCount[i]);
    			Utils::Stream::ClearPointer(&dest->dynEntCollList[i]);
    		}
    	}

    	buffer->popBlock();
    	buffer->popBlock();
    }

    void IclipMap_t::mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
    {
        Game::clipMap_t* asset = header.clipMap;
        for(int i=0; i<asset->numStaticModels; i++)
        {
            Game::XModel* m = asset->staticModelList[i].xmodel;
            builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL, m->name);
        }

        for (int j = 0; j < 2; j++)
        {
            for (int i = 0; i < asset->dynEntCount[j]; i++)
            {
                Game::XModel* m = asset->dynEntDefList[j]->xModel;
                builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL, m->name);
                Game::FxEffectDef* fx = asset->dynEntDefList[j]->destroyFx;
                builder->loadAsset(Game::XAssetType::ASSET_TYPE_FX, fx->name);
                Game::PhysPreset* p = asset->dynEntDefList[j]->physPreset;
                builder->loadAsset(Game::XAssetType::ASSET_TYPE_PHYSPRESET, p->name);
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
