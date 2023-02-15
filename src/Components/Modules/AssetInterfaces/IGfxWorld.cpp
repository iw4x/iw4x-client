#include <STDInclude.hpp>
#include "IGfxWorld.hpp"

namespace Assets
{
	void IGfxWorld::load(Game::XAssetHeader* header, const std::string& _name, Components::ZoneBuilder::Zone* builder)
	{
		header->gfxWorld = builder->getIW4OfApi()->read<Game::GfxWorld>(Game::XAssetType::ASSET_TYPE_GFXWORLD, _name);
	}

	void IGfxWorld::mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		auto* asset = header.gfxWorld;
		if (asset->draw.reflectionProbes)
		{
			for (unsigned int i = 0; i < asset->draw.reflectionProbeCount; ++i)
			{
				builder->loadAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->draw.reflectionProbes[i]);
			}
		}

		if (asset->draw.lightmaps)
		{
			for (auto i = 0; i < asset->draw.lightmapCount; ++i)
			{
				if (asset->draw.lightmaps[i].primary)
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->draw.lightmaps[i].primary);
				}

				if (asset->draw.lightmaps[i].secondary)
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->draw.lightmaps[i].secondary);
				}
			}
		}

		if (asset->draw.lightmapOverridePrimary)
		{
			builder->loadAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->draw.lightmapOverridePrimary);
		}

		if (asset->draw.lightmapOverrideSecondary)
		{
			builder->loadAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->draw.lightmapOverrideSecondary);
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
			for (int i = 0; i < asset->skyCount; ++i)
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

		if (asset->outdoorImage)
		{
			builder->loadAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->outdoorImage);
		}

		if (asset->dpvs.surfaces)
		{
			for (unsigned int i = 0; i < asset->surfaceCount; ++i)
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
				AssertSize(Game::cplane_s, 20);

				buffer->align(Utils::Stream::ALIGN_4);

				for (int i = 0; i < world->planeCount; ++i)
				{
					builder->storePointer(&asset->planes[i]);
					buffer->save(&asset->planes[i]);
				}

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

		if (asset->reflectionProbes)
		{
			buffer->align(Utils::Stream::ALIGN_4);

			auto** imageDest = buffer->dest<Game::GfxImage*>();
			buffer->saveArray(asset->reflectionProbes, asset->reflectionProbeCount);

			for (unsigned int i = 0; i < asset->reflectionProbeCount; ++i)
			{
				if (asset->reflectionProbes[i])
				{
					imageDest[i] = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->reflectionProbes[i]).image;
				}
			}

			Utils::Stream::ClearPointer(&dest->reflectionProbes);
		}

		if (asset->reflectionProbeOrigins)
		{
			AssertSize(Game::GfxReflectionProbe, 12);
			SaveLogEnter("GfxReflectionProbe");

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->reflectionProbeOrigins, asset->reflectionProbeCount);
			Utils::Stream::ClearPointer(&dest->reflectionProbeOrigins);

			SaveLogExit();
		}

		buffer->pushBlock(Game::XFILE_BLOCK_RUNTIME);

		if (asset->reflectionProbeTextures)
		{
			AssertSize(Game::GfxTexture, 4);
			SaveLogEnter("GfxRawTexture");

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->reflectionProbeTextures, asset->reflectionProbeCount);
			Utils::Stream::ClearPointer(&dest->reflectionProbeTextures);

			SaveLogExit();
		}

		buffer->popBlock();

		if (asset->lightmaps)
		{
			AssertSize(Game::GfxLightmapArray, 8);
			SaveLogEnter("GfxLightmapArray");

			buffer->align(Utils::Stream::ALIGN_4);

			auto* lightmapArrayDestTable = buffer->dest<Game::GfxLightmapArray>();
			buffer->saveArray(asset->lightmaps, asset->lightmapCount);

			for (int i = 0; i < asset->lightmapCount; ++i)
			{
				auto* lightmapArrayDest = &lightmapArrayDestTable[i];
				auto* lightmapArray = &asset->lightmaps[i];

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
			SaveLogExit();
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

		if (asset->lightmapOverridePrimary)
		{
			dest->lightmapOverridePrimary = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->lightmapOverridePrimary).image;
		}

		if (asset->lightmapOverrideSecondary)
		{
			dest->lightmapOverrideSecondary = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->lightmapOverrideSecondary).image;
		}

		// saveGfxWorldVertexData
		{
			if (asset->vd.vertices)
			{
				AssertSize(Game::GfxWorldVertex, 44);
				SaveLogEnter("GfxWorldVertex");

				buffer->align(Utils::Stream::ALIGN_4);
				buffer->saveArray(asset->vd.vertices, asset->vertexCount);
				Utils::Stream::ClearPointer(&dest->vd.vertices);

				SaveLogExit();
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
			SaveLogEnter("GfxLightGridEntry");

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->entries, asset->entryCount);
			Utils::Stream::ClearPointer(&dest->entries);

			SaveLogExit();
		}

		if (asset->colors)
		{
			AssertSize(Game::GfxLightGridColors, 168);
			SaveLogEnter("GfxLightGridColors");

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->colors, asset->colorCount);
			Utils::Stream::ClearPointer(&dest->colors);

			SaveLogExit();
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
			buffer->saveArray(asset->sortedSurfIndex, asset->staticSurfaceCount + asset->staticSurfaceCountNoDecal);
			Utils::Stream::ClearPointer(&dest->sortedSurfIndex);
		}

		if (asset->smodelInsts)
		{
			AssertSize(Game::GfxStaticModelInst, 36);
			SaveLogEnter("GfxStaticModelInst");

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->smodelInsts, asset->smodelCount);
			Utils::Stream::ClearPointer(&dest->smodelInsts);

			SaveLogExit();
		}

		if (asset->surfaces)
		{
			AssertSize(Game::GfxSurface, 24);
			SaveLogEnter("GfxSurface");

			buffer->align(Utils::Stream::ALIGN_4);
			auto* destSurfaceTable = buffer->dest<Game::GfxSurface>();
			buffer->saveArray(asset->surfaces, world->surfaceCount);

			for (unsigned int i = 0; i < world->surfaceCount; ++i)
			{
				auto* surface = &asset->surfaces[i];
				auto* destSurface = &destSurfaceTable[i];

				if (surface->material)
				{
					destSurface->material = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, surface->material).material;
				}
			}

			Utils::Stream::ClearPointer(&dest->surfaces);
			SaveLogExit();
		}

		if (asset->surfacesBounds)
		{
			AssertSize(Game::GfxSurfaceBounds, 24);
			SaveLogEnter("GfxSurfaceBounds");

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->surfacesBounds, world->surfaceCount);
			Utils::Stream::ClearPointer(&dest->surfacesBounds);

			SaveLogExit();
		}

		if (asset->smodelDrawInsts)
		{
			AssertSize(Game::GfxStaticModelDrawInst, 76);
			SaveLogEnter("GfxStaticModelDrawInst");

			buffer->align(Utils::Stream::ALIGN_4);
			auto* destModelTable = buffer->dest<Game::GfxStaticModelDrawInst>();
			buffer->saveArray(asset->smodelDrawInsts, asset->smodelCount);

			for (unsigned int i = 0; i < asset->smodelCount; ++i)
			{
				auto* model = &asset->smodelDrawInsts[i];
				auto* destModel = &destModelTable[i];

				if (model->model)
				{
					destModel->model = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_XMODEL, model->model).model;
				}
			}

			Utils::Stream::ClearPointer(&dest->smodelDrawInsts);
			SaveLogExit();
		}

		buffer->pushBlock(Game::XFILE_BLOCK_RUNTIME);

		if (asset->surfaceMaterials)
		{
			AssertSize(Game::GfxDrawSurf, 8);
			SaveLogEnter("GfxDrawSurf");

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->surfaceMaterials, world->surfaceCount);
			Utils::Stream::ClearPointer(&dest->surfaceMaterials);

			SaveLogExit();
		}

		if (asset->surfaceCastsSunShadow)
		{
			AssertSize(Game::GfxDrawSurf, 8);
			SaveLogEnter("GfxDrawSurf");

			buffer->align(Utils::Stream::ALIGN_128);
			buffer->save(asset->surfaceCastsSunShadow, 4, asset->surfaceVisDataCount);
			Utils::Stream::ClearPointer(&dest->surfaceCastsSunShadow);

			SaveLogExit();
		}

		buffer->popBlock();
		SaveLogExit();
	}

	void IGfxWorld::saveGfxWorldDpvsDynamic(Game::GfxWorldDpvsDynamic* asset, Game::GfxWorldDpvsDynamic* dest, int cellCount, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::GfxWorldDpvsDynamic, 48);

		Utils::Stream* buffer = builder->getBuffer();
		SaveLogEnter("GfxWorldDpvsDynamic");

		buffer->pushBlock(Game::XFILE_BLOCK_RUNTIME);

		if (asset->dynEntCellBits[0])
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->save(asset->dynEntCellBits[0], 4, asset->dynEntClientWordCount[0] * cellCount);
			Utils::Stream::ClearPointer(&dest->dynEntCellBits[0]);
		}

		if (asset->dynEntCellBits[1])
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->save(asset->dynEntCellBits[1], 4, asset->dynEntClientWordCount[1] * cellCount);
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

		auto* asset = header.gfxWorld;
		auto* dest = buffer->dest<Game::GfxWorld>();
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

		//buffer->setPointerAssertion(true);

		if (asset->skies)
		{
			AssertSize(Game::GfxSky, 16);
			SaveLogEnter("GfxSky");

			buffer->align(Utils::Stream::ALIGN_4);
			auto* destSkyTable = buffer->dest<Game::GfxSky>();
			buffer->saveArray(asset->skies, asset->skyCount);

			for (int i = 0; i < asset->skyCount; ++i)
			{
				auto* destSky = &destSkyTable[i];
				auto* sky = &asset->skies[i];

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
			SaveLogExit();
		}

		this->saveGfxWorldDpvsPlanes(asset, &asset->dpvsPlanes, &dest->dpvsPlanes, builder);

		int cellCount = asset->dpvsPlanes.cellCount;

		if (asset->aabbTreeCounts)
		{
			AssertSize(Game::GfxCellTreeCount, 4);
			SaveLogEnter("GfxCellTreeCount");

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->aabbTreeCounts, cellCount);
			Utils::Stream::ClearPointer(&dest->aabbTreeCounts);

			SaveLogExit();
		}

		if (asset->aabbTrees)
		{
			AssertSize(Game::GfxCellTree, 4);
			SaveLogEnter("GfxCellTree");

			buffer->align(Utils::Stream::ALIGN_128);
			auto* destCellTreeTable = buffer->dest<Game::GfxCellTree>();
			buffer->saveArray(asset->aabbTrees, cellCount);

			for (int i = 0; i < cellCount; ++i)
			{
				auto* destCellTree = &destCellTreeTable[i];
				auto* cellTree = &asset->aabbTrees[i];

				if (cellTree->aabbTree)
				{
					AssertSize(Game::GfxAabbTree, 44);
					SaveLogEnter("GfxAabbTree");

					buffer->align(Utils::Stream::ALIGN_4);
					auto* destAabbTreeTable = buffer->dest<Game::GfxAabbTree>();
					buffer->saveArray(cellTree->aabbTree, asset->aabbTreeCounts[i].aabbTreeCount);

					// ok this one is based on some assumptions because the actual count is this
					// *(int *)((char *)&varGfxWorld->aabbTreeCounts->aabbTreeCount + (((char *)varGfxCellTree - (char *)varGfxWorld->aabbTrees) & 0xFFFFFFFC))
					// which makes no sense
					// what DOES make sense is using the count from the structure

					for (int j = 0; j < asset->aabbTreeCounts[i].aabbTreeCount; ++j)
					{
						auto* destAabbTree = &destAabbTreeTable[j];
						auto* aabbTree = &cellTree->aabbTree[j];

						if (aabbTree->smodelIndexes)
						{
							if (builder->hasPointer(aabbTree->smodelIndexes))
							{
								destAabbTree->smodelIndexes = builder->getPointer(aabbTree->smodelIndexes);
							}
							else
							{
								buffer->align(Utils::Stream::ALIGN_2);

								for (unsigned short k = 0; k < aabbTree->smodelIndexCount; ++k)
								{
									builder->storePointer(&aabbTree->smodelIndexes[k]);
									buffer->save(&aabbTree->smodelIndexes[k]);
								}

								Utils::Stream::ClearPointer(&destAabbTree->smodelIndexes);
							}
						}
					}

					Utils::Stream::ClearPointer(&destCellTree->aabbTree);
					SaveLogExit();
				}
			}

			Utils::Stream::ClearPointer(&dest->aabbTrees);
			SaveLogExit();
		}

		if (asset->cells)
		{
			AssertSize(Game::GfxCell, 40);
			SaveLogEnter("GfxCell");

			buffer->align(Utils::Stream::ALIGN_4);
			auto* destCellTable = buffer->dest<Game::GfxCell>();
			buffer->saveArray(asset->cells, cellCount);

			for (int i = 0; i < cellCount; ++i)
			{
				auto* destCell = &destCellTable[i];
				auto* cell = &asset->cells[i];

				if (cell->portals)
				{
					AssertSize(Game::GfxPortal, 60);
					SaveLogEnter("GfxPortal");

					buffer->align(Utils::Stream::ALIGN_4);
					auto* destPortalTable = buffer->dest<Game::GfxPortal>();
					buffer->saveArray(cell->portals, cell->portalCount);

					for (int j = 0; j < cell->portalCount; ++j)
					{
						auto* destPortal = &destPortalTable[j];
						auto* portal = &cell->portals[j];

						if (portal->vertices)
						{
							buffer->align(Utils::Stream::ALIGN_4);
							buffer->saveArray(portal->vertices, portal->vertexCount);
							Utils::Stream::ClearPointer(&destPortal->vertices);
						}
					}

					Utils::Stream::ClearPointer(&destCell->portals);
					SaveLogExit();
				}

				if (cell->reflectionProbes)
				{
					// no align for char
					buffer->saveArray(cell->reflectionProbes, cell->reflectionProbeCount);
					Utils::Stream::ClearPointer(&destCell->reflectionProbes);
				}
			}

			Utils::Stream::ClearPointer(&dest->cells);
			SaveLogExit();
		}

		this->saveGfxWorldDraw(&asset->draw, &dest->draw, builder);
		this->saveGfxLightGrid(&asset->lightGrid, &dest->lightGrid, builder);

		if (asset->models)
		{
			AssertSize(Game::GfxBrushModel, 60);
			SaveLogEnter("GfxBrushModel");

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->models, asset->modelCount);
			Utils::Stream::ClearPointer(&dest->models);

			SaveLogExit();
		}

		if (asset->materialMemory)
		{
			AssertSize(Game::MaterialMemory, 8);
			SaveLogEnter("MaterialMemory");

			buffer->align(Utils::Stream::ALIGN_4);
			auto* destMaterialMemoryTable = buffer->dest<Game::MaterialMemory>();
			buffer->saveArray(asset->materialMemory, asset->materialMemoryCount);

			for (int i = 0; i < asset->materialMemoryCount; ++i)
			{
				auto* destMaterialMemory = &destMaterialMemoryTable[i];
				auto* materialMemory = &asset->materialMemory[i];

				if (materialMemory->material)
				{
					destMaterialMemory->material = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, materialMemory->material).material;
				}
			}

			Utils::Stream::ClearPointer(&dest->materialMemory);
			SaveLogExit();
		}

		this->savesunflare_t(&asset->sun, &dest->sun, builder);

		if (asset->outdoorImage)
		{
			dest->outdoorImage = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->outdoorImage).image;
		}

		buffer->pushBlock(Game::XFILE_BLOCK_RUNTIME);

		if (asset->cellCasterBits)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->save(asset->cellCasterBits, 4, cellCount * ((cellCount + 31) >> 5));
			Utils::Stream::ClearPointer(&dest->cellCasterBits);
		}

		if (asset->cellHasSunLitSurfsBits)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->save(asset->cellHasSunLitSurfsBits, 4, ((cellCount + 31) >> 5));
			Utils::Stream::ClearPointer(&dest->cellHasSunLitSurfsBits);
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
			buffer->save(asset->primaryLightEntityShadowVis, 1, (asset->primaryLightCount + 0x1FFFF - asset->lastSunPrimaryLightIndex) << 15);
			Utils::Stream::ClearPointer(&dest->primaryLightEntityShadowVis);
		}

		if (asset->primaryLightDynEntShadowVis[0])
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->primaryLightDynEntShadowVis[0], asset->dpvsDyn.dynEntClientCount[0] * (asset->primaryLightCount - 1 - asset->lastSunPrimaryLightIndex));
			Utils::Stream::ClearPointer(&dest->primaryLightDynEntShadowVis[0]);
		}

		if (asset->primaryLightDynEntShadowVis[1])
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->primaryLightDynEntShadowVis[1], asset->dpvsDyn.dynEntClientCount[1] * (asset->primaryLightCount - 1 - asset->lastSunPrimaryLightIndex));
			Utils::Stream::ClearPointer(&dest->primaryLightDynEntShadowVis[1]);
		}

		if (asset->nonSunPrimaryLightForModelDynEnt)
		{
			// no align cause char
			buffer->saveArray(asset->nonSunPrimaryLightForModelDynEnt, asset->dpvsDyn.dynEntClientCount[0]);
			Utils::Stream::ClearPointer(&dest->nonSunPrimaryLightForModelDynEnt);
		}

		buffer->popBlock();

		if (asset->shadowGeom)
		{
			AssertSize(Game::GfxShadowGeometry, 12);
			SaveLogEnter("GfxShadowGeometry");

			buffer->align(Utils::Stream::ALIGN_4);
			auto* destShadowGeometryTable = buffer->dest<Game::GfxShadowGeometry>();
			buffer->saveArray(asset->shadowGeom, asset->primaryLightCount);

			for (unsigned int i = 0; i < asset->primaryLightCount; ++i)
			{
				auto* destShadowGeometry = &destShadowGeometryTable[i];
				auto* shadowGeometry = &asset->shadowGeom[i];

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
			SaveLogExit();
		}

		if (asset->lightRegion)
		{
			AssertSize(Game::GfxLightRegion, 8);
			SaveLogEnter("GfxLightRegion");

			buffer->align(Utils::Stream::ALIGN_4);
			auto* destLightRegionTable = buffer->dest<Game::GfxLightRegion>();
			buffer->saveArray(asset->lightRegion, asset->primaryLightCount);

			for (unsigned int i = 0; i < asset->primaryLightCount; ++i)
			{
				auto* destLightRegion = &destLightRegionTable[i];
				auto* lightRegion = &asset->lightRegion[i];

				if (lightRegion->hulls)
				{
					AssertSize(Game::GfxLightRegionHull, 80);
					SaveLogEnter("GfxLightRegionHull");

					buffer->align(Utils::Stream::ALIGN_4);
					auto* destLightRegionHullTable = buffer->dest<Game::GfxLightRegionHull>();
					buffer->saveArray(lightRegion->hulls, lightRegion->hullCount);

					for (unsigned int j = 0; j < lightRegion->hullCount; ++j)
					{
						auto* destLightRegionHull = &destLightRegionHullTable[j];
						auto* lightRegionHull = &lightRegion->hulls[j];

						if (lightRegionHull->axis)
						{
							AssertSize(Game::GfxLightRegionAxis, 20);
							SaveLogEnter("GfxLightRegionAxis");

							buffer->align(Utils::Stream::ALIGN_4);
							buffer->saveArray(lightRegionHull->axis, lightRegionHull->axisCount);
							Utils::Stream::ClearPointer(&destLightRegionHull->axis);

							SaveLogExit();
						}

					}

					Utils::Stream::ClearPointer(&destLightRegion->hulls);
					SaveLogExit();
				}
			}

			Utils::Stream::ClearPointer(&dest->lightRegion);
			SaveLogExit();
		}

		this->saveGfxWorldDpvsStatic(asset, &asset->dpvs, &dest->dpvs, asset->dpvsPlanes.cellCount, builder);
		this->saveGfxWorldDpvsDynamic(&asset->dpvsDyn, &dest->dpvsDyn, asset->dpvsPlanes.cellCount, builder);

		if (asset->heroOnlyLights)
		{
			AssertSize(Game::GfxHeroOnlyLight, 56);
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->heroOnlyLights, asset->heroOnlyLightCount);
			Utils::Stream::ClearPointer(&dest->heroOnlyLights);
		}

		buffer->popBlock();
		SaveLogExit();
	}
}
