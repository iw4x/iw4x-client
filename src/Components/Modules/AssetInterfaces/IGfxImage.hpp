namespace Assets
{
	class IGfxImage : public Components::AssetHandler::IAsset
	{
		virtual Game::XAssetType GetType() override { return Game::XAssetType::ASSET_TYPE_IMAGE; };

		virtual void Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;

	private:
		void Save_GfxImageLoadDef(Components::ZoneBuilder::Zone* builder, const char* name);
	};
}