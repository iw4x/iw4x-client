namespace Assets
{
	class IXAnimParts : public Components::AssetHandler::IAsset
	{
	public:
		virtual Game::XAssetType GetType() override { return Game::XAssetType::ASSET_TYPE_XANIM; };

		virtual void Mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		virtual void Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
	};
}
