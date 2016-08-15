namespace Components
{
	class Maps : public Component
	{
	public:
		Maps();
		~Maps();

#ifdef DEBUG
		const char* GetName() { return "Maps"; };
#endif

		static void AddDependency(std::string expression, std::string zone);

	private:
		static std::vector<Game::XAssetEntry> EntryPool;

		static std::map<std::string, std::string> DependencyList;
		static std::vector<std::string> CurrentDependencies;

		static void GetBSPName(char* buffer, size_t size, const char* format, const char* mapname);
		static void LoadAssetRestrict(Game::XAssetType type, Game::XAssetHeader asset, std::string name, bool* restrict);
		static void LoadMapZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);

		static void OverrideMapEnts(Game::MapEnts* ents);

		void ReallocateEntryPool();
	};
}
