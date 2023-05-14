#pragma once

#include <api.hpp>

#define XFILE_MAGIC_UNSIGNED 0x3030317566665749
#define XFILE_VERSION 276

#define XFILE_HEADER_IW4X 0x78345749 // 'IW4x'
#define XFILE_VERSION_IW4X 3

namespace Components
{
	class ZoneBuilder : public Component
	{
	public:
		class Zone
		{
		public:
			class AssetRecursionMarker
			{
			public:
				AssetRecursionMarker(Zone* _builder) : builder(_builder)
				{
					this->builder->increaseAssetDepth();
				}

				~AssetRecursionMarker()
				{
					this->builder->decreaseAssetDepth();
				}

			private:
				Zone* builder;
			};

			Zone(const std::string& zoneName);
			~Zone();

			void build();

			Utils::Stream* getBuffer();
			Utils::Memory::Allocator* getAllocator();
			iw4of::api* getIW4OfApi();

			bool hasPointer(const void* pointer);
			void storePointer(const void* pointer);

			template<typename T>
			T* getPointer(const T* pointer) { return reinterpret_cast<T*>(this->safeGetPointer(pointer)); }

			int findAsset(Game::XAssetType type, std::string name);
			Game::XAssetHeader findSubAsset(Game::XAssetType type, std::string name);
			Game::XAsset* getAsset(int index);
			uint32_t getAssetTableOffset(int index);

			bool hasAlias(Game::XAsset asset);
			Game::XAssetHeader saveSubAsset(Game::XAssetType type, void* ptr);
			bool loadAssetByName(Game::XAssetType type, const std::string& name, bool isSubAsset = true);
			bool loadAsset(Game::XAssetType type, void* data, bool isSubAsset = true);

			int addScriptString(unsigned short gameIndex);
			int addScriptString(const std::string& str);
			int findScriptString(const std::string& str);
			void addRawAsset(Game::XAssetType type, void* ptr);

			void mapScriptString(unsigned short& gameIndex);

			void renameAsset(Game::XAssetType type, const std::string& asset, const std::string& newName);
			std::string getAssetName(Game::XAssetType type, const std::string& asset);

			void store(Game::XAssetHeader header);

			void incrementExternalSize(unsigned int size);

			void increaseAssetDepth() { ++this->assetDepth; }
			void decreaseAssetDepth() { --this->assetDepth; }
			bool isPrimaryAsset() { return this->assetDepth <= 1; }

		private:
			void loadFastFiles() const;

			bool loadAssets();
			bool loadAssetByName(const std::string& type, std::string name, bool isSubAsset = true);

			void saveData();
			void writeZone();

			unsigned int getAlias(Game::XAsset asset);
			void storeAlias(Game::XAsset asset);

			void addBranding();

			iw4of::params_t getIW4OfApiParams();

			uint32_t safeGetPointer(const void* pointer);

			int indexStart;
			unsigned int externalSize;
			Utils::Stream buffer;
			iw4of::api iw4ofApi;

			std::string zoneName;
			Utils::CSV dataMap;

			Utils::Memory::Allocator memAllocator;

			std::vector<Game::XAsset> loadedAssets;
			std::vector<Game::XAsset> markedAssets;
			std::vector<Game::XAsset> loadedSubAssets;
			std::vector<std::string> scriptStrings;

			std::map<unsigned short, unsigned int> scriptStringMap;

			std::map<std::string, std::string> renameMap[Game::XAssetType::ASSET_TYPE_COUNT];

			std::map<const void*, uint32_t> pointerMap;
			std::vector<std::pair<Game::XAsset, uint32_t>> aliasList;

			Game::RawFile branding;

			size_t assetDepth;
		};

		ZoneBuilder();
		~ZoneBuilder();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		bool unitTest() override;
#endif

		static bool IsEnabled();

		static std::string TraceZone;
		static std::vector<std::pair<Game::XAssetType, std::string>> TraceAssets;

		static void BeginAssetTrace(const std::string& zone);
		static std::vector<std::pair<Game::XAssetType, std::string>> EndAssetTrace();

		static Game::XAssetHeader GetEmptyAssetIfCommon(Game::XAssetType type, const std::string& name, Zone* builder);
		static void RefreshExporterWorkDirectory();

		static iw4of::api* GetExporter();

	private:
		static int StoreTexture(Game::GfxImageLoadDef **loadDef, Game::GfxImage *image);
		static void ReleaseTexture(Game::XAssetHeader header);

		static std::string FindMaterialByTechnique(const std::string& name);
		static void ReallocateLoadedSounds(void*& data, void* a2);

		static BOOL APIENTRY EntryPoint(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/);
		static void HandleError(Game::errorParm_t code, const char* fmt, ...);
		static void SoftErrorAssetOverflow();

		static void AssumeMainThreadRole();
		static void ResetThreadRole();

		static bool IsThreadMainThreadHook();
		static Game::Sys_File Sys_CreateFile_Stub(const char* dir, const char* filename);

		static iw4of::params_t GetExporterAPIParams();

		static void Com_Quitf_t();

		static void CommandThreadCallback();

		static bool MainThreadInterrupted;
		static DWORD InterruptingThreadId;

		static volatile bool CommandThreadTerminate;
		static std::thread CommandThread;
		static iw4of::api ExporterAPI;
		static std::string DumpingZone;
	};
}
