#pragma once

namespace Assets
{
	class IRawFile : public Components::AssetHandler::IAsset
	{
	public:
		virtual Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_RAWFILE; };

		virtual void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		virtual void load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder) override;
	};
}
