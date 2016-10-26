namespace Components
{
	class Maps : public Component
	{
	public:
		Maps();
		~Maps();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* GetName() { return "Maps"; };
#endif

		static void AddDependency(std::string expression, std::string zone);

	private:
		static std::vector<Game::XAssetEntry> EntryPool;

		static std::vector<std::pair<std::string, std::string>> DependencyList;
		static std::vector<std::string> CurrentDependencies;

		static void GetBSPName(char* buffer, size_t size, const char* format, const char* mapname);
		static void LoadAssetRestrict(Game::XAssetType type, Game::XAssetHeader asset, std::string name, bool* restrict);
		static void LoadMapZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);

		static void OverrideMapEnts(Game::MapEnts* ents);

		static int IgnoreEntityStub(const char* entity);

		static void PatchMapLoad(const char** mapnamePtr);
		static void MapLoadStub();

#if defined(DEBUG) && defined(ENABLE_DXSDK)
		static void ExportMap(Game::GfxWorld* world);
#endif

		void ReallocateEntryPool();
	};
}
