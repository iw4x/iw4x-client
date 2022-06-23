#pragma once

namespace Assets
{
	class ILocalizeEntry : public Components::AssetHandler::IAsset
	{
	public:
		Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_LOCALIZE_ENTRY; };

		void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
	};
}
