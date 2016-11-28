namespace Assets
{
	class IFxEffectDef : public Components::AssetHandler::IAsset
	{
	public:
		virtual Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_FX; };

		virtual void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		virtual void mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;

	private:
		void markFxElemVisuals(Game::FxElemVisuals* visuals, char elemType, Components::ZoneBuilder::Zone* builder);
		void saveFxElemVisuals(Game::FxElemVisuals* visuals, Game::FxElemVisuals* destVisuals, char elemType, Components::ZoneBuilder::Zone* builder);
	};
}
