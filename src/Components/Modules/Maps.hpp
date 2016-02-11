namespace Components
{
	class Maps : public Component
	{
	public:
		Maps();
		~Maps();
		const char* GetName() { return "Maps"; };

		static void AddDependency(std::string expression, std::string zone);

	private:
		static void* WorldMP;
		static void* WorldSP;

		static std::vector<Game::XAssetEntry> EntryPool;

		static std::map<std::string, std::string> DependencyList;
		static std::vector<std::string> CurrentDependencies;

		static void GetBSPName(char* buffer, size_t size, const char* format, const char* mapname);
		static void LoadAssetRestrict(Game::XAssetType type, Game::XAssetHeader asset, std::string name, bool* restrict);
		static void LoadMapZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);

		void ReallocateEntryPool();
	};
}
