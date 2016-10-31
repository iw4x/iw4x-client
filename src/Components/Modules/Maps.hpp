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

		static void HandleAsSPMap() { IsSPMap = true; }
		static void AddDependency(std::string expression, std::string zone);

	private:
		class DLC
		{
		public:
			int index;
			std::string url;
			std::vector<std::string> maps;
		};

		static bool IsSPMap;
		static std::vector<DLC> DlcPacks;
		static std::vector<Game::XAssetEntry> EntryPool;

		static std::string CurrentMainZone;
		static std::vector<std::pair<std::string, std::string>> DependencyList;
		static std::vector<std::string> CurrentDependencies;

		static void GetBSPName(char* buffer, size_t size, const char* format, const char* mapname);
		static void LoadAssetRestrict(Game::XAssetType type, Game::XAssetHeader asset, std::string name, bool* restrict);
		static void LoadMapZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);

		static void OverrideMapEnts(Game::MapEnts* ents);

		static int IgnoreEntityStub(const char* entity);

		static Game::GameMap_Data** GetWorldData();
		static void GetWorldDataStub();

		static void AddDlc(DLC dlc);
		static void UpdateDlcStatus();

#if defined(DEBUG) && defined(ENABLE_DXSDK)
		static void ExportMap(Game::GfxWorld* world);
#endif

		void ReallocateEntryPool();
	};
}
