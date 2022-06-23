#pragma once

namespace Assets
{
	class IMenuList : public Components::AssetHandler::IAsset
	{
	public:
		Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_MENULIST; }

		void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		void mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		void load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder) override;
	};
}
