namespace Assets
{
	class IMapEnts : public Components::AssetHandler::IAsset
	{
	public:
		virtual Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_MAP_ENTS; };

		virtual void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		virtual void load(Game::XAssetHeader* header, std::string name, Components::ZoneBuilder::Zone* builder) override;
	};
}
