namespace Assets
{
	class IXModelSurfs : public Components::AssetHandler::IAsset
	{
		virtual Game::XAssetType GetType() override { return Game::XAssetType::ASSET_TYPE_XMODELSURFS; };

		virtual void Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
	};
}
