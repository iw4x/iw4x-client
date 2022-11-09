#pragma once

namespace Assets
{
	class IStringTable : public Components::AssetHandler::IAsset
	{
	public:
		Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_STRINGTABLE; }

		void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;

	private:
		static void saveStringTableCellArray(Components::ZoneBuilder::Zone* builder, Game::StringTableCell* values, int count);
	};
}
