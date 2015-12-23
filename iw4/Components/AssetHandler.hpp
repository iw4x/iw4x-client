namespace Components
{
	class AssetHandler : public Component
	{
	public:
		typedef Game::XAssetHeader(*Callback)(Game::XAssetType, const char*);

		AssetHandler();
		~AssetHandler();
		const char* GetName() { return "AssetHandler"; };

		static void On(Game::XAssetType type, Callback callback);

	private:
		static bool BypassState;

		static Game::XAssetHeader FindAsset(Game::XAssetType type, const char* filename);
		static void FindAssetStub();

		static std::map<Game::XAssetType, Callback> TypeCallbacks;
	};
}
