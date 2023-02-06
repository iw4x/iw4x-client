#pragma once

#define VERSION_ALPHA2 316
#define VERSION_ALPHA3 318//319
#define VERSION_ALPHA3_DEC 319
#define VERSION_LATEST_CODO 461

namespace Components
{
	class Zones : public Component
	{
	public:
		struct FileData
		{
			std::uint32_t readPos;
			std::uint32_t len;
			std::string fileContents;
		};
		
		Zones();

		static void SetVersion(int version);

		static int Version() { return ZoneVersion; }

	private:
		static int ZoneVersion;
		
		static int FxEffectIndex;
		static char* FxEffectStrings[64];

		static std::unordered_map<int, FileData> fileDataMap;
		static std::mutex fileDataMutex;

		static bool CheckGameMapSp(int type);
		static void GameMapSpPatchStub();

		static void LoadFxElemDefArrayStub(bool atStreamStart);
		static bool LoadFxElemDefStub(bool atStreamStart, Game::FxElemDef* fxElem, int size);

		static void LoadXModelLodInfo(int i);
		static void LoadXModelLodInfoStub();
		static bool LoadXModel(bool atStreamStart, char* xmodel, int size);
		static bool LoadXSurfaceArray(bool atStreamStart, char* buffer, int size);
		static bool LoadGameWorldSp(bool atStreamStart, char* buffer, int size);
		static bool LoadVehicleDef(bool atStreamStart, char* buffer, int size);
		static bool Loadsnd_alias_tArray(bool atStreamStart, char* buffer, int len);
		static bool LoadLoadedSound(bool atStreamStart, char* buffer, int size);
		static bool LoadmenuDef_t(bool atStreamStart, char* buffer, int size);
		static bool LoadFxEffectDef(bool atStreamStart, char* buffer, int size);
		static bool LoadMaterialShaderArgumentArray(bool atStreamStart, Game::MaterialShaderArgument* argument, int size);
		static bool LoadStructuredDataStructPropertyArray(bool atStreamStart, char* data, int size);
		static void LoadPathDataTail();
		static void LoadWeaponAttach();
		static void LoadWeaponAttachStuff(DWORD* varWeaponAttachStuff, int count);
		static void LoadWeaponCompleteDef();
		static bool LoadGfxImage(bool atStreamStart, char* buffer, int size);
		static bool LoadXAsset(bool atStreamStart, char* buffer, int size);
		static bool LoadMaterialTechnique(bool atStreamStart, char* buffer, int size);
		static bool LoadMaterial(bool atStreamStart, char* buffer, int size);
		static bool LoadGfxWorld(bool atStreamStart, char* buffer, int size);
		static void Loadsunflare_t(bool atStreamStart);
		static bool LoadStatement(bool atStreamStart, char* buffer, int size);
		static void LoadWindowImage(bool atStreamStart);
		static void LoadPhysPreset(bool atStreamStart, char* buffer, int size);
		static void LoadXModelSurfs(bool atStreamStart, char* buffer, int size);
		static void LoadImpactFx(bool atStreamStart, char* buffer, int size);
		static void LoadPathNodeArray(bool atStreamStart, char* buffer, int size);
		static void LoadPathnodeConstantTail(bool atStreamStart, char* buffer, int size);
		static void LoadExpressionSupportingDataPtr(bool atStreamStart);
		static void LoadImpactFxArray();
		static int ImpactFxArrayCount();
		static void LoadPathDataConstant();
		static void GetCurrentAssetTypeStub();
		static int LoadRandomFxGarbage(bool atStreamStart, char* buffer, int size);
		static int LoadGfxXSurfaceArray(bool atStreamStart, char* buffer, int size);
		static int LoadGfxXSurfaceExtraData(bool atStreamStart);
		static int LoadGfxReflectionProbes(bool atStreamStart, char* buffer, int size);
		static void LoadGfxLightMapExtraData();
		static void LoadXModelColSurfPtr();
		static int PathDataSize();
		static int LoadMaterialTechniqueArray(bool atStreamStart, int count);
		static int LoadMapEnts(bool atStreamStart, Game::MapEnts* buffer, int size);
		static void Load_ClipInfo(bool atStreamStart);
		static int LoadClipMap(bool atStreamStart);
		static uint32_t HashCRC32StringInt(const std::string& Value, uint32_t Initial);
		static int FS_FOpenFileReadForThreadOriginal(const char*, int*, int);
		static int FS_FOpenFileReadForThreadHook(const char* file, int* filePointer, int thread);
		static int FS_ReadOriginal(void*, size_t, int);
		static int FS_ReadHook(void* buffer, size_t size, int filePointer);
		static void FS_FCloseFileOriginal(int);
		static void FS_FCloseFileHook(int filePointer);
		static std::uint32_t FS_SeekOriginal(int, int, int);
		static std::uint32_t FS_SeekHook(int fileHandle, int seekPosition, int seekOrigin);
		static void LoadMapTriggersModelPointer();
		static void LoadMapTriggersHullPointer();
		static void LoadMapTriggersSlabPointer();
		static void LoadFxWorldAsset(Game::FxWorld** asset);
		static void LoadXModelAsset(Game::XModel** asset);
		static void LoadMaterialAsset(Game::Material** asset);
		static void LoadTracerDef(bool atStreamStart, Game::TracerDef* tracer, int size);
		static void LoadTracerDefFxEffect();
		static void FixImageCategory(Game::GfxImage* image);
		static char* ParseShellShock_Stub(const char** data_p);
	};
}
