#define VERSION_ALPHA2 316
#define VERSION_ALPHA3 318//319

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

	//private:
		static int ZoneVersion;

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
	};
}
