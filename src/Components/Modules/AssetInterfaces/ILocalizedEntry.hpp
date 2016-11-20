namespace Assets
{
	class ILocalizedEntry : public Components::AssetHandler::IAsset
	{
	public:
		virtual Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_LOCALIZE; };

		virtual void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
	};
}
