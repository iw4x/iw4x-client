#include "STDInclude.hpp"

namespace Components
{
	void* Maps::WorldMP = 0;
	void* Maps::WorldSP = 0;

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
		std::string patchZone = Utils::VA("patch_%s", zoneInfo->name);
		if (FastFiles::Exists(patchZone))
		{
			data.push_back({ patchZone.data(), zoneInfo->allocFlags, zoneInfo->freeFlags });
		}

		return FastFiles::LoadLocalizeZones(data.data(), data.size(), sync);
	}

	bool Maps::LoadAssetRestrict(Game::XAssetType type, Game::XAssetHeader asset, const char* name)
	{
		if (std::find(Maps::CurrentDependencies.begin(), Maps::CurrentDependencies.end(), FastFiles::Current()) != Maps::CurrentDependencies.end())
		{
			if (type == Game::XAssetType::ASSET_TYPE_GAME_MAP_MP || type == Game::XAssetType::ASSET_TYPE_COL_MAP_MP || type == Game::XAssetType::ASSET_TYPE_GFX_MAP || type == Game::XAssetType::ASSET_TYPE_MAP_ENTS || type == Game::XAssetType::ASSET_TYPE_COM_MAP || type == Game::XAssetType::ASSET_TYPE_FX_MAP)
			{
				return false;
			}
		}

		if (type == Game::XAssetType::ASSET_TYPE_MAP_ENTS)
		{
			static std::string mapEntities;
			FileSystem::File ents(Utils::VA("%s.ents", name));
			if (ents.Exists())
			{
				mapEntities = ents.GetBuffer();
				asset.mapEnts->entitystring = mapEntities.data();
			}
		}

		return true;
	}

	void Maps::GetBSPName(char* buffer, size_t size, const char* format, const char* mapname)
	{
		if (_strnicmp("mp_", mapname, 3))
		{
			format = "maps/%s.d3dbsp";

			// Adjust pointer to GameMap_Data
			Utils::Hook::Set<void*>(0x4D90B7, Maps::WorldSP);
		}
		else
		{
			// Adjust pointer to GameMap_Data
			Utils::Hook::Set<void*>(0x4D90B7, Maps::WorldMP);
		}

		_snprintf(buffer, size, format, mapname);
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
			MessageBoxA(0, Utils::VA("Invalid regular expression: %s", expression.data()), "Warning", MB_ICONEXCLAMATION);
			return;
		}

		Maps::DependencyList[expression] = zone;
	}

	void Maps::ReallocateEntryPool()
	{
		static_assert(sizeof(Game::XAssetEntry) == 16, "XAssetEntry size mismatch");

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

		Maps::WorldSP = static_cast<char*>(Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_GAME_MAP_SP, 1)) + 52; // Skip name and other padding to reach world data
		Maps::WorldMP = Utils::Hook::Get<char*>(0x4D90B7); 

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
		Maps::AddDependency("oilrig", "mp_subbase");
		Maps::AddDependency("gulag", "mp_subbase");
		Maps::AddDependency("invasion", "mp_rust");
		//Maps::AddDependency("^(?!mp_).*", "mp_subbase"); // All maps not starting with "mp_"
	}

	Maps::~Maps()
	{
		Maps::EntryPool.clear();
	}
}
