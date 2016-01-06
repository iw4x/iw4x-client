namespace Components
{
	class AssetHandler : public Component
	{
	public:
		typedef Game::XAssetHeader(*Callback)(Game::XAssetType, const char*);
		typedef bool(*RestrictCallback)(Game::XAssetType type, Game::XAssetHeader asset, const char* name);

		AssetHandler();
		~AssetHandler();
		const char* GetName() { return "AssetHandler"; };

		static void OnFind(Game::XAssetType type, Callback callback);
		static void OnLoad(RestrictCallback callback);

		static void Relocate(void* start, void* to, DWORD size = 4);

	private:
		static bool BypassState;

		static Game::XAssetHeader FindAsset(Game::XAssetType type, const char* filename);
		static bool IsAssetEligible(Game::XAssetType type, Game::XAssetHeader* asset);
		static void FindAssetStub();
		static void AddAssetStub();

		static void OffsetToAlias(Utils::Stream::Offset* offset);

		static std::map<Game::XAssetType, Callback> TypeCallbacks;
		static std::vector<RestrictCallback> RestrictCallbacks;

		static std::map<void*, void*> Relocations;
	};
}
