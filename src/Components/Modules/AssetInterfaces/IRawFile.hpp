namespace Assets
{
	class IRawFile : public Components::AssetHandler::IAsset
	{
	public:
		virtual Game::XAssetType GetType() override { return Game::XAssetType::ASSET_TYPE_RAWFILE; };

		virtual void Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
	};
}