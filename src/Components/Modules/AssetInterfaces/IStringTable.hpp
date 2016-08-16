namespace Assets
{
	class IStringTable : public Components::AssetHandler::IAsset
	{
	public:
		virtual Game::XAssetType GetType() override { return Game::XAssetType::ASSET_TYPE_STRINGTABLE; };

		virtual void Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;

	private:
		void Save_StringTableCellArray(Components::ZoneBuilder::Zone* builder, Game::StringTableCell* values, int count);
	};
}
