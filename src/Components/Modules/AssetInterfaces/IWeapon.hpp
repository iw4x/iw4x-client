#pragma once

namespace Assets
{
	class IWeapon : public Components::AssetHandler::IAsset
	{
	public:
		Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_WEAPON; }

		void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		void mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		void load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder) override;

	private:
		void writeWeaponDef(Game::WeaponDef* def, Components::ZoneBuilder::Zone* builder, Utils::Stream* buffer);
	};
}
