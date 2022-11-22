#pragma once

namespace Assets
{
	class IStructuredDataDefSet : public Components::AssetHandler::IAsset
	{
	public:
		Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_STRUCTURED_DATA_DEF; }

		void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;

		static void saveStructuredDataEnumArray(Game::StructuredDataEnum* enums, int numEnums, Components::ZoneBuilder::Zone* builder);
		static void saveStructuredDataStructArray(Game::StructuredDataStruct* structs, int numStructs, Components::ZoneBuilder::Zone* builder);
	};
}
