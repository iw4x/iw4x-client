#pragma once

namespace Assets
{
	class IPhysPreset : public Components::AssetHandler::IAsset
	{
	public:
		Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_PHYSPRESET; }

		void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
	};
}
