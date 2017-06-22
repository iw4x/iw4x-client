#pragma once

namespace Assets
{
	class IXModel : public Components::AssetHandler::IAsset
	{
	public:
		virtual Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_XMODEL; };

		virtual void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		virtual void mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		virtual void load(Game::XAssetHeader* header, std::string name, Components::ZoneBuilder::Zone* builder) override;

	private:
		void loadXModelSurfs(Game::XModelSurfs* asset, Utils::Stream::Reader* reader);
		void loadXSurface(Game::XSurface* surf, Utils::Stream::Reader* reader);
		void loadXSurfaceCollisionTree(Game::XSurfaceCollisionTree* entry, Utils::Stream::Reader* reader);
	};
}
