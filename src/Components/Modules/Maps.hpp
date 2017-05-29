#pragma once

namespace Components
{
	class Maps : public Component
	{
	public:
		class UserMapContainer
		{
		public:
			UserMapContainer() : wasFreed(false), hash(0) {}
			UserMapContainer(std::string _mapname) : wasFreed(false), mapname(_mapname)
			{
				ZeroMemory(&this->searchPath, sizeof this->searchPath);
				this->hash = Maps::GetUsermapHash(this->mapname);
				Game::UI_UpdateArenas();
			}

			~UserMapContainer()
			{
				this->freeIwd();
				this->clear();
			}

			unsigned int getHash() { return this->hash; }
			std::string getName() { return this->mapname; }
			bool isValid() { return !this->mapname.empty(); }
			void clear()
			{
				bool wasValid = this->isValid();
				this->mapname.clear();
				if(wasValid) Game::UI_UpdateArenas();
			}

			void loadIwd();
			void freeIwd();

			void reloadIwd();

			void handlePackfile(void* packfile);

		private:
			bool wasFreed;
			unsigned int hash;
			std::string mapname;
			Game::searchpath_t searchPath;
		};

		Maps();
		~Maps();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "Maps"; };
#endif

		static void HandleAsSPMap();
		static void AddDependency(std::string expression, std::string zone);

		static std::pair<std::string, std::string> GetTeamsForMap(std::string map);
		static std::vector<std::string> GetDependenciesForMap(std::string map);

		static std::string CurrentMainZone;
		static const char* UserMapFiles[4];

		static bool CheckMapInstalled(const char* mapname, bool error = false, bool dlcIsTrue = false);

		static UserMapContainer* GetUserMap();
		static unsigned int GetUsermapHash(std::string map);

		static Game::XAssetEntry* GetAssetEntryPool();
		static bool IsCustomMap();

	private:
		class DLC
		{
		public:
			int index;
			std::string name;
			std::vector<std::string> maps;
		};

		static bool SPMap;
		static UserMapContainer UserMap;
		static std::vector<DLC> DlcPacks;
		static std::vector<Game::XAssetEntry> EntryPool;

		static std::vector<std::pair<std::string, std::string>> DependencyList;
		static std::vector<std::string> CurrentDependencies;

		static void GetBSPName(char* buffer, size_t size, const char* format, const char* mapname);
		static void LoadAssetRestrict(Game::XAssetType type, Game::XAssetHeader asset, std::string name, bool* restrict);
		static void LoadMapZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);
		static void UnloadMapZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);

		static void OverrideMapEnts(Game::MapEnts* ents);

		static int IgnoreEntityStub(const char* entity);

		static Game::G_GlassData* GetWorldData();
		static void GetWorldDataStub();

		static void LoadRawSun();

		static void AddDlc(DLC dlc);
		static void UpdateDlcStatus();

#if defined(DEBUG) && defined(ENABLE_DXSDK)
		static void ExportMap(Game::GfxWorld* world);
#endif

		static void PrepareUsermap(const char* mapname);
		static void SpawnServerStub();
		static void LoadMapLoadscreenStub();

		static int TriggerReconnectForMap(const char* mapname);
		static void RotateCheckStub();

		static const char* LoadArenaFileStub(const char* name, char* buffer, int size);

		static void HideModel();
		static void HideModelStub();

		static Game::dvar_t* GetDistortionDvar();
		static void SetDistortionStub();

		static Game::dvar_t* GetSpecularDvar();
		static void SetSpecularStub1();
		static void SetSpecularStub2();

		void reallocateEntryPool();
	};
}
