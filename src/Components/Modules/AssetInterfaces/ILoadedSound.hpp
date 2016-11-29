namespace Assets
{
	class ILoadedSound : public Components::AssetHandler::IAsset
	{
	public:
		virtual Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_LOADED_SOUND; };

		virtual void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
	};
}
