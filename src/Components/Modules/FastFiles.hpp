#pragma once

namespace Components
{
	class FastFiles : public Component
	{
	public:
		FastFiles();

		static void AddZonePath(const std::string& path);
		static std::string Current();
		static bool Ready();
		static bool Exists(const std::string& file);

		static void LoadLocalizeZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);

		static float GetFullLoadedFraction();

		static unsigned char ZoneKey[1191];

		static symmetric_CTR CurrentCTR;

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

		static unsigned int CurrentZone;
		static unsigned int MaxZones;

		static bool IsIW4xZone;
		static bool StreamRead;

		static char LastByteRead;

		static Dvar::Var g_loadingInitialZones;

		static Key CurrentKey;
		static std::vector<std::string> ZonePaths;
		static const char* GetZoneLocation(const char* file);
		static void LoadInitialZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);
		static void LoadDLCUIZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);
		static void LoadGfxZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);

		static void ReadHeaderStub(unsigned int* header, int size);
		static void ReadVersionStub(unsigned int* version, int size);

		static void ReadXFileHeader(void* buffer, int size);

		static void AuthLoadInitCrypto();
		static int AuthLoadInflateCompare(unsigned char* buffer, int length, unsigned char* ivValue);
		static void AuthLoadInflateDecryptBase();
		static void AuthLoadInflateDecryptBaseFunc(unsigned char* buffer);

		static void LoadZonesStub(Game::XZoneInfo *zoneInfo, unsigned int zoneCount);

		static void ReadXFile(void* buffer, int size);
		static void ReadXFileStub(char* buffer, int size);

#ifdef DEBUG
		static void LogStreamRead(int len);
#endif

		static void Load_XSurfaceArray(int atStreamStart, int count);

	};
}
