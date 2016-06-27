namespace Components
{
	class FastFiles : public Component
	{
	public:
		FastFiles();
		~FastFiles();
		const char* GetName() { return "FastFiles"; };

		static void AddZonePath(std::string path);
		static std::string Current();

		static bool Exists(std::string file);

		static void LoadLocalizeZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);

	private:
		static std::vector<std::string> ZonePaths;
		static const char* GetZoneLocation(const char* file);
		static void LoadInitialZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);
		static void LoadDLCUIZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);
		static void LoadGfxZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);

		static void ReadVersionStub(unsigned int* version, int size);
	};
}
