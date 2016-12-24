namespace Assets
{
	class IGfxLightDef : public Components::AssetHandler::IAsset
	{
	public:
		virtual Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_LIGHTDEF; };

		virtual void mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		virtual void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
	};
}
