#pragma once

namespace Assets
{
	class IFxWorld : public Components::AssetHandler::IAsset
	{
	public:
		Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_FXWORLD; };

		void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		void mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		void load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder) override;
		void loadFromDisk(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder);
		void generate(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder);
	};
}
