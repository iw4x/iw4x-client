#define XFILE_MAGIC_UNSIGNED 0x3030317566665749
#define XFILE_VERSION 276
#define XFILE_VERSION_IW4X 0x78345749 // 'IW4x'

namespace Components
{
	class ZoneBuilder : public Component
	{
	public:
		class Zone
		{
		public:
			Zone(std::string zoneName);
			~Zone();

			void build();

			Utils::Stream* getBuffer();
			Utils::Memory::Allocator* getAllocator();

			bool hasPointer(const void* pointer);
			void storePointer(const void* pointer);

			template<typename T>
			inline T* getPointer(const T* pointer) { return reinterpret_cast<T*>(this->safeGetPointer(pointer)); }

			int findAsset(Game::XAssetType type, std::string name);
			Game::XAsset* getAsset(int index);
			uint32_t getAssetOffset(int index);
			Game::XAssetHeader requireAsset(Game::XAssetType type, const char* name);
			bool loadAsset(Game::XAssetType type, std::string name);

			int addScriptString(unsigned short gameIndex);
			int addScriptString(std::string str);
			int findScriptString(std::string str);

			void mapScriptString(unsigned short* gameIndex);

			void renameAsset(Game::XAssetType type, std::string asset, std::string newName);
			std::string getAssetName(Game::XAssetType type, std::string asset);

			void store(Game::XAssetHeader header);

			void incrementExternalSize(unsigned int size);

		private:
			void loadFastFiles();

			bool loadAssets();
			bool loadAsset(std::string type, std::string name);

			void saveData();
			void writeZone();

			void addBranding();

			uint32_t safeGetPointer(const void* pointer);

			int indexStart;
			unsigned int externalSize;
			Utils::Stream buffer;

			std::string zoneName;
			Utils::CSV dataMap;

			Utils::Memory::Allocator memAllocator;

			std::vector<Game::XAsset> loadedAssets;
			std::vector<std::string> scriptStrings;
			std::map<unsigned short, unsigned int> scriptStringMap;
			std::map<std::string, std::string> renameMap[Game::XAssetType::ASSET_TYPE_COUNT];
			std::map<const void*, uint32_t> pointerMap;

			Game::RawFile branding;
		};

		ZoneBuilder();
		~ZoneBuilder();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() { return "ZoneBuilder"; };
#endif

		static bool IsEnabled();

		static std::string TraceZone;
		static std::vector<std::pair<Game::XAssetType, std::string>> TraceAssets;

		static void BeginAssetTrace(std::string zone);
		static std::vector<std::pair<Game::XAssetType, std::string>> EndAssetTrace();

	private:
		static Utils::Memory::Allocator MemAllocator;
		static int StoreTexture(Game::GfxImageLoadDef **loadDef, Game::GfxImage *image);
		static void ReleaseTexture(Game::XAssetHeader header);
	};
}
