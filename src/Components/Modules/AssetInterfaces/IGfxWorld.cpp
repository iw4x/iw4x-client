#include <STDInclude.hpp>

namespace Assets
{
    void IGfxWorld::load(Game::XAssetHeader* header, std::string name, Components::ZoneBuilder::Zone* builder)
	{
		Game::GfxWorld* map = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_GFX_MAP, name.data()).gfxMap;
		if (map) return;
		Components::Logger::Error("Missing GfxMap %s... you can't make them yet you idiot.", name.data());
        *(header);
        *(builder);
    }

    void IGfxWorld::saveGfxWorldDpvsPlanes(Game::GfxWorld* world, Game::GfxWorldDpvsPlanes* asset, Game::GfxWorldDpvsPlanes* dest, Components::ZoneBuilder::Zone* builder) {
        AssertSize(Game::GfxWorldDpvsPlanes, 16);

        Utils::Stream* buffer = builder->getBuffer();

        if(asset->planes) {
            if(builder->hasPointer(asset->planes)) {
                dest->planes = builder->getPointer(asset->planes);
            } else {
                buffer->align(Utils::Stream::ALIGN_4);
                builder->storePointer(asset->planes);

                buffer->saveArray(asset->planes, world->planeCount);
                Utils::Stream::ClearPointer(&dest->planes);
            }
        }

        if (asset->nodes)
        {
            buffer->align(Utils::Stream::ALIGN_2);
            buffer->saveArray(asset->nodes, world->nodeCount);
            Utils::Stream::ClearPointer(&dest->nodes);
        }

        buffer->pushBlock(Game::XFILE_BLOCK_RUNTIME);

        if (asset->sceneEntCellBits)
        {
            buffer->align(Utils::Stream::ALIGN_4);
            buffer->saveArray(asset->sceneEntCellBits, asset->cellCount);
            Utils::Stream::ClearPointer(&dest->sceneEntCellBits);
        }

        buffer->popBlock();
    }

    void IGfxWorld::saveGfxWorldDraw(Game::GfxWorldDraw* asset, Game::GfxWorldDraw* dest, Components::ZoneBuilder::Zone* builder) {
        AssertSize(Game::GfxWorldDraw, 72);

        Utils::Stream* buffer = builder->getBuffer();

        if (asset->reflectionImages)
        {
            buffer->align(Utils::Stream::ALIGN_4);

            Game::GfxImage** imageDest = buffer->dest<Game::GfxImage*>();
            buffer->saveArray(asset->reflectionImages, asset->reflectionProbeCount);

            for (unsigned int i = 0; i < asset->reflectionProbeCount; i++)
            {
                imageDest[i] = builder->requireAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->reflectionImages[i]->name).image;
            }
            Utils::Stream::ClearPointer(asset->reflectionImages);
        }

        if (asset->reflectionProbes)
        {
            buffer->align(Utils::Stream::ALIGN_4);
            buffer->saveArray(asset->reflectionProbes, asset->reflectionProbeCount);
            Utils::Stream::ClearPointer(&dest->reflectionProbes);
        }

        buffer->pushBlock(Game::XFILE_BLOCK_RUNTIME);

        if (asset->reflectionProbeTextures)
        {
            buffer->align(Utils::Stream::ALIGN_4);
            buffer->saveArray(asset->reflectionProbeTextures, asset->reflectionProbeCount);
            Utils::Stream::ClearPointer(&dest->reflectionProbeTextures);
        }

        buffer->popBlock();

        if (asset->lightmaps)
        {
            buffer->align(Utils::Stream::ALIGN_4);

            Game::GfxLightmapArray* lightmapArrayDestTable = buffer->dest<Game::GfxLightmapArray>();
            buffer->saveArray(asset->lightmaps, asset->lightmapCount);

            for (int i = 0; i < asset->lightmapCount; i++)
            {
                Game::GfxLightmapArray* lightmapArrayDest = &lightmapArrayDestTable[i];
                Game::GfxLightmapArray* lightmapArray = &asset->lightmaps[i];

                if (lightmapArray->primary) {
                    lightmapArrayDest->primary = builder->requireAsset(Game::XAssetType::ASSET_TYPE_IMAGE, lightmapArray->primary->name).image;
                }

                if (lightmapArray->secondary) {
                    lightmapArrayDest->secondary = builder->requireAsset(Game::XAssetType::ASSET_TYPE_IMAGE, lightmapArray->secondary->name).image;
                }
            }
            Utils::Stream::ClearPointer(asset->reflectionImages);
        }

        buffer->pushBlock(Game::XFILE_BLOCK_RUNTIME);

        if (asset->lightmapPrimaryTextures)
        {
            buffer->align(Utils::Stream::ALIGN_4);
            buffer->saveArray(asset->lightmapPrimaryTextures, asset->lightmapCount);
            Utils::Stream::ClearPointer(&dest->lightmapPrimaryTextures);
        }

        if (asset->lightmapSecondaryTextures)
        {
            buffer->align(Utils::Stream::ALIGN_4);
            buffer->saveArray(asset->lightmapSecondaryTextures, asset->lightmapCount);
            Utils::Stream::ClearPointer(&dest->lightmapSecondaryTextures);
        }

        buffer->popBlock();

        if(asset->skyImage) {
            dest->skyImage = builder->requireAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->skyImage->name).image;
        }

        if(asset->outdoorImage) {
            dest->outdoorImage = builder->requireAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->outdoorImage->name).image;
        }

        // the next 2 blocks *SHOULD* be in their own functions but i'm too lazy to type it all out

        // saveGfxWorldVertexData
        if (asset->vd.vertices)
        {
            buffer->align(Utils::Stream::ALIGN_4);
            buffer->saveArray(asset->vd.vertices, asset->vertexCount);
            Utils::Stream::ClearPointer(&dest->vd.vertices);
        }

        // this one has no if statement on purpouse
        buffer->save(&asset->vd.worldVb, 1, 4);

        // saveGfxWorldVertexLayerData
        if (asset->vld.data) {
            // no align for char
            buffer->saveArray(asset->vld.data, asset->vertexLayerDataSize);
            Utils::Stream::ClearPointer(&dest->vld.data);
        }

        buffer->save(&asset->vld.layerVb, 1, 4);

        if (asset->indices) {
            buffer->align(Utils::Stream::ALIGN_2);
            buffer->saveArray(asset->indices, asset->indexCount);
            Utils::Stream::ClearPointer(&dest->indices);
        }
    }

    void IGfxWorld::saveGfxLightGrid(Game::GfxLightGrid* asset, Game::GfxLightGrid* dest, Components::ZoneBuilder::Zone* builder) {
        AssertSize(Game::GfxLightGrid, 56);

        Utils::Stream* buffer = builder->getBuffer();

        if (asset->rowDataStart)
        {
            buffer->align(Utils::Stream::ALIGN_2);
            buffer->saveArray(asset->rowDataStart, (asset->maxs[asset->rowAxis] - asset->mins[asset->rowAxis]) + 2);
            Utils::Stream::ClearPointer(&dest->rowDataStart);
        }

        if (asset->rawRowData)
        {
            // no align for char
            buffer->saveArray(asset->rawRowData, asset->rawRowDataSize);
            Utils::Stream::ClearPointer(&dest->rawRowData);
        }

        if (asset->entries)
        {
            buffer->align(Utils::Stream::ALIGN_4);
            buffer->saveArray(asset->entries, asset->entryCount);
            Utils::Stream::ClearPointer(&dest->entries);
        }

        if (asset->colors)
        {
            buffer->align(Utils::Stream::ALIGN_4);
            buffer->saveArray(asset->colors, asset->colorCount);
            Utils::Stream::ClearPointer(&dest->colors);
        }
    }

    void IGfxWorld::savesunflare_t(Game::sunflare_t* asset, Game::sunflare_t* dest, Components::ZoneBuilder::Zone* builder) {
        AssertSize(Game::sunflare_t, 96);

        Utils::Stream* buffer = builder->getBuffer();

        if(asset->spriteMaterial) {
            asset->spriteMaterial = builder->requireAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->spriteMaterial->name).material;
        }

        if(asset->flareMaterial) {
            asset->flareMaterial = builder->requireAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->flareMaterial->name).material;
        }
    }

    void IGfxWorld::saveGfxWorldDpvsStatic(Game::GfxWorldDpvsStatic* asset, Game::GfxWorldDpvsStatic* dest, int planeCount, Components::ZoneBuilder::Zone* builder) {
        AssertSize(Game::GfxWorldDpvsStatic, 108);

        Utils::Stream* buffer = builder->getBuffer();
    }

    void IGfxWorld::saveGfxWorldDpvsDynamic(Game::GfxWorldDpvsDynamic* asset, Game::GfxWorldDpvsDynamic* dest, int planeCount, Components::ZoneBuilder::Zone* builder) {
        AssertSize(Game::GfxWorldDpvsDynamic, 48);

        Utils::Stream* buffer = builder->getBuffer();

        buffer->pushBlock(Game::XFILE_BLOCK_RUNTIME);

        if (asset->dynEntCellBits[0])
        {
            buffer->align(Utils::Stream::ALIGN_4);
            buffer->saveArray(asset->dynEntCellBits[0], planeCount);
            Utils::Stream::ClearPointer(&dest->dynEntCellBits[0]);
        }

        if (asset->dynEntCellBits[1])
        {
            buffer->align(Utils::Stream::ALIGN_4);
            buffer->saveArray(asset->dynEntCellBits[1], planeCount);
            Utils::Stream::ClearPointer(&dest->dynEntCellBits[1]);
        }

        // this covers [0][0], [1][0], [0][1], [1][1], [0][2], [1][2]
        for(char i = 0; i < 3; i++)
        {
            for (char j = 0; j < 2; j++)
            {
                if(asset->dynEntVisData[j][i])
                {
                    buffer->align(Utils::Stream::ALIGN_16);
                    buffer->save(asset->dynEntVisData[j][i], 32, asset->dynEntClientWordCount[j]);
                    Utils::Stream::ClearPointer(&dest->dynEntVisData[j][i]);
                }
            }
        }

        buffer->popBlock();
    }

    void IGfxWorld::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
    {
        AssertSize(Game::GfxWorld, 0x274);

        Utils::Stream* buffer = builder->getBuffer();
        Game::GfxWorld* asset = header.gfxMap;
        Game::GfxWorld* dest = buffer->dest<Game::GfxWorld>();
        buffer->save(asset);

        buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

        if (asset->name)
        {
            buffer->saveString(builder->getAssetName(this->getType(), asset->name));
            Utils::Stream::ClearPointer(&dest->name);
        }

        if (asset->baseName)
        {
            buffer->saveString(asset->baseName);
            Utils::Stream::ClearPointer(&dest->baseName);
        }

        if (asset->skies)
        {
            AssertSize(Game::GfxSky, 16);

			buffer->align(Utils::Stream::ALIGN_4);
			Game::GfxSky* destSkyTable = buffer->dest<Game::GfxSky>();
			buffer->saveArray(asset->skies, asset->skyCount);

			for (char i = 0; i < asset->skyCount; ++i)
			{
                Game::GfxSky* destSky = &destSkyTable[i];
                Game::GfxSky* sky = &asset->skies[i];

                if (sky->skyStartSurfs)
                {
                    buffer->align(Utils::Stream::ALIGN_4);
        			buffer->saveArray(sky->skyStartSurfs, sky->skySurfCount);
                    Utils::Stream::ClearPointer(&destSky->skyStartSurfs);
                }

                if(sky->skyImage) {
                    destSky->skyImage = builder->requireAsset(Game::XAssetType::ASSET_TYPE_IMAGE, sky->skyImage->name).image;
                }
            }
            Utils::Stream::ClearPointer(&dest->skies);
        }

        this->saveGfxWorldDpvsPlanes(asset, &asset->dpvsPlanes, &dest->dpvsPlanes, builder);

        uint32_t cellCount = asset->dpvsPlanes.cellCount;

        if (asset->aabbTreeCounts)
        {
            AssertSize(Game::GfxCellTreeCount, 4);

            buffer->align(Utils::Stream::ALIGN_4);
            buffer->saveArray(asset->aabbTreeCounts, cellCount);
            Utils::Stream::ClearPointer(&dest->aabbTreeCounts);
        }
        if (asset->aabbTrees)
        {
            AssertSize(Game::GfxCellTree, 4);

			buffer->align(Utils::Stream::ALIGN_128);
			Game::GfxCellTree* destCellTreeTable = buffer->dest<Game::GfxCellTree>();
			buffer->saveArray(asset->aabbTrees, cellCount);

			for (char i = 0; i < cellCount; ++i)
			{
                Game::GfxCellTree* destCellTree = &destCellTreeTable[i];
                Game::GfxCellTree* cellTree = &asset->aabbTrees[i];

                if (cellTree->aabbTree)
                {
                    AssertSize(Game::GfxAabbTree, 44);

        			buffer->align(Utils::Stream::ALIGN_4);
        			Game::GfxAabbTree* destAabbTreeTable = buffer->dest<Game::GfxAabbTree>();
        			buffer->saveArray(cellTree->aabbTree, asset->aabbTreeCounts[i].aabbTreeCount);

                    // ok this one is based on some assumptions because the actual count is this
                    // *(int *)((char *)&varGfxWorld->aabbTreeCounts->aabbTreeCount + (((char *)varGfxCellTree - (char *)varGfxWorld->aabbTrees) & 0xFFFFFFFC))
                    // which makes no sense
                    // what DOES make sense is using the count from the strucutre

        			for (char j = 0; j < asset->aabbTreeCounts[i].aabbTreeCount; ++j)
        			{
                        Game::GfxAabbTree* destAabbTree = &destAabbTreeTable[j];
                        Game::GfxAabbTree* aabbTree = &cellTree->aabbTree[j];

                        if (aabbTree->smodelIndexes)
                        {
                            if(builder->hasPointer(aabbTree->smodelIndexes)) {
                                destAabbTree->smodelIndexes = builder->getPointer(aabbTree->smodelIndexes);
                            }
                            else
                            {
                                buffer->align(Utils::Stream::ALIGN_2);
                                builder->storePointer(aabbTree->smodelIndexes);

                                buffer->saveArray(aabbTree->smodelIndexes, aabbTree->smodelIndexCount);
                                Utils::Stream::ClearPointer(&destAabbTree->smodelIndexes);
                            }
                        }
                    }
                    Utils::Stream::ClearPointer(&destCellTree->aabbTree);
                }

            }
            Utils::Stream::ClearPointer(&dest->aabbTrees);
        }

        if (asset->cells)
        {
            AssertSize(Game::GfxCell, 40);

			buffer->align(Utils::Stream::ALIGN_4);
			Game::GfxCell* destCellTable = buffer->dest<Game::GfxCell>();
			buffer->saveArray(asset->cells, cellCount);

			for (char i = 0; i < cellCount; ++i)
			{
                Game::GfxCell* destCell = &destCellTable[i];
                Game::GfxCell* cell = &asset->cells[i];

                if (cell->portals)
                {
                    AssertSize(Game::GfxPortal, 60);

        			buffer->align(Utils::Stream::ALIGN_4);
        			Game::GfxPortal* destPortalTable = buffer->dest<Game::GfxPortal>();
        			buffer->saveArray(cell->portals, cell->portalCount);

        			for (char j = 0; j < cell->portalCount; ++j)
        			{
                        Game::GfxPortal* destPortal = &destPortalTable[j];
                        Game::GfxPortal* portal = &cell->portals[j];

                        if (portal->vertices)
                        {
                            buffer->align(Utils::Stream::ALIGN_4);
                            buffer->saveArray(portal->vertices, portal->vertexCount);
                            Utils::Stream::ClearPointer(&destPortal->vertices);
                        }
                    }
                    Utils::Stream::ClearPointer(&destCell->portals);
                }

                if (cell->reflectionProbes)
                {
                    // no align for char
                    buffer->saveArray(cell->reflectionProbes, cell->reflectionProbeCount);
                    Utils::Stream::ClearPointer(&destCell->reflectionProbes);
                }
            }
            Utils::Stream::ClearPointer(&dest->cells);
        }

        this->saveGfxWorldDraw(&asset->worldDraw, &dest->worldDraw, builder);
        this->saveGfxLightGrid(&asset->lightGrid, &dest->lightGrid, builder);

        if (asset->models)
        {
            buffer->align(Utils::Stream::ALIGN_4);
            buffer->saveArray(asset->models, asset->modelCount);
            Utils::Stream::ClearPointer(&dest->aabbTreeCounts);
        }
        if (asset->materialMemory)
        {
            AssertSize(Game::MaterialMemory, 8);

			buffer->align(Utils::Stream::ALIGN_4);
			Game::MaterialMemory* destMaterialMemoryTable = buffer->dest<Game::MaterialMemory>();
			buffer->saveArray(asset->materialMemory, asset->materialMemoryCount);

			for (char i = 0; i < asset->materialMemoryCount; ++i)
			{
                Game::MaterialMemory* destMaterialMemory = &destMaterialMemoryTable[i];
                Game::MaterialMemory* materialMemory = &asset->materialMemory[i];

                if (materialMemory->material)
                {
                    destMaterialMemory->material = builder->requireAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, materialMemory->material->name).material;
                }
            }
            Utils::Stream::ClearPointer(&dest->materialMemory);
        }

        this->savesunflare_t(&asset->sun, &dest->sun, builder);

        if(asset->unknownImage) {
            dest->unknownImage = builder->requireAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->unknownImage->name).image;
        }

        buffer->pushBlock(Game::XFILE_BLOCK_RUNTIME);

        if (asset->cellCasterBits[0])
        {
          buffer->align(Utils::Stream::ALIGN_4);
          buffer->saveArray(asset->cellCasterBits[0], cellCount * ((cellCount + 31) >> 5));
          Utils::Stream::ClearPointer(&dest->cellCasterBits[0]);
        }

        if (asset->cellCasterBits[1])
        {
            buffer->align(Utils::Stream::ALIGN_4);
            buffer->saveArray(asset->cellCasterBits[1], ((cellCount + 31) >> 5));
            Utils::Stream::ClearPointer(&dest->cellCasterBits[1]);
        }

        if (asset->sceneDynModel)
        {
            buffer->align(Utils::Stream::ALIGN_4);
            buffer->saveArray(asset->sceneDynModel, asset->dpvsDyn.dynEntClientCount[0]);
            Utils::Stream::ClearPointer(&dest->sceneDynModel);
        }

        if (asset->sceneDynBrush)
        {
            buffer->align(Utils::Stream::ALIGN_4);
            buffer->saveArray(asset->sceneDynBrush, asset->dpvsDyn.dynEntClientCount[1]);
            Utils::Stream::ClearPointer(&dest->sceneDynBrush);
        }

        if (asset->primaryLightEntityShadowVis)
        {
            buffer->align(Utils::Stream::ALIGN_4);
            buffer->saveArray(asset->primaryLightEntityShadowVis, (asset->unkCount2 + 0x1FFFF - asset->unkCount1) << 15);
            Utils::Stream::ClearPointer(&dest->primaryLightEntityShadowVis);
        }

        if (asset->primaryLightDynEntShadowVis[0])
        {
            buffer->align(Utils::Stream::ALIGN_4);
            buffer->saveArray(asset->primaryLightDynEntShadowVis[0], asset->dpvsDyn.dynEntClientCount[0] * (asset->unkCount2 - 1 - asset->unkCount1));
            Utils::Stream::ClearPointer(&dest->primaryLightDynEntShadowVis[0]);
        }

        if (asset->primaryLightDynEntShadowVis[1])
        {
            buffer->align(Utils::Stream::ALIGN_4);
            buffer->saveArray(asset->primaryLightDynEntShadowVis[1], asset->dpvsDyn.dynEntClientCount[1] * (asset->unkCount2 - 1 - asset->unkCount1));
            Utils::Stream::ClearPointer(&dest->primaryLightDynEntShadowVis[1]);
        }

        if (asset->primaryLightForModelDynEnt)
        {
            // no align cause char
            buffer->saveArray(asset->primaryLightForModelDynEnt, asset->dpvsDyn.dynEntClientCount[0]);
            Utils::Stream::ClearPointer(&dest->primaryLightForModelDynEnt);
        }

        buffer->popBlock();

        if (asset->shadowGeom)
        {
            AssertSize(Game::GfxShadowGeometry, 12);

			buffer->align(Utils::Stream::ALIGN_4);
			Game::GfxShadowGeometry* destShadowGeometryTable = buffer->dest<Game::GfxShadowGeometry>();
			buffer->saveArray(asset->shadowGeom, asset->unkCount2);

			for (char i = 0; i < asset->unkCount2; ++i)
			{
                Game::GfxShadowGeometry* destShadowGeometry = &destShadowGeometryTable[i];
                Game::GfxShadowGeometry* shadowGeometry = &asset->shadowGeom[i];

                if (shadowGeometry->sortedSurfIndex)
                {
                    buffer->align(Utils::Stream::ALIGN_2);
                    buffer->saveArray(shadowGeometry->sortedSurfIndex, shadowGeometry->surfaceCount);
                    Utils::Stream::ClearPointer(&destShadowGeometry->sortedSurfIndex);
                }

                if (shadowGeometry->smodelIndex)
                {
                    buffer->align(Utils::Stream::ALIGN_2);
                    buffer->saveArray(shadowGeometry->smodelIndex, shadowGeometry->smodelCount);
                    Utils::Stream::ClearPointer(&destShadowGeometry->smodelIndex);
                }
            }
            Utils::Stream::ClearPointer(&dest->shadowGeom);
        }

        if (asset->lightRegion)
        {
            AssertSize(Game::GfxLightRegion, 8);

			buffer->align(Utils::Stream::ALIGN_4);
			Game::GfxLightRegion* destLightRegionTable = buffer->dest<Game::GfxLightRegion>();
			buffer->saveArray(asset->lightRegion, asset->unkCount2);

			for (char i = 0; i < asset->unkCount2; ++i)
			{
                Game::GfxLightRegion* destLightRegion = &destLightRegionTable[i];
                Game::GfxLightRegion* lightRegion = &asset->lightRegion[i];

                if (lightRegion->hulls)
                {
                    AssertSize(Game::GfxLightRegionHull, 80);

        			buffer->align(Utils::Stream::ALIGN_4);
        			Game::GfxLightRegionHull* destLightRegionHullTable = buffer->dest<Game::GfxLightRegionHull>();
        			buffer->saveArray(lightRegion->hulls, lightRegion->hullCount);

        			for (char j = 0; j < lightRegion->hullCount; ++j)
        			{
                        Game::GfxLightRegionHull* destLightRegionHull = &destLightRegionHullTable[j];
                        Game::GfxLightRegionHull* lightRegionHull = &lightRegion->hulls[j];

                        if (lightRegionHull->axis)
                        {
                            AssertSize(Game::GfxLightRegionAxis, 20);

                            buffer->align(Utils::Stream::ALIGN_4);
                            buffer->saveArray(lightRegionHull->axis, lightRegionHull->axisCount);
                            Utils::Stream::ClearPointer(&destLightRegionHull->axis);
                        }

                    }
                    Utils::Stream::ClearPointer(&destLightRegion->hulls);
                }

            }
            Utils::Stream::ClearPointer(&dest->lightRegion);
        }

        this->saveGfxWorldDpvsStatic(&asset->dpvs, &dest->dpvs, asset->dpvsPlanes.cellCount, builder);
        this->saveGfxWorldDpvsDynamic(&asset->dpvsDyn, &dest->dpvsDyn, asset->dpvsPlanes.cellCount, builder);

        if (asset->heroOnlyLight)
        {
            // no assert cause we use save manually here
            buffer->align(Utils::Stream::ALIGN_4);
            buffer->save(asset->heroOnlyLight, 56, asset->heroOnlyLightCount);
            Utils::Stream::ClearPointer(&dest->heroOnlyLight);
        }

        buffer->popBlock();
    }
}
