#pragma once

namespace Components
{
	class AssetHandler : public Component
	{
	public:
		class IAsset
		{
		public:
			virtual ~IAsset() {}
			virtual Game::XAssetType getType() { return Game::XAssetType::ASSET_TYPE_INVALID; }
			virtual void mark(Game::XAssetHeader /*header*/, ZoneBuilder::Zone* /*builder*/) {}
			virtual void save(Game::XAssetHeader /*header*/, ZoneBuilder::Zone* /*builder*/) {}
			virtual void dump(Game::XAssetHeader /*header*/) {}
			virtual void load(Game::XAssetHeader* /*header*/, const std::string& /*name*/, ZoneBuilder::Zone* /*builder*/) {}
		};

		typedef Game::XAssetHeader(Callback)(Game::XAssetType type, const std::string& name);
		typedef void(RestrictCallback)(Game::XAssetType type, Game::XAssetHeader asset, const std::string& name, bool* restrict);

		AssetHandler();
		~AssetHandler();

		static void OnFind(Game::XAssetType type, Utils::Slot<Callback> callback);
		static void OnLoad(Utils::Slot<RestrictCallback> callback);

		static void ClearRelocations();
		static void Relocate(void* start, void* to, DWORD size = 4);

		static void ZoneSave(Game::XAsset asset, ZoneBuilder::Zone* builder);
		static void ZoneMark(Game::XAsset asset, ZoneBuilder::Zone* builder);

		static Game::XAssetHeader FindOriginalAsset(Game::XAssetType type, const char* filename);
		static Game::XAssetHeader FindAssetForZone(Game::XAssetType type, const std::string& filename, ZoneBuilder::Zone* builder, bool isSubAsset = true);

		static void ClearTemporaryAssets();
		static void StoreTemporaryAsset(Game::XAssetType type, Game::XAssetHeader asset);

		static void ResetBypassState();

		static void ExposeTemporaryAssets(bool expose);

		static void OffsetToAlias(Utils::Stream::Offset* offset);
		
	private:
		static thread_local int BypassState;
		static bool ShouldSearchTempAssets;

		static std::map<std::string, Game::XAssetHeader> TemporaryAssets[Game::XAssetType::ASSET_TYPE_COUNT];

		static std::map<Game::XAssetType, IAsset*> AssetInterfaces;
		static std::map<Game::XAssetType, Utils::Slot<Callback>> TypeCallbacks;
		static Utils::Signal<RestrictCallback> RestrictSignal;

		static std::map<void*, void*> Relocations;

		static std::vector<std::pair<Game::XAssetType, std::string>> EmptyAssets;

		static void RegisterInterface(IAsset* iAsset);

		static Game::XAssetHeader FindAsset(Game::XAssetType type, const char* filename);
		static Game::XAssetHeader FindTemporaryAsset(Game::XAssetType type, const char* filename);
		static bool IsAssetEligible(Game::XAssetType type, Game::XAssetHeader* asset);
		static void FindAssetStub();
		static void AddAssetStub();

		static void StoreEmptyAsset(Game::XAssetType type, const char* name);
		static void StoreEmptyAssetStub();

		static void ModifyAsset(Game::XAssetType type, Game::XAssetHeader asset, const std::string& name);

		static int HasThreadBypass();
		static void SetBypassState(bool value);

		static void MissingAssetError(int severity, const char* format, const char* type, const char* name);

		void reallocateEntryPool();
	};
}
