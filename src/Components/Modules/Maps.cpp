#include "STDInclude.hpp"

namespace Components
{
	std::map<std::string, std::string> Maps::DependencyList;
	std::vector<std::string> Maps::CurrentDependencies;

	std::vector<Game::XAssetEntry> Maps::EntryPool;

	void Maps::LoadMapZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync)
	{
		if (!zoneInfo) return;

		Maps::CurrentDependencies.clear();
		for (auto i = Maps::DependencyList.begin(); i != Maps::DependencyList.end(); ++i)
		{
			if (std::regex_match(zoneInfo->name, std::regex(i->first)))
			{
				if (std::find(Maps::CurrentDependencies.begin(), Maps::CurrentDependencies.end(), i->second) == Maps::CurrentDependencies.end())
				{
					Maps::CurrentDependencies.push_back(i->second);
				}
			}
		}

		std::vector<Game::XZoneInfo> data;
		Utils::Merge(&data, zoneInfo, zoneCount);

		for (unsigned int i = 0; i < Maps::CurrentDependencies.size(); ++i)
		{
			Game::XZoneInfo info;

			info.name = (&Maps::CurrentDependencies[i])->data();
			info.allocFlags = zoneInfo->allocFlags;
			info.freeFlags = zoneInfo->freeFlags;

			data.push_back(info);
		}

		// Load patch files
		std::string patchZone = fmt::sprintf("patch_%s", zoneInfo->name);
		if (FastFiles::Exists(patchZone))
		{
			data.push_back({ patchZone.data(), zoneInfo->allocFlags, zoneInfo->freeFlags });
		}

		return FastFiles::LoadLocalizeZones(data.data(), data.size(), sync);
	}

	void Maps::OverrideMapEnts(Game::MapEnts* ents)
	{
		auto callback = [] (Game::XAssetHeader header, void* ents)
		{
			Game::MapEnts* mapEnts = reinterpret_cast<Game::MapEnts*>(ents);
			Game::clipMap_t* clipMap = header.clipMap;

			if (clipMap && mapEnts && !_stricmp(mapEnts->name, clipMap->name))
			{
				clipMap->mapEnts = mapEnts;
				//*Game::marMapEntsPtr = mapEnts;
				//Game::G_SpawnEntitiesFromString();
			}
		};

		// Internal doesn't lock the thread, as locking is impossible, due to executing this in the thread that holds the current lock
		Game::DB_EnumXAssets_Internal(Game::XAssetType::ASSET_TYPE_COL_MAP_MP, callback, ents, true);
		Game::DB_EnumXAssets_Internal(Game::XAssetType::ASSET_TYPE_COL_MAP_SP, callback, ents, true);
	}

	void Maps::LoadAssetRestrict(Game::XAssetType type, Game::XAssetHeader asset, std::string name, bool* restrict)
	{
		if (std::find(Maps::CurrentDependencies.begin(), Maps::CurrentDependencies.end(), FastFiles::Current()) != Maps::CurrentDependencies.end())
		{
			if (type == Game::XAssetType::ASSET_TYPE_GAME_MAP_MP || type == Game::XAssetType::ASSET_TYPE_COL_MAP_MP || type == Game::XAssetType::ASSET_TYPE_GFX_MAP || type == Game::XAssetType::ASSET_TYPE_MAP_ENTS || type == Game::XAssetType::ASSET_TYPE_COM_MAP || type == Game::XAssetType::ASSET_TYPE_FX_MAP)
			{
				*restrict = true;
				return;
			}
		}

		if (type == Game::XAssetType::ASSET_TYPE_ADDON_MAP_ENTS)
		{
			*restrict = true;
			return;
		}

		if (type == Game::XAssetType::ASSET_TYPE_WEAPON)
		{
			if (!strstr(name.data(), "_mp") && name != "none" && name != "destructible_car")
			{
				*restrict = true;
				return;
			}
		}

		if (type == Game::XAssetType::ASSET_TYPE_STRINGTABLE)
		{
			if (FastFiles::Current() == "mp_cross_fire")
			{
				*restrict = true;
				return;
			}
		}

		if (type == Game::XAssetType::ASSET_TYPE_MAP_ENTS)
		{
			static std::string mapEntities;
			FileSystem::File ents(name + ".ents");
			if (ents.Exists())
			{
				mapEntities = ents.GetBuffer();
				asset.mapEnts->entityString = const_cast<char*>(mapEntities.data());
				asset.mapEnts->numEntityChars = mapEntities.size() + 1;
			}

			// Apply new mapEnts
			// This doesn't work, entities are spawned before the patch file is loaded
			//Maps::OverrideMapEnts(asset.mapEnts);
		}

		if (type == Game::XAssetType::ASSET_TYPE_MENU && Zones::Version() >= 359)
		{
			*restrict = true;
			return;
		}
	}

	void Maps::GetBSPName(char* buffer, size_t size, const char* format, const char* mapname)
	{
		if (_strnicmp("mp_", mapname, 3))
		{
			format = "maps/%s.d3dbsp";
		}

		bool handleAsSp = false;

		for (auto dependency : Maps::DependencyList)
		{
			if (dependency.second == "iw4x_dependencies_mp" && std::regex_match(mapname, std::regex(dependency.first)))
			{
				handleAsSp = true;
				break;
			}
		}

		if (_strnicmp("mp_", mapname, 3) || handleAsSp)
		{
			// Adjust pointer to GameMap_Data
			Utils::Hook::Set<Game::GameMap_Data**>(0x4D90B7, &(Game::DB_XAssetPool[Game::XAssetType::ASSET_TYPE_GAME_MAP_SP].gameMapSP[0].data));
		}
		else
		{
			// Adjust pointer to GameMap_Data
			Utils::Hook::Set<Game::GameMap_Data**>(0x4D90B7, &(Game::DB_XAssetPool[Game::XAssetType::ASSET_TYPE_GAME_MAP_MP].gameMapMP[0].data));
		}

		AntiCheat::EmptyHash();

		_snprintf_s(buffer, size, size, format, mapname);
	}

	void Maps::AddDependency(std::string expression, std::string zone)
	{
		// Test expression before adding it
		try
		{
			std::regex _(expression);
		}
		catch (const std::exception e)
		{
			MessageBoxA(0, Utils::String::VA("Invalid regular expression: %s", expression.data()), "Warning", MB_ICONEXCLAMATION);
			return;
		}

		Maps::DependencyList[expression] = zone;
	}

	int Maps::IgnoreEntityStub(const char* entity)
	{
		return (Utils::String::StartsWith(entity, "dyn_") || Utils::String::StartsWith(entity, "node_") || Utils::String::StartsWith(entity, "actor_"));
	}

#if defined(DEBUG) && defined(ENABLE_DXSDK)
	// Credit to SE2Dev, as we shouldn't share the code, keep that in debug mode!
	void Maps::ExportMap(Game::GfxWorld* world)
	{
		Utils::Memory::Allocator allocator;
		if (!world) return;

		Logger::Print("Exporting '%s'...\n", world->baseName);

		std::string mtl;
		mtl.append("# IW4x MTL File\n");
		mtl.append("# Credit to SE2Dev for his D3DBSP Tool\n");

		std::string map;
		map.append("# Generated by IW4x\n");
		map.append("# Credit to SE2Dev for his D3DBSP Tool\n");
		map.append(fmt::sprintf("o %s\n", world->baseName));
		map.append(fmt::sprintf("mtllib %s.mtl\n", world->baseName));

		Logger::Print("Writing vertices...\n");
		for (unsigned int i = 0; i < world->worldDraw.vertexCount; i++)
		{
			// Y/Z need to be inverted
			float x = world->worldDraw.vd.vertices[i].xyz[0];
			float y = world->worldDraw.vd.vertices[i].xyz[2];
			float z = world->worldDraw.vd.vertices[i].xyz[1];

			map.append(fmt::sprintf("v %.6f %.6f %.6f\n", x,y, z));
		}

		Logger::Print("Writing texture coordinates...\n");
		for (unsigned int i = 0; i < world->worldDraw.vertexCount; i++)
		{
			map.append(fmt::sprintf("vt %.6f %.6f\n", world->worldDraw.vd.vertices[i].texCoord[0], world->worldDraw.vd.vertices[i].texCoord[1]));
		}

		Logger::Print("Searching materials...\n");
		int materialCount = 0;
		Game::Material** materials = allocator.AllocateArray<Game::Material*>(world->dpvs.staticSurfaceCount);

		for (unsigned int i = 0; i < world->dpvs.staticSurfaceCount; i++)
		{
			bool isNewMat = true;

			for (int j = 0; j < materialCount; j++)
			{
				if (world->dpvs.surfaces[i].material == materials[j])
				{
					isNewMat = false;
					break;
				}
			}

			if (isNewMat)
			{
				materials[materialCount++] = world->dpvs.surfaces[i].material;
			}
		}

		Utils::IO::CreateDirectory(fmt::sprintf("raw/mapdump/%s/textures", world->baseName));
		mtl.append(fmt::sprintf("# Material Count: %d\n", materialCount));

		Logger::Print("Exporting materials and faces...\n");
		for (int m = 0; m < materialCount; m++)
		{
			std::string name = materials[m]->name;

			auto pos = name.find_last_of("/");
			if (pos != std::string::npos)
			{
				name = name.substr(pos + 1);
			}

			map.append(fmt::sprintf("usemtl %s\n", name.data()));
			map.append("s off\n");

			Game::GfxImage* image = materials[m]->textureTable[0].info.image;

			for (char l = 0; l < materials[m]->textureCount; ++l)
			{
				if (materials[m]->textureTable[l].nameStart == 'c')
				{
					if (materials[m]->textureTable[l].nameEnd == 'p')
					{
						image = materials[m]->textureTable[l].info.image; // Hopefully our colorMap
					}
				}
			}

			std::string _name = fmt::sprintf("raw/mapdump/%s/textures/%s.png", world->baseName, image->name);
			D3DXSaveTextureToFile(std::wstring(_name.begin(), _name.end()).data(), D3DXIFF_PNG, image->texture, NULL);

			mtl.append(fmt::sprintf("newmtl %s\n", name.data()));
			mtl.append("Ka 1.0000 1.0000 1.0000\n");
			mtl.append("Kd 1.0000 1.0000 1.0000\n");
			mtl.append("illum 1\n");
			mtl.append(fmt::sprintf("map_Ka textures/%s.png\n", image->name));
			mtl.append(fmt::sprintf("map_Kd textures/%s.png\n", image->name));

			for (unsigned int i = 0; i < world->dpvs.staticSurfaceCount; i++)
			{
				if (world->dpvs.surfaces[i].material != materials[m])
					continue;

				int vertOffset = world->dpvs.surfaces[i].tris.firstVertex + 1;//+1 cus obj starts at 1
				int indexOffset = world->dpvs.surfaces[i].tris.baseIndex;
				for (unsigned short j = 0; j < world->dpvs.surfaces[i].tris.triCount; j++)
				{
					int a = world->worldDraw.indices[indexOffset + j * 3 + 0] + vertOffset;
					int b = world->worldDraw.indices[indexOffset + j * 3 + 1] + vertOffset;
					int c = world->worldDraw.indices[indexOffset + j * 3 + 2] + vertOffset;

					map.append(fmt::sprintf("f %d/%d %d/%d %d/%d\n", a, a, b, b, c, c));
				}
			}
		}

		Logger::Print("Writing final files...\n");
		Utils::IO::WriteFile(fmt::sprintf("raw/mapdump/%s/%s.mtl", world->baseName, world->baseName), mtl);
		Utils::IO::WriteFile(fmt::sprintf("raw/mapdump/%s/%s.obj", world->baseName, world->baseName), map);
	}
#endif

	void Maps::ReallocateEntryPool()
	{
		Assert_Size(Game::XAssetEntry, 16);

		Maps::EntryPool.clear();
		Maps::EntryPool.resize(789312);

		// Apply new size
		Utils::Hook::Set<DWORD>(0x5BAEB0, Maps::EntryPool.size());

		// Apply new pool
		Utils::Hook::Set<Game::XAssetEntry*>(0x48E6F4, Maps::EntryPool.data());
		Utils::Hook::Set<Game::XAssetEntry*>(0x4C67E4, Maps::EntryPool.data());
		Utils::Hook::Set<Game::XAssetEntry*>(0x4C8584, Maps::EntryPool.data());
		Utils::Hook::Set<Game::XAssetEntry*>(0x5BAEA8, Maps::EntryPool.data());
		Utils::Hook::Set<Game::XAssetEntry*>(0x5BB0C4, Maps::EntryPool.data());
		Utils::Hook::Set<Game::XAssetEntry*>(0x5BB0F5, Maps::EntryPool.data());
		Utils::Hook::Set<Game::XAssetEntry*>(0x5BB1D4, Maps::EntryPool.data());
		Utils::Hook::Set<Game::XAssetEntry*>(0x5BB235, Maps::EntryPool.data());
		Utils::Hook::Set<Game::XAssetEntry*>(0x5BB278, Maps::EntryPool.data());
		Utils::Hook::Set<Game::XAssetEntry*>(0x5BB34C, Maps::EntryPool.data());
		Utils::Hook::Set<Game::XAssetEntry*>(0x5BB484, Maps::EntryPool.data());
		Utils::Hook::Set<Game::XAssetEntry*>(0x5BB570, Maps::EntryPool.data());
		Utils::Hook::Set<Game::XAssetEntry*>(0x5BB6B7, Maps::EntryPool.data());
		Utils::Hook::Set<Game::XAssetEntry*>(0x5BB844, Maps::EntryPool.data());
		Utils::Hook::Set<Game::XAssetEntry*>(0x5BB98D, Maps::EntryPool.data());
		Utils::Hook::Set<Game::XAssetEntry*>(0x5BBA66, Maps::EntryPool.data());
		Utils::Hook::Set<Game::XAssetEntry*>(0x5BBB8D, Maps::EntryPool.data());
		Utils::Hook::Set<Game::XAssetEntry*>(0x5BBCB1, Maps::EntryPool.data());
		Utils::Hook::Set<Game::XAssetEntry*>(0x5BBD9B, Maps::EntryPool.data());
		Utils::Hook::Set<Game::XAssetEntry*>(0x5BBE4C, Maps::EntryPool.data());
		Utils::Hook::Set<Game::XAssetEntry*>(0x5BBF14, Maps::EntryPool.data());
		Utils::Hook::Set<Game::XAssetEntry*>(0x5BBF54, Maps::EntryPool.data());
		Utils::Hook::Set<Game::XAssetEntry*>(0x5BBFB8, Maps::EntryPool.data());

		Utils::Hook::Set<Game::XAssetEntry*>(0x5BAE91, Maps::EntryPool.data() + 1);
		Utils::Hook::Set<Game::XAssetEntry*>(0x5BAEA2, Maps::EntryPool.data() + 1);

		// This is placed here in case the anticheat has been disabled!
#ifndef DEBUG
		QuickPatch::OnFrame(AntiCheat::ScanIntegrityCheck);
#endif
	}

	Maps::Maps()
	{
		// Restrict asset loading
		AssetHandler::OnLoad(Maps::LoadAssetRestrict);

		// hunk size (was 300 MiB)
		Utils::Hook::Set<DWORD>(0x64A029, 0x1C200000); // 450 MiB
		Utils::Hook::Set<DWORD>(0x64A057, 0x1C200000);

		// Intercept BSP name resolving
		Utils::Hook(0x4C5979, Maps::GetBSPName, HOOK_CALL).Install()->Quick();

		// Intercept map zone loading
		Utils::Hook(0x42C2AF, Maps::LoadMapZones, HOOK_CALL).Install()->Quick();

		// Ignore SP entities
		Utils::Hook(0x444810, Maps::IgnoreEntityStub, HOOK_JUMP).Install()->Quick();

		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_GAME_MAP_SP, 1);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_IMAGE, 7168);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_LOADED_SOUND, 2700);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_FX, 1200);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_LOCALIZE, 14000);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_XANIM, 8192);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_XMODEL, 5125);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_PHYSPRESET, 128);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_PIXELSHADER, 10000);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_VERTEXSHADER, 3072);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_MATERIAL, 8192);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_VERTEXDECL, 196);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_WEAPON, 2400);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_STRINGTABLE, 800);

		Maps::ReallocateEntryPool();

		// Dependencies
		//Maps::AddDependency("oilrig", "mp_subbase");
		//Maps::AddDependency("gulag", "mp_subbase");
		//Maps::AddDependency("invasion", "mp_rust");
		Maps::AddDependency("co_hunted", "mp_storm");
		Maps::AddDependency("mp_nuked", "iw4x_dependencies_mp");
		Maps::AddDependency("mp_bloc", "iw4x_dependencies_mp");
		Maps::AddDependency("mp_cargoship", "iw4x_dependencies_mp");
		Maps::AddDependency("mp_cross_fire", "iw4x_dependencies_mp");
		Maps::AddDependency("mp_bog_sh", "iw4x_dependencies_mp");
		Maps::AddDependency("mp_killhouse", "iw4x_dependencies_mp");

		Maps::AddDependency("^(?!mp_).*", "iw4x_dependencies_mp"); // All maps not starting with "mp_"

		Maps::AddDependency("mp_bloc_sh", "iw4x_dependencies_mp");
		Maps::AddDependency("mp_cargoship_sh", "iw4x_dependencies_mp");
		Maps::AddDependency("mp_firingrange", "iw4x_dependencies_mp");
		Maps::AddDependency("mp_shipment_long", "iw4x_dependencies_mp");
		Maps::AddDependency("mp_firingrange", "iw4x_dependencies_mp");

#if defined(DEBUG) && defined(ENABLE_DXSDK)
		Command::Add("dumpmap", [] (Command::Params)
		{
			if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled())
			{
				Logger::Print("DirectX needs to be enabled, please start a client to use this command!\n");
				return;
			}

			Game::GfxWorld* world = nullptr;
			Game::DB_EnumXAssets(Game::XAssetType::ASSET_TYPE_GFX_MAP, [] (Game::XAssetHeader header, void* world)
			{
				*reinterpret_cast<Game::GfxWorld**>(world) = header.gfxMap;
			}, &world, false);

			if (world)
			{
				Maps::ExportMap(world);
				Logger::Print("Map '%s' exported!\n", world->baseName);
			}
			else
			{
				Logger::Print("No map loaded, unable to dump anything!\n");
			}
		});
#endif
	}

	Maps::~Maps()
	{
		Maps::EntryPool.clear();
	}
}
