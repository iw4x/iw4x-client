#pragma once

namespace Assets
{
	class IXModelSurfs : public Components::AssetHandler::IAsset
	{
	public:
		Game::XAssetType getType() override { return Game::ASSET_TYPE_XMODEL_SURFS; }

		void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;

	private:
		void saveXSurface(Game::XSurface* surf, Game::XSurface* destSurf, Components::ZoneBuilder::Zone* builder);
		void saveXSurfaceCollisionTree(Game::XSurfaceCollisionTree* entry, Components::ZoneBuilder::Zone* builder);
	};
}
