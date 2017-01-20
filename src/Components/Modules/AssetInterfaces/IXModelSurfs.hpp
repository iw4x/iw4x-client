#pragma once

namespace Assets
{
	class IXModelSurfs : public Components::AssetHandler::IAsset
	{
	public:
		virtual Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_XMODELSURFS; };

		virtual void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;

	private:
		void saveXSurface(Game::XSurface* surf, Game::XSurface* destSurf, Components::ZoneBuilder::Zone* builder);
		void saveXSurfaceCollisionTree(Game::XSurfaceCollisionTree* entry, Components::ZoneBuilder::Zone* builder);
	};
}
