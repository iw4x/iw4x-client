namespace Assets
{
	class IPhysCollmap : public Components::AssetHandler::IAsset
	{
	public:
		virtual Game::XAssetType GetType() override { return Game::XAssetType::ASSET_TYPE_PHYS_COLLMAP; };

		virtual void Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;

	private:
		void Save_PhysGeomInfoArray(Components::ZoneBuilder::Zone* builder, Game::PhysGeomInfo* geoms, unsigned int count);
		void Save_BrushWrapper(Components::ZoneBuilder::Zone* builder, Game::BrushWrapper* brush);
	};
}
