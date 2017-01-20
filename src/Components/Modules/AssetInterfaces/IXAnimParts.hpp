#pragma once

namespace Assets
{
	class IXAnimParts : public Components::AssetHandler::IAsset
	{
	public:
		virtual Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_XANIMPARTS; };

		virtual void mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		virtual void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		virtual void load(Game::XAssetHeader* header, std::string name, Components::ZoneBuilder::Zone* builder) override;

	private:
		void saveXAnimDeltaPart(Game::XAnimDeltaPart* delta, unsigned short framecount, Components::ZoneBuilder::Zone* builder);
	};
}
