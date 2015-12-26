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

		static void On(Game::XAssetType type, Callback callback);
		static void Restrict(RestrictCallback callback);

	private:
		static bool BypassState;

		static Game::XAssetHeader FindAsset(Game::XAssetType type, const char* filename);
		static bool IsAssetEligible(Game::XAssetType type, Game::XAssetHeader* asset);
		static void FindAssetStub();
		static void AddAssetStub();

		static std::map<Game::XAssetType, Callback> TypeCallbacks;
		static std::vector<RestrictCallback> RestrictCallbacks;
	};
}
