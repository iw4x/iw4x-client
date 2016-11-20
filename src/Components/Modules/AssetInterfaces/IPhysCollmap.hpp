namespace Assets
{
	class IPhysCollmap : public Components::AssetHandler::IAsset
	{
	public:
		virtual Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_PHYS_COLLMAP; };

		virtual void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;

	private:
		void savePhysGeomInfoArray(Components::ZoneBuilder::Zone* builder, Game::PhysGeomInfo* geoms, unsigned int count);
		void saveBrushWrapper(Components::ZoneBuilder::Zone* builder, Game::BrushWrapper* brush);
	};
}
