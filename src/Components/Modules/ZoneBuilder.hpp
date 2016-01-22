#define XFILE_MAGIC_UNSIGNED 0x3030317566665749
#define XFILE_VERSION 276

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

			void Build();

			Utils::Stream* GetBuffer();

			bool HasPointer(const void* pointer);
			void StorePointer(const void* pointer);

			template<typename T>
			T* GetPointer(const T* pointer) { return reinterpret_cast<T*>(SafeGetPointer(pointer)); }

			int FindAsset(Game::XAssetType type, const char* name);
			Game::XAsset* GetAsset(int index);
			uint32_t GetAssetOffset(int index);
			Game::XAssetHeader RequireAsset(Game::XAssetType type, const char* name);
			bool LoadAsset(Game::XAssetType type, std::string name);

			int AddScriptString(unsigned short gameIndex);
			int AddScriptString(std::string str);
			int FindScriptString(std::string str);

			void MapScriptString(unsigned short* gameIndex);

			void RenameAsset(Game::XAssetType type, std::string asset, std::string newName);
			std::string GetAssetName(Game::XAssetType type, std::string asset);

		private:
			void LoadFastFiles();

			bool LoadAssets();
			bool LoadAsset(std::string type, std::string name);

			void SaveData();
			void WriteZone();

			void AddBranding();

			uint32_t SafeGetPointer(const void* pointer);

			int IndexStart;
			Utils::Stream Buffer;

			std::string ZoneName;
			Utils::CSV DataMap;

			std::vector<Game::XAsset> LoadedAssets;
			std::vector<std::string> ScriptStrings;
			std::map<unsigned short, unsigned int> ScriptStringMap;
			std::map<std::string, std::string> RenameMap[Game::XAssetType::ASSET_TYPE_COUNT];
			std::map<const void*, uint32_t> PointerMap;

			Game::RawFile branding;
		};

		ZoneBuilder();
		const char* GetName() { return "ZoneBuilder"; };

		static bool IsEnabled();
	};
}
