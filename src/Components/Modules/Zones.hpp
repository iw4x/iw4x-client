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
		const char* GetName() { return "Zones"; };
#endif

		static void InstallPatches(int version);

		static int Version() { return Zones::ZoneVersion; };

	private:
		static int ZoneVersion;

		static int FxEffectIndex;
		static char* FxEffectStrings[64];

		static Utils::Hook LoadFxElemDefHook;
		static Utils::Hook LoadFxElemDefArrayHook;
		static Utils::Hook LoadXModelLodInfoHook;
		static Utils::Hook LoadXModelHook;
		static Utils::Hook LoadXSurfaceArrayHook;
		static Utils::Hook LoadGameWorldSpHook;
		static Utils::Hook LoadPathDataHook;
		static Utils::Hook LoadVehicleDefHook;
		static Utils::Hook Loadsnd_alias_tArrayHook;
		static Utils::Hook LoadLoadedSoundHook;
		static Utils::Hook LoadmenuDef_tHook;
		static Utils::Hook LoadFxEffectDefHook;
		static Utils::Hook LoadMaterialShaderArgumentArrayHook;
		static Utils::Hook LoadStructuredDataStructPropertyArrayHook;
		static Utils::Hook LoadPathDataTailHook;
		static Utils::Hook LoadWeaponAttachHook;
		static Utils::Hook LoadWeaponCompleteDefHook;
		static Utils::Hook LoadGfxImageHook;

		static void LoadFxElemDefArrayStub(bool atStreamStart);
		static bool LoadFxElemDefStub(bool atStreamStart, Game::FxElemDef* fxElem, int size);

		static void LoadXModelLodInfo(int i);
		static void LoadXModelLodInfoStub();
		static bool LoadXModel(bool atStreamStart, char* xmodel, int size);
		static bool LoadXSurfaceArray(bool atStreamStart, char* buffer, int size);
		static bool LoadGameWorldSp(bool atStreamStart, char* buffer);
		static bool LoadVehicleDef(bool atStreamStart, char* buffer);
		static bool Loadsnd_alias_tArray(bool atStreamStart, char* buffer, int len);
		static bool LoadLoadedSound(bool atStreamStart, char* buffer);
		static bool LoadmenuDef_t(bool atStreamStart, char* buffer, int size);
		static bool LoadFxEffectDef(bool atStreamStart, char* buffer, int size);
		static bool LoadMaterialShaderArgumentArray(bool atStreamStart, Game::MaterialShaderArgument* argument, int size);
		static bool LoadStructuredDataStructPropertyArray(bool atStreamStart, char* data, int size);
		static void LoadPathDataTail();
		static void LoadWeaponAttach();
		static void LoadWeaponAttachStuff(DWORD* varWeaponAttachStuff, int count);
		static void LoadWeaponCompleteDef();
		static bool LoadGfxImage(bool atStreamStart, char* buffer, int size);
	};
}
