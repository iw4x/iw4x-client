namespace Assets
{
	class IXAnimParts : public Components::AssetHandler::IAsset
	{
	public:
		virtual Game::XAssetType GetType() override { return Game::XAssetType::ASSET_TYPE_XANIM; };

		virtual void Mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		virtual void Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		virtual void Load(Game::XAssetHeader* header, std::string name, Components::ZoneBuilder::Zone* builder) override;

	private:
		void Save_XAnimDeltaPart(Game::XAnimDeltaPart* delta, unsigned short framecount, Components::ZoneBuilder::Zone* builder);
	};
}
