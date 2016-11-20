namespace Assets
{
	class IStructuredDataDefSet : public Components::AssetHandler::IAsset
	{
	public:
		virtual Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_STRUCTUREDDATADEF; };

		virtual void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;

		void saveStructuredDataEnumArray(Game::StructuredDataEnum* enums, int numEnums, Components::ZoneBuilder::Zone* builder);
		void saveStructuredDataStructArray(Game::StructuredDataStruct* structs, int numStructs, Components::ZoneBuilder::Zone* builder);
	};
}
