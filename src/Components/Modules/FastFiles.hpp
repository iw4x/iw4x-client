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

	private:
		static std::vector<std::string> ZonePaths;
		static const char* GetZoneLocation(const char* file);
		static void LoadDLCUIZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);
	};
}
