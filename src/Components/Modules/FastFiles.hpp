namespace Components
{
	class FastFiles : public Component
	{
	public:
		FastFiles();
		~FastFiles();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* GetName() { return "FastFiles"; };
#endif

		static void AddZonePath(std::string path);
		static std::string Current();

		static bool Exists(std::string file);

		static void LoadLocalizeZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);

	private:
		union Key
		{
			struct
			{
				unsigned char key[24];
				unsigned char iv[16];
			};

			unsigned char data[1];
		};

		static Key CurrentKey;
		static symmetric_CTR CurrentCTR;
		static unsigned char ZoneKey[];
		static std::vector<std::string> ZonePaths;
		static const char* GetZoneLocation(const char* file);
		static void LoadInitialZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);
		static void LoadDLCUIZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);
		static void LoadGfxZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);

		static void ReadVersionStub(unsigned int* version, int size);

		static void AuthLoadInitCrypto();
		static int AuthLoadInflateCompare(unsigned char* buffer, int length, unsigned char* ivValue);
		static void AuthLoadInflateDecryptBase();
		static void AuthLoadInflateDecryptBaseFunc(unsigned char* buffer);
		static int InflateInitDecrypt(z_streamp strm, const char *version, int stream_size);
	};
}
