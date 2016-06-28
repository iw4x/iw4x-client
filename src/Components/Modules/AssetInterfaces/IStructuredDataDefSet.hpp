namespace Assets
{
	class IStructuredDataDefSet : public Components::AssetHandler::IAsset
	{
	public:
		virtual Game::XAssetType GetType() override { return Game::XAssetType::ASSET_TYPE_STRUCTUREDDATADEF; };

		virtual void Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;

		void Save_StructuredDataEnumArray(Game::StructuredDataEnum* enums, int numEnums, Components::ZoneBuilder::Zone* builder);
		void Save_StructuredDataStructArray(Game::StructuredDataStruct* structs, int numStructs, Components::ZoneBuilder::Zone* builder);
	};
}
