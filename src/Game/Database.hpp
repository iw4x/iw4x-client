#pragma once

namespace Game
{
	template <typename T> static void DB_ConvertOffsetToPointer(T* pointer)
	{
		Utils::Hook::Call<void(T*)>(0x4A82B0)(pointer);
	}

	template <typename T> static T** DB_InsertPointer()
	{
		static auto DB_InsertPointer_t = 0x43B290;
		T** result = nullptr;

		__asm
		{
			call DB_InsertPointer_t;
			mov result, eax;
		}

		return result;
	}

	typedef char*(*DB_AllocStreamPos_t)(int alignment);
	extern DB_AllocStreamPos_t DB_AllocStreamPos;

	typedef void(*DB_PushStreamPos_t)(unsigned int index);
	extern DB_PushStreamPos_t DB_PushStreamPos;

	typedef void(*DB_PopStreamPos_t)();
	extern DB_PopStreamPos_t DB_PopStreamPos;

	typedef void(*DB_BeginRecoverLostDevice_t)();
	extern DB_BeginRecoverLostDevice_t DB_BeginRecoverLostDevice;

	typedef void(*DB_EndRecoverLostDevice_t)();
	extern DB_EndRecoverLostDevice_t DB_EndRecoverLostDevice;

	typedef void(*DB_EnumXAssets_t)(XAssetType type, void(*)(XAssetHeader, void*), void* userdata, bool overrides);
	extern DB_EnumXAssets_t DB_EnumXAssets;

	typedef void(*DB_EnumXAssets_Internal_t)(XAssetType type, void(*)(XAssetHeader, void*), void* userdata, bool overrides);
	extern DB_EnumXAssets_Internal_t DB_EnumXAssets_Internal;

	typedef XAssetHeader(*DB_FindXAssetHeader_t)(XAssetType type, const char* name);
	extern DB_FindXAssetHeader_t DB_FindXAssetHeader;

	typedef float(*DB_GetLoadedFraction_t)();
	extern DB_GetLoadedFraction_t DB_GetLoadedFraction;

	typedef const char*(*DB_GetXAssetTypeName_t)(XAssetType type);
	extern DB_GetXAssetTypeName_t DB_GetXAssetTypeName;

	typedef int(*DB_IsXAssetDefault_t)(XAssetType type, const char* name);
	extern DB_IsXAssetDefault_t DB_IsXAssetDefault;

	typedef void(*DB_GetRawBuffer_t)(RawFile* rawfile, char* buffer, int size);
	extern DB_GetRawBuffer_t DB_GetRawBuffer;

	typedef int(*DB_GetRawFileLen_t)(RawFile* rawfile);
	extern DB_GetRawFileLen_t DB_GetRawFileLen;

	typedef void(*DB_LoadXAssets_t)(XZoneInfo* zoneInfo, unsigned int zoneCount, int sync);
	extern DB_LoadXAssets_t DB_LoadXAssets;

	typedef void(*DB_LoadXFileData_t)(char* pos, int size);
	extern DB_LoadXFileData_t DB_LoadXFileData;

	typedef void(*DB_ReadXFileUncompressed_t)(void* buffer, int size);
	extern DB_ReadXFileUncompressed_t DB_ReadXFileUncompressed;

	typedef void(*DB_ReadXFile_t)(void* buffer, int size);
	extern DB_ReadXFile_t DB_ReadXFile;

	typedef void(*DB_SetXAssetName_t)(XAsset* asset, const char* name);
	extern DB_SetXAssetName_t DB_SetXAssetName;

	typedef void(*DB_XModelSurfsFixup_t)(XModel* model);
	extern DB_XModelSurfsFixup_t DB_XModelSurfsFixup;

	typedef void(*DB_SetXAssetNameHandler_t)(XAssetHeader* header, const char* name);
	extern DB_SetXAssetNameHandler_t* DB_SetXAssetNameHandlers;

	typedef int(*DB_GetXAssetSizeHandler_t)();
	extern DB_GetXAssetSizeHandler_t* DB_GetXAssetSizeHandlers;

	typedef const char* (*DB_GetXAssetNameHandler_t)(XAssetHeader* asset);
	extern DB_GetXAssetNameHandler_t* DB_GetXAssetNameHandlers;

	typedef void(*DB_ReleaseXAssetHandler_t)(XAssetHeader header);
	extern DB_ReleaseXAssetHandler_t* DB_ReleaseXAssetHandlers;

	extern XAssetHeader* DB_XAssetPool;
	extern unsigned int* g_poolSize;

	extern XBlock** g_streamBlocks;
	extern int* g_streamPos;
	extern int* g_streamPosIndex;

	extern FastCriticalSection* db_hashCritSect;

	extern XZone* g_zones;
	extern unsigned short* db_hashTable;

	extern XAssetHeader ReallocateAssetPool(XAssetType type, unsigned int newSize);

	extern const char* DB_GetXAssetName(XAsset* asset);
	extern XAssetType DB_GetXAssetNameType(const char* name);
	extern int DB_GetZoneIndex(const std::string& name);
	extern bool DB_IsZoneLoaded(const char* zone);
	extern void DB_EnumXAssetEntries(XAssetType type, std::function<void(XAssetEntry*)> callback, bool overrides);
	extern XAssetHeader DB_FindXAssetDefaultHeaderInternal(XAssetType type);
	extern XAssetEntry* DB_FindXAssetEntry(XAssetType type, const char* name);
}
