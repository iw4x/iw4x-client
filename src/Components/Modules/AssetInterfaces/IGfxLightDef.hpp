#pragma once

namespace Assets
{
	class IGfxLightDef : public Components::AssetHandler::IAsset
	{
	public:
		Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_LIGHT_DEF; }

		void mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		void load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder) override;
	};
}
