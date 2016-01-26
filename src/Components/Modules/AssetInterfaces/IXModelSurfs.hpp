namespace Assets
{
	class IXModelSurfs : public Components::AssetHandler::IAsset
	{
	public:
		virtual Game::XAssetType GetType() override { return Game::XAssetType::ASSET_TYPE_XMODELSURFS; };

		virtual void Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;

	private:
		void Save_XSurface(Game::XSurface* surf, Game::XSurface* destSurf, Components::ZoneBuilder::Zone* builder);
		void Save_XSurfaceCollisionTree(Game::XSurfaceCollisionTree* entry, Components::ZoneBuilder::Zone* builder);
	};
}
