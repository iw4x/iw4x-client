#define VERSION_ALPHA2 316
#define VERSION_ALPHA3 318//319
#define VERSION_ALPHA3_DEC 319

namespace Components
{
	class Zones : public Component
	{
	public:
		Zones();
		~Zones();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() { return "Zones"; };
#endif

		static void SetVersion(int version);

		static int Version() { return Zones::ZoneVersion; };

	private:
		static int ZoneVersion;

		static int FxEffectIndex;
		static char* FxEffectStrings[64];

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
		static int PathDataSize();
	};
}
