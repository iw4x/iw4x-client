#include "STDInclude.hpp"

namespace Components
{
	bool AssetHandler::BypassState = false;
	std::map<Game::XAssetType, AssetHandler::IAsset*> AssetHandler::AssetInterfaces;
	std::map<Game::XAssetType, wink::slot<AssetHandler::Callback>> AssetHandler::TypeCallbacks;
	wink::signal<wink::slot<AssetHandler::RestrictCallback>> AssetHandler::RestrictSignal;

	std::map<void*, void*> AssetHandler::Relocations;

	std::vector<std::pair<Game::XAssetType, std::string>> AssetHandler::EmptyAssets;

	std::map<std::string, Game::XAssetHeader> AssetHandler::TemporaryAssets[Game::XAssetType::ASSET_TYPE_COUNT];

	void AssetHandler::RegisterInterface(IAsset* iAsset)
	{
		if (!iAsset) return;
		if (iAsset->GetType() == Game::XAssetType::ASSET_TYPE_INVALID)
		{
			delete iAsset;
			return;
		}

		if (AssetHandler::AssetInterfaces.find(iAsset->GetType()) != AssetHandler::AssetInterfaces.end())
		{
			Logger::Print("Duplicate asset interface: %s\n", Game::DB_GetXAssetTypeName(iAsset->GetType()));
			delete AssetHandler::AssetInterfaces[iAsset->GetType()];
		}
		else
		{
			Logger::Print("Asset interface registered: %s\n", Game::DB_GetXAssetTypeName(iAsset->GetType()));
		}

		AssetHandler::AssetInterfaces[iAsset->GetType()] = iAsset;
	}

	void AssetHandler::ClearTemporaryAssets()
	{
		for (int i = 0; i < Game::XAssetType::ASSET_TYPE_COUNT; ++i)
		{
			AssetHandler::TemporaryAssets[i].clear();
		}
	}

	void AssetHandler::StoreTemporaryAsset(Game::XAssetType type, Game::XAssetHeader asset)
	{
		AssetHandler::TemporaryAssets[type][Game::DB_GetXAssetNameHandlers[type](&asset)] = asset;
	}

	Game::XAssetHeader AssetHandler::FindAsset(Game::XAssetType type, const char* filename)
	{
		Game::XAssetHeader header = { 0 };

		if (filename)
		{
			// Allow call DB_FindXAssetHeader within the hook
			AssetHandler::BypassState = true;

			if (AssetHandler::TypeCallbacks.find(type) != AssetHandler::TypeCallbacks.end())
			{
				header = AssetHandler::TypeCallbacks[type](type, filename);
			}

			// Disallow calling DB_FindXAssetHeader ;)
			AssetHandler::BypassState = false;
		}

		return header;
	}

	__declspec(naked) void AssetHandler::FindAssetStub()
	{
		__asm
		{
			push ecx
			push ebx
			push ebp
			push esi
			push edi

			// Check if custom handler should be bypassed
			xor eax, eax
			mov al, AssetHandler::BypassState

			test al, al
			jnz finishOriginal

			mov ecx, [esp + 18h] // Asset type
			mov ebx, [esp + 1Ch] // Filename

			push ebx
			push ecx

			call AssetHandler::FindAsset

			add esp, 8h

			test eax, eax
			jnz finishFound

		finishOriginal:
			// Asset not found using custom handlers, redirect to DB_FindXAssetHeader
			mov ebx, ds:6D7190h // InterlockedDecrement
			mov eax, 40793Bh
			jmp eax

		finishFound:
			pop edi
			pop esi
			pop ebp
			pop ebx
			pop ecx
			retn
		}
	}

	bool AssetHandler::IsAssetEligible(Game::XAssetType type, Game::XAssetHeader *asset)
	{
		const char* name = Game::DB_GetXAssetNameHandlers[type](asset);
		if (!name) return false;

		for (auto i = AssetHandler::EmptyAssets.begin(); i != AssetHandler::EmptyAssets.end();)
		{
			if (i->first == type && i->second == name)
			{
				i = AssetHandler::EmptyAssets.erase(i);
			}
			else
			{
				++i;
			}
		}

		if (type == 20/* && name == "maps/mp/mp_underpass.d3dbsp"s*/)
		{
			char* world = nullptr;
			Game::DB_EnumXAssets_Internal(Game::XAssetType::ASSET_TYPE_FX_MAP, [] (Game::XAssetHeader header, void* world)
			{
				*reinterpret_cast<Game::GfxWorld**>(world) = header.gfxMap;
			}, &world, false);

			if (world)
			{
				//std::memcpy(world + 4, &asset->rawfile->sizeCompressed, Game::DB_GetXAssetSizeHandlers[Game::XAssetType::ASSET_TYPE_FX_MAP]() - 4);
			}
		}

		if (type == 22)
		{
			OutputDebugStringA(Utils::String::VA("%s: %s\n", FastFiles::Current().data(), name));
		}

		/*
		if (type <= 9 && type >= 6 && Zones::Version() >= 359)
		{
			static Game::XAssetType __type;
			static std::string __name;
			__name = name;
			__type = type;

			Game::MaterialTechniqueSet* techset = nullptr;
			Game::DB_EnumXAssets_Internal(type, [] (Game::XAssetHeader header, void* _asset)
			{
				Game::MaterialTechniqueSet** _techset = reinterpret_cast<Game::MaterialTechniqueSet**>(_asset);

				if (__name == Game::DB_GetXAssetNameHandlers[__type](&header))
				{
					*_techset = header.materialTechset;
				}
			}, &techset, false);

			if (techset)
			{
				if(type == 6)

				std::memcpy(asset->materialTechset, techset, Game::DB_GetXAssetSizeHandlers[type]());
				//return false;
				OutputDebugStringA("");
			}
		}
		*/

		if (type == 21/* && asset->gfxMap->baseName == "mp_underpass"s*/ && false)
		{

			//asset->gfxMap->dpvs.smodelCount = 0;

			Game::GfxWorld* world = nullptr;
			Game::DB_EnumXAssets_Internal(Game::XAssetType::ASSET_TYPE_GFX_MAP, [] (Game::XAssetHeader header, void* world)
			{
				*reinterpret_cast<Game::GfxWorld**>(world) = header.gfxMap;
			}, &world, false);

// 			int targetId = *world->skies->skyStartSurfs;
//			int sourceId = *asset->gfxMap->skies->skyStartSurfs;

// 			world->dpvs.surfaceVisData[0][targetId] = asset->gfxMap->dpvs.surfaceVisData[0][sourceId];
// 			world->dpvs.surfaceVisData[1][targetId] = asset->gfxMap->dpvs.surfaceVisData[1][sourceId];
// 			world->dpvs.surfaceVisData[2][targetId] = asset->gfxMap->dpvs.surfaceVisData[2][sourceId];
// 
// 			world->dpvs.surfaces[targetId].material = asset->gfxMap->dpvs.surfaces[sourceId].material;
// 			world->dpvs.surfacesBounds[targetId].flags = asset->gfxMap->dpvs.surfacesBounds[sourceId].flags;
// 
// 			world->dpvs.surfaceCastsSunShadow[targetId] = asset->gfxMap->dpvs.surfaceCastsSunShadow[sourceId];
// 
// 			world->skies->skyImage = asset->gfxMap->skies->skyImage;

//			std::memcpy(&world->worldDraw.lightmaps[world->dpvs.surfaces[targetId].lightmapIndex], &asset->gfxMap->worldDraw.lightmaps[asset->gfxMap->dpvs.surfaces[sourceId].lightmapIndex], 8);

// 			DWORD* stuff = reinterpret_cast<DWORD*>(asset->gfxMap->unknown1);
// 			for (unsigned int i = 0; i < asset->gfxMap->dpvs.staticSurfaceCount; ++i)
// 			{
// 				asset->gfxMap->dpvs.surfaces[i].tris.vertexLayerData = 0;
// 			}

			auto replaceMaterial = [] (Game::GfxSurface* surfs, unsigned int count, Game::Material** base)
			{
				static int _i = 0;
				_i++;
				for (unsigned int i = 0; i < count; ++i)
				{
					Game::Material* source = *base;
					Game::Material* target = surfs[i].material;

					if (source->gameFlags == target->gameFlags)
					{
						*base = target;
						OutputDebugStringA(Utils::String::VA("SUCCESS: replaced %s with %s %d\n", source->name, target->name, _i));
						OutputDebugStringA(Utils::String::VA("SortKey: %X\nGameFlags: %X\nStateFlags: %X\nTechset: %s\n\n", source->sortKey & 0xFF, source->gameFlags & 0xFF, source->stateFlags & 0xFF, source->techniqueSet->name));

						source = target;
						OutputDebugStringA(Utils::String::VA("SortKey: %X\nGameFlags: %X\nStateFlags: %X\nTechset: %s\n\n", source->sortKey & 0xFF, source->gameFlags & 0xFF, source->stateFlags & 0xFF, source->techniqueSet->name));
						return;
					}
				}

				OutputDebugStringA(Utils::String::VA("FAILURE: unable to replace %s %d\n", (*base)->name, _i));

				Game::Material* source = *base;
				OutputDebugStringA(Utils::String::VA("SortKey: %X\nGameFlags: %X\nStateFlags: %X\nTechset: %s\n\n", source->sortKey & 0xFF, source->gameFlags & 0xFF, source->stateFlags & 0xFF, source->techniqueSet->name));
			};

// 			for (unsigned int i = 0; i < world->dpvs.staticSurfaceCount; ++i)
// 			{
// 				replaceMaterial(asset->gfxMap->dpvs.surfaces, asset->gfxMap->dpvs.staticSurfaceCount, &world->dpvs.surfaces[i].material);
// 			}

// 			for (int i = 0; i < world->materialMemoryCount; ++i)
// 			{
// 				for (int j = 0; j < asset->gfxMap->materialMemoryCount; ++j)
// 				{
// 					if (world->materialMemory[i].material->gameFlags == asset->gfxMap->materialMemory[j].material->gameFlags)
// 					{
// 						world->materialMemory[i].material = asset->gfxMap->materialMemory[j].material;
// 						world->materialMemory[i].memory = asset->gfxMap->materialMemory[j].memory;
// 					}
// 				}
// 			}

// 			//std::memcpy(&world->dpvs, &asset->gfxMap->dpvs, sizeof(Game::GfxWorldDpvsStatic));
// 			std::memcpy(&world->worldDraw, &asset->gfxMap->worldDraw, sizeof(Game::GfxWorldDraw));
// 			std::memcpy(&world->lightGrid, &asset->gfxMap->lightGrid, sizeof(Game::GfxLightGrid));
// 			//std::memcpy(&world->dpvsPlanes, &asset->gfxMap->dpvsPlanes, sizeof(Game::GfxWorldDpvsPlanes));
// 			world->dpvs.surfaces = asset->gfxMap->dpvs.surfaces;
// 			world->dpvs.surfacesBounds = asset->gfxMap->dpvs.surfacesBounds;
// 			world->dpvs.staticSurfaceCount = asset->gfxMap->dpvs.staticSurfaceCount;
// 			world->dpvs.sortedSurfIndex = asset->gfxMap->dpvs.sortedSurfIndex;
// 			world->dpvs.surfaceVisData[0] = asset->gfxMap->dpvs.surfaceVisData[0];
// 			world->dpvs.surfaceVisData[1] = asset->gfxMap->dpvs.surfaceVisData[1];
// 			world->dpvs.surfaceVisData[2] = asset->gfxMap->dpvs.surfaceVisData[2];
// 
// 			world->cellCasterBits[0] = asset->gfxMap->cellCasterBits[0];
// 			world->cellCasterBits[1] = asset->gfxMap->cellCasterBits[1];
// 
// 			world->skies = asset->gfxMap->skies;
// 			world->materialMemory = asset->gfxMap->materialMemory;
// 			world->materialMemoryCount = asset->gfxMap->materialMemoryCount;
// 
// 			world->dpvs.litSurfsBegin = asset->gfxMap->dpvs.litSurfsBegin;
// 			world->dpvs.litSurfsEnd = asset->gfxMap->dpvs.litSurfsEnd;
// 			world->dpvs.surfaceMaterials = asset->gfxMap->dpvs.surfaceMaterials;
// 
// 			std::memcpy(world->pad, asset->gfxMap->pad, 68);
// 			std::memcpy(world->dpvs.unknown1, asset->gfxMap->dpvs.unknown1, 8);
// 			std::memcpy(&world->dpvs.unknown1[0x18], &asset->gfxMap->dpvs.unknown1[0x18], 4);
// 			std::memcpy(&world->sun, &asset->gfxMap->sun, sizeof(Game::sunflare_t));

			//world->dpvs.smodelCount = 0;

			//std::memcpy(world->dpvsPlanes.planes, asset->gfxMap->dpvsPlanes.planes, 20 * (std::min(world->planeCount, asset->gfxMap->planeCount)));

// 			for (int i = 0; i < world->planeCount; ++i)
// 			{
// 				world->dpvsPlanes.planes[i].type = 0;
// 			}

			//world->cells = asset->gfxMap->cells;

// 			for(int i = 0; i < std::min(asset->gfxMap->dpvsPlanes.cellCount, world->dpvsPlanes.cellCount); ++i)
// 			{
// // 				for (int j = 0; j < std::min(asset->gfxMap->aabbTreeCounts[i].aabbTreeCount, world->aabbTreeCounts[i].aabbTreeCount); ++j)
// // 				{
// // 					Game::GfxAabbTree* treeA = &world->aabbTrees[i].aabbTree[j];
// // 					Game::GfxAabbTree* treeB = &asset->gfxMap->aabbTrees[i].aabbTree[j];
// // 
// // 					treeA->childrenOffset
// // 				}
// 
// 				for (int j = 0; j < std::min(asset->gfxMap->cells[i].portalCount, world->cells[i].portalCount); ++j)
// 				{
// 					Game::GfxPortal* portalA = &world->cells[i].portals[j];
// 					Game::GfxPortal* portalB = &asset->gfxMap->cells[i].portals[j];
// 
// 					std::memcpy(&portalA->writable, &portalB->writable, sizeof(portalA->writable));
// 					std::memcpy(&portalA->plane, &portalB->plane, sizeof(portalA->plane));
// 					//std::memcpy(&portalA->unknown, &portalB->unknown, sizeof(portalA->unknown));
// 					std::memcpy(&portalA->hullAxis, &portalB->hullAxis, sizeof(portalA->hullAxis));
// 				}
// 
// 				//std::memcpy(&world->cells[i], &asset->gfxMap->cells[i], sizeof(Game::GfxCell));
// 				//std::memcpy(&world->cells[i], &asset->gfxMap->cells[i], sizeof(Game::GfxCell));
// 			}

			//std::memcpy(&world->planeCount, &asset->gfxMap->planeCount, 620);
			
// 			asset->gfxMap->cells = world->cells;
// 			asset->gfxMap->aabbTrees = world->aabbTrees;
// 			asset->gfxMap->aabbTreeCounts = world->aabbTreeCounts;
// 			std::memcpy(&asset->gfxMap->dpvsPlanes, &world->dpvsPlanes, sizeof(Game::GfxWorldDpvsPlanes));


			//std::memcpy(&world->planeCount, &asset->gfxMap->planeCount, 620);

			OutputDebugStringA("");
		}

		static Game::MaterialTechniqueSet* technique = nullptr;

		/*if (type == 9)
		{
			FILE* fp;
			fopen_s(&fp, "test.txt", "a");
			fprintf(fp, "%s: %s\n", FastFiles::Current().data(), name);

			for (int i = 0; i < 48; i++)
			{
				if (asset->materialTechset->techniques[i])
				{
					fprintf(fp, "\t%d: %s\n", i, asset->materialTechset->techniques[i]->name);

					for (int j = 0; j <asset->materialTechset->techniques[i]->numPasses; j++)
					{
						Game::MaterialPass* pass = &asset->materialTechset->techniques[i]->passes[j];

						for (int k = 0; k < (pass->argCount1 + pass->argCount2 + pass->argCount3); k++)
						{
							fprintf(fp, "\t\t%d.%d.%d:\n", i, j, k);

							fprintf(fp, "\t\t\tDest: %d\n", pass->argumentDef[k].dest & 0xFFFF);
							fprintf(fp, "\t\t\tMore: %d\n", pass->argumentDef[k].more & 0xFFFF);
							fprintf(fp, "\t\t\tType: %d\n", pass->argumentDef[k].type & 0xFFFF);
							fprintf(fp, "\t\t\tPara: %d\n", pass->argumentDef[k].paramID & 0xFFFF);
						}
					}
				}
			}

			fprintf(fp, "\n");
			fclose(fp);
		}*/

// 		if (type == 5)
// 		{
// 			FILE* fp;
// 			fopen_s(&fp, "test.txt", "a");
// 			fprintf(fp, "%s: %s %X %X %X\n", FastFiles::Current().data(), name, asset->material->sortKey & 0xFF, asset->material->gameFlags & 0xFF, asset->material->stateFlags & 0xFF);
// 			fclose(fp);
// 		}

		if (type == 5 && name == "wc/codo_ui_viewer_black_decal3"s)
		{
			asset->material->sortKey = 0xE;
		}

		if (Flags::HasFlag("entries"))
		{
			OutputDebugStringA(Utils::String::VA("%s: %d: %s\n", FastFiles::Current().data(), type, name));
		}

		bool restrict = false;
		AssetHandler::RestrictSignal(type, *asset, name, &restrict);

		// If no slot restricts the loading, we can load the asset
		return (!restrict);
	}

	__declspec(naked) void AssetHandler::AddAssetStub()
	{
		__asm
		{
			push [esp + 8]
			push [esp + 8]
			call AssetHandler::IsAssetEligible
			add esp, 08h

			test al, al
			jz doNotLoad

			mov eax, [esp + 8]
			sub esp, 14h
			mov ecx, 5BB657h
			jmp ecx

		doNotLoad:
			mov eax, [esp + 8]
			retn
		}
	}

	void AssetHandler::OnFind(Game::XAssetType type, AssetHandler::Callback* callback)
	{
		AssetHandler::TypeCallbacks[type] = callback;
	}

	void AssetHandler::OnLoad(AssetHandler::RestrictCallback* callback)
	{
		AssetHandler::RestrictSignal.connect(callback);
	}

	void AssetHandler::ClearRelocations()
	{
		AssetHandler::Relocations.clear();
	}

	void AssetHandler::Relocate(void* start, void* to, DWORD size)
	{
		for (DWORD i = 0; i < size; i += 4)
		{
			AssetHandler::Relocations[reinterpret_cast<char*>(start) + i] = reinterpret_cast<char*>(to) + i;
		}
	}

	void AssetHandler::OffsetToAlias(Utils::Stream::Offset* offset)
	{
		// Same here, reinterpret the value, as we're operating inside the game's environment
		void* pointer = (*Game::g_streamBlocks)[offset->GetUnpackedBlock()].data + offset->GetUnpackedOffset();

		if (AssetHandler::Relocations.find(pointer) != AssetHandler::Relocations.end())
		{
			pointer = AssetHandler::Relocations[pointer];
		}

		offset->pointer = *reinterpret_cast<void**>(pointer);

#ifdef DEBUG
		Game::XAssetHeader zob{ offset->pointer };
#endif
 	}

	void AssetHandler::ZoneSave(Game::XAsset asset, ZoneBuilder::Zone* builder)
	{
		if (AssetHandler::AssetInterfaces.find(asset.type) != AssetHandler::AssetInterfaces.end())
		{
			AssetHandler::AssetInterfaces[asset.type]->Save(asset.header, builder);
		}
		else
		{
			Logger::Error("No interface for type '%s'!", Game::DB_GetXAssetTypeName(asset.type));
		}
	}

	void AssetHandler::ZoneMark(Game::XAsset asset, ZoneBuilder::Zone* builder)
	{
		if (AssetHandler::AssetInterfaces.find(asset.type) != AssetHandler::AssetInterfaces.end())
		{
			AssetHandler::AssetInterfaces[asset.type]->Mark(asset.header, builder);
		}
		else
		{
			Logger::Error("No interface for type '%s'!", Game::DB_GetXAssetTypeName(asset.type));
		}
	}

	Game::XAssetHeader AssetHandler::FindAssetForZone(Game::XAssetType type, std::string filename, ZoneBuilder::Zone* builder)
	{
		Game::XAssetHeader header = { 0 };
		if (type >= Game::XAssetType::ASSET_TYPE_COUNT) return header;

		auto tempPool = &AssetHandler::TemporaryAssets[type];
		auto entry = tempPool->find(filename);
		if (entry != tempPool->end())
		{
			return { entry->second };
		}

		if (AssetHandler::AssetInterfaces.find(type) != AssetHandler::AssetInterfaces.end())
		{
			AssetHandler::AssetInterfaces[type]->Load(&header, filename, builder);

			if (header.data)
			{
				Components::AssetHandler::StoreTemporaryAsset(type, header);
			}
		}

		if (!header.data)
		{
			header = Game::DB_FindXAssetHeader(type, filename.data());
			if(header.data) Components::AssetHandler::StoreTemporaryAsset(type, header); // Might increase efficiency...
		}

		return header;
	}

	Game::XAssetHeader AssetHandler::FindOriginalAsset(Game::XAssetType type, const char* filename)
	{
		bool OriginalState = AssetHandler::BypassState;

		AssetHandler::BypassState = true;
		Game::XAssetHeader header = Game::DB_FindXAssetHeader(type, filename);
		if(!OriginalState) AssetHandler::BypassState = false;

		return header;
	}

	void AssetHandler::StoreEmptyAsset(Game::XAssetType type, const char* name)
	{
		AssetHandler::EmptyAssets.push_back({ type, name });
	}

	__declspec(naked) void AssetHandler::StoreEmptyAssetStub()
	{
		__asm
		{
			pushad
			push ebx
			push eax

			call AssetHandler::StoreEmptyAsset

			pop eax
			pop ebx
			popad

			push 5BB290h
			retn
		}
	}

	AssetHandler::AssetHandler()
	{
		AssetHandler::ClearTemporaryAssets();

		// DB_FindXAssetHeader
		Utils::Hook(Game::DB_FindXAssetHeader, AssetHandler::FindAssetStub).Install()->Quick();

		// DB_ConvertOffsetToAlias
		Utils::Hook(0x4FDFA0, AssetHandler::OffsetToAlias, HOOK_JUMP).Install()->Quick();

		// DB_AddXAsset
		Utils::Hook(0x5BB650, AssetHandler::AddAssetStub, HOOK_JUMP).Install()->Quick();

		// Store empty assets
		Utils::Hook(0x5BB6EC, AssetHandler::StoreEmptyAssetStub, HOOK_CALL).Install()->Quick();

		// Log missing empty assets
		QuickPatch::OnFrame([] ()
		{
			if (FastFiles::Ready() && !AssetHandler::EmptyAssets.empty())
			{
				for (auto& asset : AssetHandler::EmptyAssets)
				{
					Game::Sys_Error(25, reinterpret_cast<char*>(0x724428), Game::DB_GetXAssetTypeName(asset.first), asset.second.data());
				}

				AssetHandler::EmptyAssets.clear();
			}
		});

		// Register asset interfaces
		if (ZoneBuilder::IsEnabled())
		{
			AssetHandler::RegisterInterface(new Assets::IXModel());
			AssetHandler::RegisterInterface(new Assets::IMapEnts());
			AssetHandler::RegisterInterface(new Assets::IRawFile());
			AssetHandler::RegisterInterface(new Assets::IGfxImage());
			AssetHandler::RegisterInterface(new Assets::IMaterial());
			AssetHandler::RegisterInterface(new Assets::IPhysPreset());
			AssetHandler::RegisterInterface(new Assets::IXAnimParts());
			AssetHandler::RegisterInterface(new Assets::IPhysCollmap());
			AssetHandler::RegisterInterface(new Assets::IStringTable());
			//AssetHandler::RegisterInterface(new Assets::IXModelSurfs());
			AssetHandler::RegisterInterface(new Assets::ILocalizedEntry());
			AssetHandler::RegisterInterface(new Assets::IMaterialPixelShader());
			AssetHandler::RegisterInterface(new Assets::IMaterialTechniqueSet());
			AssetHandler::RegisterInterface(new Assets::IMaterialVertexShader());
			AssetHandler::RegisterInterface(new Assets::IStructuredDataDefSet());
			AssetHandler::RegisterInterface(new Assets::IMaterialVertexDeclaration());
		}
	}

	AssetHandler::~AssetHandler()
	{
		ClearTemporaryAssets();

		for (auto i = AssetHandler::AssetInterfaces.begin(); i != AssetHandler::AssetInterfaces.end(); ++i)
		{
			delete i->second;
		}

		AssetHandler::Relocations.clear();
		AssetHandler::AssetInterfaces.clear();
		AssetHandler::RestrictSignal.clear();
		AssetHandler::TypeCallbacks.clear();
	}
}
