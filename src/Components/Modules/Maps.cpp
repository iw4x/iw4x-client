#include "STDInclude.hpp"
#include <glm/glm.hpp>

namespace Components
{
	Maps::UserMapContainer Maps::UserMap;
	std::string Maps::CurrentMainZone;
	std::vector<std::pair<std::string, std::string>> Maps::DependencyList;
	std::vector<std::string> Maps::CurrentDependencies;

	bool Maps::SPMap;
	std::vector<Maps::DLC> Maps::DlcPacks;
	std::vector<Game::XAssetEntry> Maps::EntryPool;

	Maps::UserMapContainer* Maps::GetUserMap()
	{
		return &Maps::UserMap;
	}

	void Maps::UserMapContainer::loadIwd()
	{
		if (this->isValid() && !this->searchPath.iwd)
		{
			std::string iwdName = Utils::String::VA("%s.iwd", this->mapname.data());
			std::string path = Utils::String::VA("%s\\usermaps\\%s\\%s", Dvar::Var("fs_basepath").get<const char*>(), this->mapname.data(), iwdName.data());

			this->searchPath.iwd = Game::FS_IsShippedIWD(path.data(), iwdName.data());

			if (this->searchPath.iwd)
			{
				this->searchPath.bLocalized = false;
				this->searchPath.ignore = 0;
				this->searchPath.ignorePureCheck = 0;
				this->searchPath.language = 0;
				this->searchPath.dir = nullptr;
				this->searchPath.next = *Game::fs_searchpaths;
				*Game::fs_searchpaths = &this->searchPath;
			}
		}
	}

	void Maps::UserMapContainer::reloadIwd()
	{
		if (this->isValid() && this->wasFreed)
		{
			this->wasFreed = false;
			this->searchPath.iwd = nullptr;
			this->loadIwd();
		}
	}

	void Maps::UserMapContainer::handlePackfile(void* packfile)
	{
		if (this->isValid() && this->searchPath.iwd == packfile)
		{
			this->wasFreed = true;
		}
	}

	void Maps::UserMapContainer::freeIwd()
	{
		if (this->isValid() && this->searchPath.iwd && !this->wasFreed)
		{
			this->wasFreed = true;

			// Unchain our searchpath
			for (Game::searchpath_t** pathPtr = Game::fs_searchpaths; *pathPtr; pathPtr = &(*pathPtr)->next)
			{
				if (*pathPtr == &this->searchPath)
				{
					*pathPtr = (*pathPtr)->next;
					break;
				}
			}

			Game::unzClose(this->searchPath.iwd->handle);

			auto _free = Utils::Hook::Call<void(void*)>(0x6B5CF2);
			_free(this->searchPath.iwd->buildBuffer);
			_free(this->searchPath.iwd);

			ZeroMemory(&this->searchPath, sizeof this->searchPath);
		}
	}

	void Maps::UnloadMapZones(Game::XZoneInfo* zoneInfo, unsigned int zoneCount, int sync)
	{
		Game::DB_LoadXAssets(zoneInfo, zoneCount, sync);

		if (Maps::UserMap.isValid())
		{
			Maps::UserMap.freeIwd();
			Maps::UserMap.clear();
		}
	}

	void Maps::LoadMapZones(Game::XZoneInfo* zoneInfo, unsigned int zoneCount, int sync)
	{
		if (!zoneInfo) return;

		Maps::SPMap = false;
		Maps::CurrentMainZone = zoneInfo->name;

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

		Utils::Memory::Allocator allocator;
		auto teams = Maps::GetTeamsForMap(Maps::CurrentMainZone);

		auto dependencies = Maps::GetDependenciesForMap(Maps::CurrentMainZone);
		Utils::Merge(&Maps::CurrentDependencies, dependencies.data(), dependencies.size());

		std::vector<Game::XZoneInfo> data;
		Utils::Merge(&data, zoneInfo, zoneCount);

		Game::XZoneInfo team;
		team.allocFlags = zoneInfo->allocFlags;
		team.freeFlags = zoneInfo->freeFlags;

		team.name = allocator.duplicateString(Utils::String::VA("iw4x_team_%s", teams.first.data()));
		data.push_back(team);

		team.name = allocator.duplicateString(Utils::String::VA("iw4x_team_%s", teams.second.data()));
		data.push_back(team);

		for (unsigned int i = 0; i < Maps::CurrentDependencies.size(); ++i)
		{
			Game::XZoneInfo info;

			info.name = (&Maps::CurrentDependencies[i])->data();
			info.allocFlags = zoneInfo->allocFlags;
			info.freeFlags = zoneInfo->freeFlags;

			data.push_back(info);
		}

		// Load patch files
		std::string patchZone = Utils::String::VA("patch_%s", zoneInfo->name);
		if (FastFiles::Exists(patchZone))
		{
			data.push_back({patchZone.data(), zoneInfo->allocFlags, zoneInfo->freeFlags});
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
		Game::DB_EnumXAssets_Internal(Game::XAssetType::ASSET_TYPE_CLIPMAP_PVS, callback, ents, true);
		Game::DB_EnumXAssets_Internal(Game::XAssetType::ASSET_TYPE_CLIPMAP, callback, ents, true);
	}

	void Maps::LoadAssetRestrict(Game::XAssetType type, Game::XAssetHeader asset, std::string name, bool* restrict)
	{
		if (std::find(Maps::CurrentDependencies.begin(), Maps::CurrentDependencies.end(), FastFiles::Current()) != Maps::CurrentDependencies.end()
			&& (FastFiles::Current() != "mp_shipment_long" || Maps::CurrentMainZone != "mp_shipment")) // Shipment is a special case
		{
			if (type == Game::XAssetType::ASSET_TYPE_CLIPMAP_PVS || type == Game::XAssetType::ASSET_TYPE_CLIPMAP || type == Game::XAssetType::ASSET_TYPE_GAMEWORLD_SP || type == Game::XAssetType::ASSET_TYPE_GAMEWORLD_MP || type == Game::XAssetType::ASSET_TYPE_GFXWORLD || type == Game::XAssetType::ASSET_TYPE_MAP_ENTS || type == Game::XAssetType::ASSET_TYPE_COMWORLD || type == Game::XAssetType::ASSET_TYPE_FX_MAP)
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
			if ((!strstr(name.data(), "_mp") && name != "none" && name != "destructible_car") || Zones::Version() >= VERSION_ALPHA2)
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
			if (Flags::HasFlag("dump"))
			{
				Utils::IO::WriteFile(Utils::String::VA("raw/%s.ents", name), asset.mapEnts->entityString);
			}

			static std::string mapEntities;
			FileSystem::File ents(name + ".ents");
			if (ents.exists())
			{
				mapEntities = ents.getBuffer();
				asset.mapEnts->entityString = const_cast<char*>(mapEntities.data());
				asset.mapEnts->numEntityChars = mapEntities.size() + 1;
			}

			// Apply new mapEnts
			// This doesn't work, entities are spawned before the patch file is loaded
			//Maps::OverrideMapEnts(asset.mapEnts);
		}

		// This is broken
		if ((type == Game::XAssetType::ASSET_TYPE_MENU || type == Game::XAssetType::ASSET_TYPE_MENULIST) && Zones::Version() >= 359)
		{
			*restrict = true;
			return;
		}
	}

	Game::G_GlassData* Maps::GetWorldData()
	{
		Logger::Print("Waiting for database...\n");
		while (!Game::Sys_IsDatabaseReady()) std::this_thread::sleep_for(100ms);

		if (!Utils::String::StartsWith(Maps::CurrentMainZone, "mp_") || Maps::SPMap)
		{
			return Game::DB_XAssetPool[Game::XAssetType::ASSET_TYPE_GAMEWORLD_SP].gameWorldSp[0].data;
		}
		else
		{
			return Game::DB_XAssetPool[Game::XAssetType::ASSET_TYPE_GAMEWORLD_MP].gameWorldMp[0].data;
		}
	}

	__declspec(naked) void Maps::GetWorldDataStub()
	{
		__asm
		{
			push eax
			pushad

			call Maps::GetWorldData

			mov [esp + 20h], eax
			popad
			pop eax

			retn
		}
	}

	void Maps::LoadRawSun()
	{
		Game::R_FlushSun();

		Game::GfxWorld* world = *reinterpret_cast<Game::GfxWorld**>(0x66DEE94);

		if (FileSystem::File(Utils::String::VA("sun/%s.sun", Maps::CurrentMainZone.data())).exists())
		{
			Game::R_LoadSunThroughDvars(Maps::CurrentMainZone.data(), &world->sun);
		}
	}

	void Maps::GetBSPName(char* buffer, size_t size, const char* format, const char* mapname)
	{
		if (!Utils::String::StartsWith(mapname, "mp_") && !Utils::String::StartsWith(mapname, "zm_"))
		{
			format = "maps/%s.d3dbsp";
		}

		// Redirect shipment to shipment long
		if (mapname == "mp_shipment"s)
		{
			mapname = "mp_shipment_long";
		}

		_snprintf_s(buffer, size, size, format, mapname);
	}

	void Maps::HandleAsSPMap()
	{
		Maps::SPMap = true;
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
			MessageBoxA(nullptr, Utils::String::VA("Invalid regular expression: %s", expression.data()), "Warning", MB_ICONEXCLAMATION);
			return;
		}

		Maps::DependencyList.push_back({expression, zone});
	}

	int Maps::IgnoreEntityStub(const char* entity)
	{
		return (Utils::String::StartsWith(entity, "dyn_") || Utils::String::StartsWith(entity, "node_") || Utils::String::StartsWith(entity, "actor_"));
	}

	std::vector<std::string> Maps::GetDependenciesForMap(std::string map)
	{
		for (int i = 0; i < *Game::arenaCount; ++i)
		{
			Game::newMapArena_t* arena = &ArenaLength::NewArenas[i];
			if (arena->mapName == map)
			{
				for (int j = 0; j < ARRAYSIZE(arena->keys); ++j)
				{
					if (arena->keys[j] == "dependency"s)
					{
						return Utils::String::Explode(arena->values[j], ' ');
					}
				}
			}
		}

		return {};
	}

	std::pair<std::string, std::string> Maps::GetTeamsForMap(std::string map)
	{
		std::string team_axis = "opforce_composite";
		std::string team_allies = "us_army";

		for (int i = 0; i < *Game::arenaCount; ++i)
		{
			Game::newMapArena_t* arena = &ArenaLength::NewArenas[i];
			if (arena->mapName == map)
			{
				for (int j = 0; j < ARRAYSIZE(arena->keys); ++j)
				{
					if (arena->keys[j] == "allieschar"s)
					{
						team_allies = arena->values[j];
					}
					else if (arena->keys[j] == "axischar"s)
					{
						team_axis = arena->values[j];
					}
				}

				break;
			}
		}

		return {team_axis, team_allies};
	}

	void Maps::PrepareUsermap(const char* mapname)
	{
		if (Maps::UserMap.isValid())
		{
			Maps::UserMap.freeIwd();
			Maps::UserMap.clear();
		}

		if (Utils::IO::DirectoryExists(Utils::String::VA("usermaps/%s", mapname)) && Utils::IO::FileExists(Utils::String::VA("usermaps/%s/%s.ff", mapname, mapname)))
		{
			Maps::UserMap = Maps::UserMapContainer(mapname);
			Maps::UserMap.loadIwd();
		}
		else
		{
			Maps::UserMap.clear();
		}
	}

	unsigned int Maps::GetUsermapHash(std::string map)
	{
		if (Utils::IO::DirectoryExists(Utils::String::VA("usermaps/%s", map.data())))
		{
			std::string zoneHash;
			std::string zonePath = Utils::String::VA("usermaps/%s/%s.ff", map.data(), map.data());
			if (Utils::IO::FileExists(zonePath))
			{
				zoneHash = Utils::Cryptography::SHA256::Compute(Utils::IO::ReadFile(zonePath));
			}

			std::string iwdHash;
			std::string iwdPath = Utils::String::VA("usermaps/%s/%s.iwd", map.data(), map.data());
			if (Utils::IO::FileExists(iwdPath))
			{
				iwdHash = Utils::Cryptography::SHA256::Compute(Utils::IO::ReadFile(iwdPath));
			}

			return Utils::Cryptography::JenkinsOneAtATime::Compute(zoneHash + iwdHash);
		}

		return 0;
	}

	int Maps::TriggerReconnectForMap(const char* mapname)
	{
		Theatre::StopRecording();

		if(!Maps::CheckMapInstalled(mapname, false, true))
		{
			// Reconnecting forces the client to download the new map
			Command::Execute("disconnect", false);
			Command::Execute("awaitDatabase", false); // Wait for the database to load
			Command::Execute("wait 100", false);
			Command::Execute("openmenu popup_reconnectingtoparty", false);
			Command::Execute("wait 8000", false); // Seems like 8000ms?
			Command::Execute("closemenu popup_reconnectingtoparty", false);
			Command::Execute("reconnect", false);
			return true;
		}

		return false;
	}

	__declspec(naked) void Maps::RotateCheckStub()
	{
		__asm
		{
			push eax
			pushad

			push [esp + 28h]
			call Maps::TriggerReconnectForMap
			add esp, 4h

			mov[esp + 20h], eax

			popad
			pop eax

			test eax, eax
			jnz skipRotation

			push 487C50h // Rotate map

		skipRotation:
			retn
		}
	}

	__declspec(naked) void Maps::SpawnServerStub()
	{
		__asm
		{
			pushad

			push [esp + 24h]
			call Maps::PrepareUsermap
			pop eax

			popad

			push 4A7120h // SV_SpawnServer
			retn
		}
	}

	__declspec(naked) void Maps::LoadMapLoadscreenStub()
	{
		__asm
		{
			pushad

			push [esp + 24h]
			call Maps::PrepareUsermap
			pop eax

			popad

			push 4D8030h // LoadMapLoadscreen
			retn
		}
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
		map.append(Utils::String::VA("o %s\n", world->baseName));
		map.append(Utils::String::VA("mtllib %s.mtl\n\n", world->baseName));

		Logger::Print("Writing vertices...\n");
		for (unsigned int i = 0; i < world->draw.vertexCount; ++i)
		{
			float x = world->draw.vd.vertices[i].xyz[1];
			float y = world->draw.vd.vertices[i].xyz[2];
			float z = world->draw.vd.vertices[i].xyz[0];

			map.append(Utils::String::VA("v %.6f %.6f %.6f\n", x, y, z));
		}

		map.append("\n");

		Logger::Print("Writing texture coordinates...\n");
		for (unsigned int i = 0; i < world->draw.vertexCount; ++i)
		{
			map.append(Utils::String::VA("vt %.6f %.6f\n", world->draw.vd.vertices[i].texCoord[0], -world->draw.vd.vertices[i].texCoord[1]));
		}

		Logger::Print("Searching materials...\n");
		int materialCount = 0;
		Game::Material** materials = allocator.allocateArray<Game::Material*>(world->dpvs.staticSurfaceCount);

		for (unsigned int i = 0; i < world->dpvs.staticSurfaceCount; ++i)
		{
			bool isNewMat = true;

			for (int j = 0; j < materialCount; ++j)
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

		Utils::IO::CreateDir(Utils::String::VA("raw/mapdump/%s/textures", world->baseName));
		mtl.append(Utils::String::VA("# Material Count: %d\n", materialCount));

		Logger::Print("Exporting materials and faces...\n");
		for (int m = 0; m < materialCount; ++m)
		{
			std::string name = materials[m]->name;

			auto pos = name.find_last_of("/");
			if (pos != std::string::npos)
			{
				name = name.substr(pos + 1);
			}

			map.append(Utils::String::VA("\nusemtl %s\n", name.data()));
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

			std::string _name = Utils::String::VA("raw/mapdump/%s/textures/%s.png", world->baseName, image->name);
			D3DXSaveTextureToFile(std::wstring(_name.begin(), _name.end()).data(), D3DXIFF_PNG, image->map, NULL);

			mtl.append(Utils::String::VA("\nnewmtl %s\n", name.data()));
			mtl.append("Ka 1.0000 1.0000 1.0000\n");
			mtl.append("Kd 1.0000 1.0000 1.0000\n");
			mtl.append("illum 1\n");
			mtl.append(Utils::String::VA("map_Ka textures/%s.png\n", image->name));
			mtl.append(Utils::String::VA("map_Kd textures/%s.png\n", image->name));

			for (unsigned int i = 0; i < world->dpvs.staticSurfaceCount; ++i)
			{
				if (world->dpvs.surfaces[i].material != materials[m])
					continue;

				int vertOffset = world->dpvs.surfaces[i].tris.firstVertex + 1;//+1 cus obj starts at 1
				int indexOffset = world->dpvs.surfaces[i].tris.baseIndex;
				for (unsigned short j = 0; j < world->dpvs.surfaces[i].tris.triCount; ++j)
				{
					int a = world->draw.indices[indexOffset + j * 3 + 0] + vertOffset;
					int b = world->draw.indices[indexOffset + j * 3 + 1] + vertOffset;
					int c = world->draw.indices[indexOffset + j * 3 + 2] + vertOffset;

					map.append(Utils::String::VA("f %d/%d %d/%d %d/%d\n", a, a, b, b, c, c));
				}
			}
		}

		Logger::Print("Writing final files...\n");
		Utils::IO::WriteFile(Utils::String::VA("raw/mapdump/%s/%s.mtl", world->baseName, world->baseName), mtl);
		Utils::IO::WriteFile(Utils::String::VA("raw/mapdump/%s/%s.obj", world->baseName, world->baseName), map);
	}
#endif

	void Maps::AddDlc(Maps::DLC dlc)
	{
		for (auto& pack : Maps::DlcPacks)
		{
			if (pack.index == dlc.index)
			{
				pack.maps = dlc.maps;
				Maps::UpdateDlcStatus();
				return;
			}
		}

		Dvar::Register<bool>(Utils::String::VA("isDlcInstalled_%d", dlc.index), false, Game::DVAR_FLAG_USERCREATED | Game::DVAR_FLAG_WRITEPROTECTED, "");

		Maps::DlcPacks.push_back(dlc);
		Maps::UpdateDlcStatus();
	}

	void Maps::UpdateDlcStatus()
	{
		bool hasAllDlcs = true;
		std::vector<bool> hasDlc;
		for (auto& pack : Maps::DlcPacks)
		{
			bool hasAllMaps = true;
			for (auto map : pack.maps)
			{
				if (!FastFiles::Exists(map))
				{
					hasAllMaps = false;
					hasAllDlcs = false;
					break;
				}
			}

			hasDlc.push_back(hasAllMaps);
			Dvar::Var(Utils::String::VA("isDlcInstalled_%d", pack.index)).setRaw(hasAllMaps ? 1 : 0);
		}

		// Must have all of dlc 3 to 5 or it causes issues
		static bool sentMessage = false;
		if (hasDlc.size() >= 5 && (hasDlc[2] || hasDlc[3] || hasDlc[4]) && (!hasDlc[2] || !hasDlc[3] || !hasDlc[4]) && !sentMessage)
		{
			StartupMessages::AddMessage("Warning:\n You only have some of DLCs 3-5 which are all required to be installed to work. There may be issues with those maps.");
			sentMessage = true;
		}

		Dvar::Var("isDlcInstalled_All").setRaw(hasAllDlcs ? 1 : 0);
	}

	void Maps::reallocateEntryPool()
	{
		AssertSize(Game::XAssetEntry, 16);

		Maps::EntryPool.clear();

		if (ZoneBuilder::IsEnabled())
		{
			Maps::EntryPool.resize(1183968);
		}
		else
		{
			Maps::EntryPool.resize(789312);
		}

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
	}

	// dlcIsTrue serves as a check if the map is a custom map and if it's missing
	bool Maps::CheckMapInstalled(const char* mapname, bool error, bool dlcIsTrue)
	{
		if (FastFiles::Exists(mapname)) return true;

		for (auto& pack : Maps::DlcPacks)
		{
			for (auto map : pack.maps)
			{
				if (map == std::string(mapname))
				{
					if (error)
					{
						Components::Logger::SoftError("Missing DLC pack %s (%d) containing map %s (%s).\nPlease download it to play this map.",
							pack.name.data(), pack.index, Game::UI_LocalizeMapName(mapname), mapname);
					}

					return dlcIsTrue;
				}
			}
		}

		if (error) Components::Logger::SoftError("Missing map file %s.\nYou may have a damaged installation or are attempting to load a non-existant map.", mapname);
		return false;
	}

	Maps::Maps()
	{
		Dvar::OnInit([]()
			{
				bool value = false;
#ifdef DEBUG
			value = true;
#endif
				Dvar::Register<bool>("r_disableModelWorkaround", value, 0, "Disable static model drawing workaround for custom maps");
			});

		// Hook R_ClearDpvsSceneView to force drawing all models in front of us
		Utils::Hook(0x50EF39, []()
		            {
			            Utils::Hook::Call<void()>(0x518530)(); // R_ClearDpvsSceneView

			            Game::GfxWorld*& gameWorld = *reinterpret_cast<Game::GfxWorld**>(0x66DEE94);
			            if (!Game::CL_IsCgameInitialized() || !gameWorld || Dvar::Var("r_disableModelWorkaround").get<bool>() || gameWorld->checksum != 0xDEADBEEF) return;

			            Game::vec3_t _forward, _right;
			            Game::AngleVectors(reinterpret_cast<float*>(0x85F650), _forward, _right, nullptr);

			            glm::vec2 right(_right[0], _right[1]);
			            glm::vec2 forward = glm::normalize(glm::vec2(_forward[0], _forward[1]));

			            float* _selfOrigin = reinterpret_cast<float*>(0x85B708);
			            glm::vec2 selfOrigin(_selfOrigin[0], _selfOrigin[1]);
			            selfOrigin -= (forward * 200.0f); // Move 200 units back

			            for (unsigned int i = 0; i < gameWorld->dpvs.smodelCount; ++i)
			            {
				            float* _origin = gameWorld->dpvs.smodelDrawInsts[i].placement.origin;
				            glm::vec2 modelOrigin(_origin[0], _origin[1]);

				            if ((selfOrigin - modelOrigin).length() <= gameWorld->dpvs.smodelDrawInsts[i].cullDist * 1.0f)
				            {
					            // If matrix is singular just draw the models
					            glm::mat2x2 matrix(right[0], -(forward[0]), right[1], -(forward[1]));
					            if (glm::determinant(matrix) != 0)
					            {
						            glm::mat2x2 invMatrix = glm::inverse(matrix);
						            glm::vec2 solve = modelOrigin - selfOrigin;
						            glm::vec2 result = invMatrix * solve;
						            glm::vec2 path = modelOrigin - (selfOrigin + (result[0] * right));

						            // Compare signs and skip to the next model if they don't equal
						            if ((path[0] < 0) == (forward[0] >= 0) && (path[1] < 0) == (forward[1] >= 0)) continue;
					            }

					            gameWorld->dpvs.smodelVisData[0][i] = 1;
					            gameWorld->dpvs.smodelVisData[1][i] = 1;
					            gameWorld->dpvs.smodelVisData[2][i] = 1;
				            }
			            }
		            }, HOOK_CALL).install()->quick();

		Dvar::OnInit([] ()
			{
				Dvar::Register<bool>("isDlcInstalled_All", false, Game::DVAR_FLAG_USERCREATED | Game::DVAR_FLAG_WRITEPROTECTED, "");

				Maps::AddDlc({1, "Stimulus Pack", {"mp_complex", "mp_compact", "mp_storm", "mp_overgrown", "mp_crash"}});
				Maps::AddDlc({2, "Resergence Pack", {"mp_abandon", "mp_vacant", "mp_trailerpark", "mp_strike", "mp_fuel2"}});
				Maps::AddDlc({3, "Nuketown", {"mp_nuked"}});
				Maps::AddDlc({4, "Classics Pack", {"mp_cross_fire", "mp_cargoship", "mp_bloc"}});
				Maps::AddDlc({5, "Classics Pack", {"mp_killhouse", "mp_bog_sh"}});
				Maps::AddDlc({6, "Freighter", {"mp_cargoship_sh"}});
				Maps::AddDlc({7, "Resurrection Pack", {"mp_shipment_long", "mp_rust_long", "mp_firingrange"}});
				Maps::AddDlc({8, "Recycled Pack", {"mp_bloc_sh", "mp_crash_tropical", "mp_estate_tropical", "mp_fav_tropical", "mp_storm_spring"}});

				Maps::UpdateDlcStatus();

				UIScript::Add("downloadDLC", [] (UIScript::Token token)
				              {
					              int dlc = token.get<int>();

					              for (auto pack : Maps::DlcPacks)
					              {
						              if (pack.index == dlc)
						              {
							              News::LaunchUpdater(Utils::String::VA("-dlc %i -c", pack.index));
							              //ShellExecuteA(nullptr, "open", pack.url.data(), nullptr, nullptr, SW_SHOWNORMAL);
							              return;
						              }
					              }

					              Game::ShowMessageBox(Utils::String::VA("DLC %d does not exist!", dlc), "ERROR");
				              });
			});

		// Restrict asset loading
		AssetHandler::OnLoad(Maps::LoadAssetRestrict);

		// hunk size (was 300 MiB)
		Utils::Hook::Set<DWORD>(0x64A029, 0x1C200000); // 450 MiB
		Utils::Hook::Set<DWORD>(0x64A057, 0x1C200000);

#if DEBUG
		// Hunk debugging
		Utils::Hook::Set<BYTE>(0x4FF57B, 0xCC);
		Utils::Hook::Nop(0x4FF57C, 4);
#endif

		// Intercept BSP name resolving
		Utils::Hook(0x4C5979, Maps::GetBSPName, HOOK_CALL).install()->quick();

		// Intercept map zone loading/unloading
		Utils::Hook(0x42C2AF, Maps::LoadMapZones, HOOK_CALL).install()->quick();
		Utils::Hook(0x60B477, Maps::UnloadMapZones, HOOK_CALL).install()->quick();

		// Ignore SP entities
		Utils::Hook(0x444810, Maps::IgnoreEntityStub, HOOK_JUMP).install()->quick();

		// WorldData pointer replacement
		Utils::Hook(0x4D90B6, Maps::GetWorldDataStub, HOOK_CALL).install()->quick();

		// Allow loading raw suns
		Utils::Hook(0x51B46A, Maps::LoadRawSun, HOOK_CALL).install()->quick();

		// Intercept map loading for usermap initialization
		Utils::Hook(0x6245E3, Maps::SpawnServerStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x62493E, Maps::SpawnServerStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x42CF58, Maps::LoadMapLoadscreenStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x487CDD, Maps::LoadMapLoadscreenStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x4CA3E9, Maps::LoadMapLoadscreenStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x5A9D51, Maps::LoadMapLoadscreenStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x5B34DD, Maps::LoadMapLoadscreenStub, HOOK_CALL).install()->quick();

		// Download the map before a maprotation if necessary
		// Conflicts with Theater's SV map rotation check, but this one is safer!
		Utils::Hook(0x5AA91C, Maps::RotateCheckStub, HOOK_CALL).install()->quick();

		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_GAMEWORLD_SP, 1);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_IMAGE, 7168);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_LOADED_SOUND, 2700);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_FX, 1200);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_LOCALIZE_ENTRY, 14000);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_XANIMPARTS, 8192);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_XMODEL, 5125);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_PHYSPRESET, 128);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_PIXELSHADER, 10000);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_VERTEXSHADER, 3072);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_MATERIAL, 8192);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_VERTEXDECL, 196);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_WEAPON, WEAPON_LIMIT);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_STRINGTABLE, 800);
		Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_IMPACT_FX, 8);

		this->reallocateEntryPool();

		// Dependencies
		//Maps::AddDependency("oilrig", "mp_subbase");
		//Maps::AddDependency("gulag", "mp_subbase");
		//Maps::AddDependency("invasion", "mp_rust");
		//Maps::AddDependency("co_hunted", "mp_storm");
		//Maps::AddDependency("mp_shipment", "mp_shipment_long");

#if defined(DEBUG) && defined(ENABLE_DXSDK)
		Command::Add("dumpmap", [] (Command::Params*)
		{
			if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled())
			{
				Logger::Print("DirectX needs to be enabled, please start a client to use this command!\n");
				return;
			}

			Game::GfxWorld* world = nullptr;
			Game::DB_EnumXAssets(Game::XAssetType::ASSET_TYPE_GFXWORLD, [] (Game::XAssetHeader header, void* world)
			{
				*reinterpret_cast<Game::GfxWorld**>(world) = header.gfxWorld;
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
		Maps::DlcPacks.clear();
		Maps::DependencyList.clear();
		Maps::CurrentMainZone.clear();
		Maps::CurrentDependencies.clear();

		Maps::EntryPool.clear();
	}
}
