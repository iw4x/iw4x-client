#include <STDInclude.hpp>

namespace Game
{
	DB_BeginRecoverLostDevice_t DB_BeginRecoverLostDevice = DB_BeginRecoverLostDevice_t(0x4BFF90);
	DB_EndRecoverLostDevice_t DB_EndRecoverLostDevice = DB_EndRecoverLostDevice_t(0x46B660);
	DB_EnumXAssets_t DB_EnumXAssets = DB_EnumXAssets_t(0x4B76D0);
	DB_EnumXAssets_Internal_t DB_EnumXAssets_Internal = DB_EnumXAssets_Internal_t(0x5BB0A0);
	DB_FindXAssetHeader_t DB_FindXAssetHeader = DB_FindXAssetHeader_t(0x407930);
	DB_GetRawBuffer_t DB_GetRawBuffer = DB_GetRawBuffer_t(0x4CDC50);
	DB_GetRawFileLen_t DB_GetRawFileLen = DB_GetRawFileLen_t(0x4DAA80);
	DB_GetLoadedFraction_t DB_GetLoadedFraction = DB_GetLoadedFraction_t(0x468380);
	DB_GetXAssetTypeName_t DB_GetXAssetTypeName = DB_GetXAssetTypeName_t(0x4CFCF0);
	DB_IsXAssetDefault_t DB_IsXAssetDefault = DB_IsXAssetDefault_t(0x48E6A0);
	DB_LoadXAssets_t DB_LoadXAssets = DB_LoadXAssets_t(0x4E5930);
	DB_LoadXFileData_t DB_LoadXFileData = DB_LoadXFileData_t(0x445460);
	DB_ReadXFile_t DB_ReadXFile = DB_ReadXFile_t(0x445460);
	DB_ReadXFileUncompressed_t DB_ReadXFileUncompressed = DB_ReadXFileUncompressed_t(0x4705E0);
	DB_SetXAssetName_t DB_SetXAssetName = DB_SetXAssetName_t(0x453580);
	DB_XModelSurfsFixup_t DB_XModelSurfsFixup = DB_XModelSurfsFixup_t(0x5BAC50);

	DB_SetXAssetNameHandler_t* DB_SetXAssetNameHandlers = reinterpret_cast<DB_SetXAssetNameHandler_t*>(0x7993D8);
	DB_GetXAssetNameHandler_t* DB_GetXAssetNameHandlers = reinterpret_cast<DB_GetXAssetNameHandler_t*>(0x799328);
	DB_GetXAssetSizeHandler_t* DB_GetXAssetSizeHandlers = reinterpret_cast<DB_GetXAssetSizeHandler_t*>(0x799488);
	DB_ReleaseXAssetHandler_t* DB_ReleaseXAssetHandlers = reinterpret_cast<DB_ReleaseXAssetHandler_t*>(0x799AB8);

	XAssetHeader* DB_XAssetPool = reinterpret_cast<XAssetHeader*>(0x7998A8);
	unsigned int* g_poolSize = reinterpret_cast<unsigned int*>(0x7995E8);

	XBlock** g_streamBlocks = reinterpret_cast<XBlock**>(0x16E554C);
	int* g_streamPos = reinterpret_cast<int*>(0x16E5554);
	int* g_streamPosIndex = reinterpret_cast<int*>(0x16E5578);

	FastCriticalSection* db_hashCritSect = reinterpret_cast<FastCriticalSection*>(0x16B8A54);

	XZone* g_zones = reinterpret_cast<XZone*>(0x14C0F80);
	unsigned short* db_hashTable = reinterpret_cast<unsigned short*>(0x12412B0);

	XAssetHeader ReallocateAssetPool(XAssetType type, unsigned int newSize)
	{
		const auto size = DB_GetXAssetSizeHandlers[type]();
		XAssetHeader poolEntry = {Utils::Memory::GetAllocator()->allocate(newSize * size)};
		DB_XAssetPool[type] = poolEntry;
		g_poolSize[type] = newSize;
		return poolEntry;
	}

	const char* DB_GetXAssetName(XAsset* asset)
	{
		if (!asset) return "";

		assert(asset->header.data);

		return DB_GetXAssetNameHandlers[asset->type](&asset->header);
	}

	XAssetType DB_GetXAssetNameType(const char* name)
	{
		for (int i = 0; i < ASSET_TYPE_COUNT; ++i)
		{
			XAssetType type = static_cast<XAssetType>(i);
			if (!_stricmp(DB_GetXAssetTypeName(type), name))
			{
				// Col map workaround!
				if (type == Game::ASSET_TYPE_CLIPMAP_SP)
				{
					return Game::ASSET_TYPE_CLIPMAP_MP;
				}

				return type;
			}
		}

		return ASSET_TYPE_INVALID;
	}

	int DB_GetZoneIndex(const std::string& name)
	{
		for (int i = 0; i < 32; ++i)
		{
			if (Game::g_zones[i].name == name)
			{
				return i;
			}
		}

		return -1;
	}

	bool DB_IsZoneLoaded(const char* zone)
	{
		auto zoneCount = Utils::Hook::Get<int>(0x1261BCC);
		auto* zoneIndices = reinterpret_cast<char*>(0x16B8A34);
		auto* zoneData = reinterpret_cast<char*>(0x14C0F80);

		for (int i = 0; i < zoneCount; ++i)
		{
			std::string name = zoneData + 4 + 0xA4 * (zoneIndices[i] & 0xFF);

			if (name == zone)
			{
				return true;
			}
		}

		return false;
	}

	void DB_EnumXAssetEntries(XAssetType type, std::function<void(XAssetEntry*)> callback, bool overrides)
	{
		Sys_LockRead(db_hashCritSect);

		const auto pool = Components::Maps::GetAssetEntryPool();
		for (auto hash = 0; hash < 37000; hash++)
		{
			auto hashIndex = db_hashTable[hash];
			while (hashIndex)
			{
				auto* assetEntry = &pool[hashIndex];

				if (assetEntry->asset.type == type)
				{
					callback(assetEntry);
					if (overrides)
					{
						auto overrideIndex = assetEntry->nextOverride;
						while (overrideIndex)
						{
							auto* overrideEntry = &pool[overrideIndex];
							callback(overrideEntry);
							overrideIndex = overrideEntry->nextOverride;
						}
					}
				}

				hashIndex = assetEntry->nextHash;
			}
		}

		Sys_UnlockRead(db_hashCritSect);
	}

	__declspec(naked) XAssetHeader DB_FindXAssetDefaultHeaderInternal(XAssetType /*type*/)
	{
		__asm
		{
			push eax
			pushad

			mov eax, 5BB210h
			mov edi, [esp + 28h]
			call eax

			mov [esp + 20h], eax
			popad
			pop eax

			retn
		}
	}

	__declspec(naked) XAssetEntry* DB_FindXAssetEntry(XAssetType /*type*/, const char* /*name*/)
	{
		__asm
		{
			push eax
			pushad

			mov edi, [esp + 2Ch] // name
			push edi

			mov edi, [esp + 2Ch] // type

			mov eax, 5BB1B0h
			call eax

			add esp, 4h

			mov [esp + 20h], eax
			popad
			pop eax

			retn
		}
	}
}
