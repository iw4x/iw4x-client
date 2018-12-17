#pragma once

namespace Assets
{
	class IFxWorld : public Components::AssetHandler::IAsset
	{
	public:
		virtual Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_FXWORLD; };

		virtual void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		virtual void mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		virtual void load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder) override;
	};
}
