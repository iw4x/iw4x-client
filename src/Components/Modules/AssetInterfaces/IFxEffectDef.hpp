#pragma once

namespace Assets
{
	class IFxEffectDef : public Components::AssetHandler::IAsset
	{
	public:
		Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_FX; }

		void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		void mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		void load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder) override;

	private:
		void markFxElemVisuals(Game::FxElemVisuals* visuals, char elemType, Components::ZoneBuilder::Zone* builder);
		void saveFxElemVisuals(Game::FxElemVisuals* visuals, Game::FxElemVisuals* destVisuals, char elemType, Components::ZoneBuilder::Zone* builder);

		void loadEfx(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder);
		void loadNative(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder);
		void loadFromIW4OF(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder);
	};
}
