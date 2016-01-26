namespace Assets
{
	class ILocalizedEntry : public Components::AssetHandler::IAsset
	{
	public:
		virtual Game::XAssetType GetType() override { return Game::XAssetType::ASSET_TYPE_LOCALIZE; };

		virtual void Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
	};
}
