#include "..\STDInclude.hpp"

namespace Components
{
	void* Maps::WorldMP = 0;
	void* Maps::WorldSP = 0;

	void Maps::GetBSPName(char* buffer, size_t size, const char* format, const char* mapname)
	{
		if (_strnicmp("mp_", mapname, 3))
		{
			format = "maps/%s.d3dbsp";

			Utils::Hook::Set<void*>(0x4D90B7, (char*)Maps::WorldSP + 52);
		}
		else
		{
			Utils::Hook::Set<void*>(0x4D90B7, (char*)Maps::WorldMP + 4);
		}

		_snprintf(buffer, size, format, mapname);
	}

	typedef struct
	{
		char unknown[16];
	} xAssetEntry_t;

	static xAssetEntry_t xEntries[789312];

	void ReallocXAssetEntries()
	{
		int newsize = 516 * 2048;
		//newEnts = malloc(newsize);

		// get (obfuscated) memory locations
		unsigned int origMin = 0x134CAD8;
		unsigned int origMax = 0x134CAE8;

		unsigned int difference = (unsigned int)xEntries - origMin;

		// scan the .text memory
		char* scanMin = (char*)0x401000;
		char* scanMax = (char*)0x6D7000;
		char* current = scanMin;

		for (; current < scanMax; current += 1) {
			unsigned int* intCur = (unsigned int*)current;

			// if the address points to something within our range of interest
			if (*intCur == origMin || *intCur == origMax) {
				// patch it
				*intCur += difference;
			}
		}

		*(DWORD*)0x5BAEB0 = 789312;
	}

	Maps::Maps()
	{
		// hunk size (was 300 MiB)
		Utils::Hook::Set<DWORD>(0x64A029, 0x1C200000); // 450 MiB
		Utils::Hook::Set<DWORD>(0x64A057, 0x1C200000);

		// Intercept BSP name resolving
		Utils::Hook(0x4C5979, Maps::GetBSPName, HOOK_CALL).Install()->Quick();

		Maps::WorldSP = Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_GAME_MAP_SP, 1);
		Maps::WorldMP = Utils::Hook::Get<char*>(0x4D90B7) - 4;

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

		ReallocXAssetEntries();

		AssetHandler::Restrict([] (Game::XAssetType type, Game::XAssetHeader asset, const char* name) -> bool
		{
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
		});
	}
}
