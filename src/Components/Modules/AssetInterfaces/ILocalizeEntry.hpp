#pragma once

namespace Assets
{
	class ILocalizeEntry : public Components::AssetHandler::IAsset
	{
	public:
		Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_LOCALIZE_ENTRY; }

		void load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder) override;
		void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;

		static void ParseLocalizedStringsFile(Components::ZoneBuilder::Zone* builder, const std::string& name, const std::string& filename);
		static void ParseLocalizedStringsJSON(Components::ZoneBuilder::Zone* builder, Components::FileSystem::File& file);
	};
}
