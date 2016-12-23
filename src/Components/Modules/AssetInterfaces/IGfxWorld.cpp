#include <STDInclude.hpp>

namespace Assets
{
	void IGfxWorld::load(Game::XAssetHeader* /*header*/, std::string name, Components::ZoneBuilder::Zone* /*builder*/)
	{
		Game::GfxWorld* map = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_GFX_MAP, name.data()).gfxWorld;
		if (map) return;

		Components::Logger::Error("Missing GfxMap %s... you can't make them yet you idiot.", name.data());
	}

	void IGfxWorld::mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::GfxWorld* asset = header.gfxWorld;

		if (asset->worldDraw.reflectionImages)
		{
			for (unsigned int i = 0; i < asset->worldDraw.reflectionProbeCount; ++i)
			{
				builder->loadAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->worldDraw.reflectionImages[i]);
			}
		}

		if (asset->worldDraw.lightmaps)
		{
			for (int i = 0; i < asset->worldDraw.lightmapCount; ++i)
			{
				if (asset->worldDraw.lightmaps[i].primary)
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->worldDraw.lightmaps[i].primary);
				}

				if (asset->worldDraw.lightmaps[i].secondary)
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->worldDraw.lightmaps[i].secondary);
				}
			}
		}

		if (asset->worldDraw.skyImage)
		{
			builder->loadAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->worldDraw.skyImage);
		}

		if (asset->worldDraw.outdoorImage)
		{
			builder->loadAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->worldDraw.outdoorImage);
		}

		if (asset->sun.spriteMaterial)
		{
			builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->sun.spriteMaterial);
		}

		if (asset->sun.flareMaterial)
		{
			builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->sun.flareMaterial);
		}

		if (asset->skies)
		{
			for (unsigned int i = 0; i < asset->skyCount; ++i)
			{
				if (asset->skies[i].skyImage)
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->skies[i].skyImage);
				}
			}
		}

		if (asset->materialMemory)
		{
			for (int i = 0; i < asset->materialMemoryCount; ++i)
			{
				if (asset->materialMemory[i].material)
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->materialMemory[i].material);
				}
			}
		}

		if (asset->unknownImage)
		{
			builder->loadAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->unknownImage);
		}

		if (asset->dpvs.surfaces)
		{
			for (int i = 0; i < asset->dpvsSurfaceCount; ++i)
			{
				if (asset->dpvs.surfaces[i].material)
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->dpvs.surfaces[i].material);
				}
			}
		}

		if (asset->dpvs.smodelDrawInsts)
		{
			for (unsigned int i = 0; i < asset->dpvs.smodelCount; ++i)
			{
				if (asset->dpvs.smodelDrawInsts[i].model)
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL, asset->dpvs.smodelDrawInsts[i].model);
				}
			}
		}
	}

	void IGfxWorld::saveGfxWorldDpvsPlanes(Game::GfxWorld* world, Game::GfxWorldDpvsPlanes* asset, Game::GfxWorldDpvsPlanes* dest, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::GfxWorldDpvsPlanes, 16);

		Utils::Stream* buffer = builder->getBuffer();
		SaveLogEnter("GfxWorldDpvsPlanes");

		if (asset->planes)
		{
			if (builder->hasPointer(asset->planes))
			{
				dest->planes = builder->getPointer(asset->planes);
			}
			else
			{
				AssertSize(Game::cplane_t, 20);

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
			buffer->save(asset->sceneEntCellBits, 1, asset->cellCount << 11);
			Utils::Stream::ClearPointer(&dest->sceneEntCellBits);
		}

		buffer->popBlock();
		SaveLogExit();
	}

	void IGfxWorld::saveGfxWorldDraw(Game::GfxWorldDraw* asset, Game::GfxWorldDraw* dest, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::GfxWorldDraw, 72);
		SaveLogEnter("GfxWorldDraw");

		Utils::Stream* buffer = builder->getBuffer();

		if (asset->reflectionImages)
		{
			buffer->align(Utils::Stream::ALIGN_4);

			Game::GfxImage** imageDest = buffer->dest<Game::GfxImage*>();
			buffer->saveArray(asset->reflectionImages, asset->reflectionProbeCount);

			for (unsigned int i = 0; i < asset->reflectionProbeCount; ++i)
			{
				imageDest[i] = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->reflectionImages[i]).image;
			}

			Utils::Stream::ClearPointer(&dest->reflectionImages);
		}

		if (asset->reflectionProbes)
		{
			AssertSize(Game::GfxReflectionProbe, 12);

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->reflectionProbes, asset->reflectionProbeCount);
			Utils::Stream::ClearPointer(&dest->reflectionProbes);
		}

		buffer->pushBlock(Game::XFILE_BLOCK_RUNTIME);

		if (asset->reflectionProbeTextures)
		{
			AssertSize(Game::GfxRawTexture, 4);

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->reflectionProbeTextures, asset->reflectionProbeCount);
			Utils::Stream::ClearPointer(&dest->reflectionProbeTextures);
		}

		buffer->popBlock();

		if (asset->lightmaps)
		{
			AssertSize(Game::GfxLightmapArray, 8);

			buffer->align(Utils::Stream::ALIGN_4);

			Game::GfxLightmapArray* lightmapArrayDestTable = buffer->dest<Game::GfxLightmapArray>();
			buffer->saveArray(asset->lightmaps, asset->lightmapCount);

			for (int i = 0; i < asset->lightmapCount; ++i)
			{
				Game::GfxLightmapArray* lightmapArrayDest = &lightmapArrayDestTable[i];
				Game::GfxLightmapArray* lightmapArray = &asset->lightmaps[i];

				if (lightmapArray->primary)
				{
					lightmapArrayDest->primary = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_IMAGE, lightmapArray->primary).image;
				}

				if (lightmapArray->secondary)
				{
					lightmapArrayDest->secondary = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_IMAGE, lightmapArray->secondary).image;
				}
			}

			Utils::Stream::ClearPointer(&dest->lightmaps);
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

		if (asset->skyImage)
		{
			dest->skyImage = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->skyImage).image;
		}

		if (asset->outdoorImage)
		{
			dest->outdoorImage = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->outdoorImage).image;
		}

		// saveGfxWorldVertexData
		{
			if (asset->vd.vertices)
			{
				AssertSize(Game::GfxWorldVertex, 44);

				buffer->align(Utils::Stream::ALIGN_4);
				buffer->saveArray(asset->vd.vertices, asset->vertexCount);
				Utils::Stream::ClearPointer(&dest->vd.vertices);
			}
		}

		// saveGfxWorldVertexLayerData
		{
			if (asset->vld.data)
			{
				// no align for char
				buffer->saveArray(asset->vld.data, asset->vertexLayerDataSize);
				Utils::Stream::ClearPointer(&dest->vld.data);
			}
		}

		if (asset->indices)
		{
			buffer->align(Utils::Stream::ALIGN_2);
			buffer->saveArray(asset->indices, asset->indexCount);
			Utils::Stream::ClearPointer(&dest->indices);
		}

		SaveLogExit();
	}

	void IGfxWorld::saveGfxLightGrid(Game::GfxLightGrid* asset, Game::GfxLightGrid* dest, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::GfxLightGrid, 56);

		Utils::Stream* buffer = builder->getBuffer();
		SaveLogEnter("GfxLightGrid");

		if (asset->rowDataStart)
		{
			buffer->align(Utils::Stream::ALIGN_2);
			buffer->saveArray(asset->rowDataStart, (asset->maxs[asset->rowAxis] - asset->mins[asset->rowAxis]) + 1);
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
			AssertSize(Game::GfxLightGridEntry, 4);

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->entries, asset->entryCount);
			Utils::Stream::ClearPointer(&dest->entries);
		}

		if (asset->colors)
		{
			AssertSize(Game::GfxLightGridColors, 168);

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->colors, asset->colorCount);
			Utils::Stream::ClearPointer(&dest->colors);
		}

		SaveLogExit();
	}

	void IGfxWorld::savesunflare_t(Game::sunflare_t* asset, Game::sunflare_t* dest, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::sunflare_t, 96);
		SaveLogEnter("sunflare_t");

		if (asset->spriteMaterial)
		{
			dest->spriteMaterial = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->spriteMaterial).material;
		}

		if (asset->flareMaterial)
		{
			dest->flareMaterial = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->flareMaterial).material;
		}

		SaveLogExit();
	}

	void IGfxWorld::saveGfxWorldDpvsStatic(Game::GfxWorld* world, Game::GfxWorldDpvsStatic* asset, Game::GfxWorldDpvsStatic* dest, int /*planeCount*/, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::GfxWorldDpvsStatic, 108);

		Utils::Stream* buffer = builder->getBuffer();
		SaveLogEnter("GfxWorldDpvsStatic");

		buffer->pushBlock(Game::XFILE_BLOCK_RUNTIME);

		for (int i = 0; i < 3; ++i)
		{
			if (asset->smodelVisData[i])
			{
				buffer->saveArray(asset->smodelVisData[i], asset->smodelCount);
				Utils::Stream::ClearPointer(&dest->smodelVisData[i]);
			}
		}

		for (int i = 0; i < 3; ++i)
		{
			if (asset->surfaceVisData[i])
			{
				buffer->saveArray(asset->surfaceVisData[i], asset->staticSurfaceCount);
				Utils::Stream::ClearPointer(&dest->surfaceVisData[i]);
			}
		}

		buffer->popBlock();

		if (asset->sortedSurfIndex)
		{
			buffer->align(Utils::Stream::ALIGN_2);
			buffer->saveArray(asset->sortedSurfIndex, asset->staticSurfaceCount + asset->litSurfsBegin);
			Utils::Stream::ClearPointer(&dest->sortedSurfIndex);
		}

		if (asset->smodelInsts)
		{
			AssertSize(Game::GfxStaticModelInst, 36);

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->smodelInsts, asset->smodelCount);
			Utils::Stream::ClearPointer(&dest->smodelInsts);
		}

		if (asset->surfaces)
		{
			AssertSize(Game::GfxSurface, 24);

			buffer->align(Utils::Stream::ALIGN_4);
			Game::GfxSurface* destSurfaceTable = buffer->dest<Game::GfxSurface>();
			buffer->saveArray(asset->surfaces, world->dpvsSurfaceCount);

			for (int i = 0; i < world->dpvsSurfaceCount; ++i)
			{
				Game::GfxSurface* surface = &asset->surfaces[i];
				Game::GfxSurface* destSurface = &destSurfaceTable[i];

				if (surface->material)
				{
					destSurface->material = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, surface->material).material;
				}
			}

			Utils::Stream::ClearPointer(&dest->surfaces);
		}

		if (asset->surfacesBounds)
		{
			AssertSize(Game::GfxSurfaceBounds, 24);

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->surfacesBounds, world->dpvsSurfaceCount);
			Utils::Stream::ClearPointer(&dest->surfacesBounds);
		}

		if (asset->smodelDrawInsts)
		{
			AssertSize(Game::GfxStaticModelDrawInst, 76);

			buffer->align(Utils::Stream::ALIGN_4);
			Game::GfxStaticModelDrawInst* destModelTable = buffer->dest<Game::GfxStaticModelDrawInst>();
			buffer->saveArray(asset->smodelDrawInsts, asset->smodelCount);

			for (unsigned int i = 0; i < asset->smodelCount; ++i)
			{
				Game::GfxStaticModelDrawInst* model = &asset->smodelDrawInsts[i];
				Game::GfxStaticModelDrawInst* destModel = &destModelTable[i];

				if (model->model)
				{
					destModel->model = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_XMODEL, model->model).model;
				}
			}

			Utils::Stream::ClearPointer(&dest->smodelDrawInsts);
		}

		buffer->pushBlock(Game::XFILE_BLOCK_RUNTIME);

		if (asset->surfaceMaterials)
		{
			AssertSize(Game::GfxDrawSurf, 8);

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->surfaceMaterials, world->dpvsSurfaceCount);
			Utils::Stream::ClearPointer(&dest->surfaceMaterials);
		}

		if (asset->surfaceCastsSunShadow)
		{
			AssertSize(Game::GfxDrawSurf, 8);

			buffer->align(Utils::Stream::ALIGN_128);
			buffer->save(asset->surfaceCastsSunShadow, 4, asset->sunShadowCount);
			Utils::Stream::ClearPointer(&dest->surfaceCastsSunShadow);
		}

		buffer->popBlock();
		SaveLogExit();
	}

	void IGfxWorld::saveGfxWorldDpvsDynamic(Game::GfxWorldDpvsDynamic* asset, Game::GfxWorldDpvsDynamic* dest, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::GfxWorldDpvsDynamic, 48);

		Utils::Stream* buffer = builder->getBuffer();
		SaveLogEnter("GfxWorldDpvsDynamic");

		buffer->pushBlock(Game::XFILE_BLOCK_RUNTIME);

		if (asset->dynEntCellBits[0])
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->dynEntCellBits[0], asset->dynEntClientWordCount[0]);
			Utils::Stream::ClearPointer(&dest->dynEntCellBits[0]);
		}

		if (asset->dynEntCellBits[1])
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->dynEntCellBits[1], asset->dynEntClientWordCount[1]);
			Utils::Stream::ClearPointer(&dest->dynEntCellBits[1]);
		}

		// this covers [0][0], [1][0], [0][1], [1][1], [0][2], [1][2]
		for (char i = 0; i < 3; ++i)
		{
			for (char j = 0; j < 2; ++j)
			{
				if (asset->dynEntVisData[j][i])
				{
					buffer->align(Utils::Stream::ALIGN_16);
					buffer->save(asset->dynEntVisData[j][i], 32, asset->dynEntClientWordCount[j]);
					Utils::Stream::ClearPointer(&dest->dynEntVisData[j][i]);
				}
			}
		}

		buffer->popBlock();
        SaveLogExit();
	}

	void IGfxWorld::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::GfxWorld, 628);

		Utils::Stream* buffer = builder->getBuffer();
		SaveLogEnter("GfxWorld");

		Game::GfxWorld* asset = header.gfxWorld;
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

			for (unsigned int i = 0; i < asset->skyCount; ++i)
			{
				Game::GfxSky* destSky = &destSkyTable[i];
				Game::GfxSky* sky = &asset->skies[i];

				if (sky->skyStartSurfs)
				{
					buffer->align(Utils::Stream::ALIGN_4);
					buffer->saveArray(sky->skyStartSurfs, sky->skySurfCount);
					Utils::Stream::ClearPointer(&destSky->skyStartSurfs);
				}

				if (sky->skyImage)
				{
					destSky->skyImage = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_IMAGE, sky->skyImage).image;
				}
			}

			Utils::Stream::ClearPointer(&dest->skies);
		}

		this->saveGfxWorldDpvsPlanes(asset, &asset->dpvsPlanes, &dest->dpvsPlanes, builder);

		int cellCount = asset->dpvsPlanes.cellCount;

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

			for (int i = 0; i < cellCount; ++i)
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
					// what DOES make sense is using the count from the structure

					for (int j = 0; j < asset->aabbTreeCounts[i].aabbTreeCount; ++j)
					{
						Game::GfxAabbTree* destAabbTree = &destAabbTreeTable[j];
						Game::GfxAabbTree* aabbTree = &cellTree->aabbTree[j];

						if (aabbTree->smodelIndexes)
						{
							if (builder->hasPointer(aabbTree->smodelIndexes))
							{
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

			for (int i = 0; i < cellCount; ++i)
			{
				Game::GfxCell* destCell = &destCellTable[i];
				Game::GfxCell* cell = &asset->cells[i];

				if (cell->portals)
				{
					AssertSize(Game::GfxPortal, 60);

					buffer->align(Utils::Stream::ALIGN_4);
					Game::GfxPortal* destPortalTable = buffer->dest<Game::GfxPortal>();
					buffer->saveArray(cell->portals, cell->portalCount);

					for (int j = 0; j < cell->portalCount; ++j)
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
			AssertSize(Game::GfxBrushModel, 60);

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->models, asset->modelCount);
			Utils::Stream::ClearPointer(&dest->models);
		}

		if (asset->materialMemory)
		{
			AssertSize(Game::MaterialMemory, 8);

			buffer->align(Utils::Stream::ALIGN_4);
			Game::MaterialMemory* destMaterialMemoryTable = buffer->dest<Game::MaterialMemory>();
			buffer->saveArray(asset->materialMemory, asset->materialMemoryCount);

			for (int i = 0; i < asset->materialMemoryCount; ++i)
			{
				Game::MaterialMemory* destMaterialMemory = &destMaterialMemoryTable[i];
				Game::MaterialMemory* materialMemory = &asset->materialMemory[i];

				if (materialMemory->material)
				{
					destMaterialMemory->material = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, materialMemory->material).material;
				}
			}

			Utils::Stream::ClearPointer(&dest->materialMemory);
		}

		this->savesunflare_t(&asset->sun, &dest->sun, builder);

		if (asset->unknownImage)
		{
			dest->unknownImage = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->unknownImage).image;
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
			AssertSize(Game::GfxSceneDynModel, 6);

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->sceneDynModel, asset->dpvsDyn.dynEntClientCount[0]);
			Utils::Stream::ClearPointer(&dest->sceneDynModel);
		}

		if (asset->sceneDynBrush)
		{
			AssertSize(Game::GfxSceneDynBrush, 4);

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->sceneDynBrush, asset->dpvsDyn.dynEntClientCount[1]);
			Utils::Stream::ClearPointer(&dest->sceneDynBrush);
		}

		if (asset->primaryLightEntityShadowVis)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->save(asset->primaryLightEntityShadowVis, 1, (asset->unkCount2 + 0x1FFFF - asset->unkCount1) << 15);
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

			for (int i = 0; i < asset->unkCount2; ++i)
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

			for (int i = 0; i < asset->unkCount2; ++i)
			{
				Game::GfxLightRegion* destLightRegion = &destLightRegionTable[i];
				Game::GfxLightRegion* lightRegion = &asset->lightRegion[i];

				if (lightRegion->hulls)
				{
					AssertSize(Game::GfxLightRegionHull, 80);

					buffer->align(Utils::Stream::ALIGN_4);
					Game::GfxLightRegionHull* destLightRegionHullTable = buffer->dest<Game::GfxLightRegionHull>();
					buffer->saveArray(lightRegion->hulls, lightRegion->hullCount);

					for (unsigned int j = 0; j < lightRegion->hullCount; ++j)
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

		this->saveGfxWorldDpvsStatic(asset, &asset->dpvs, &dest->dpvs, asset->dpvsPlanes.cellCount, builder);
		this->saveGfxWorldDpvsDynamic(&asset->dpvsDyn, &dest->dpvsDyn, builder);

		if (asset->heroOnlyLight)
		{
			// no assert cause we use save manually here
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->save(asset->heroOnlyLight, 56, asset->heroOnlyLightCount);
			Utils::Stream::ClearPointer(&dest->heroOnlyLight);
		}

		buffer->popBlock();
		SaveLogExit();
	}
}
