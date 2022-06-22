#pragma once

namespace Assets
{
	class IXModel : public Components::AssetHandler::IAsset
	{
	public:
		Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_XMODEL; }

		void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		void mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		void load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder) override;

	private:
		std::map<void*, void*> triIndicies;
		void loadXModelSurfs(Game::XModelSurfs* asset, Utils::Stream::Reader* reader, Components::ZoneBuilder::Zone* builder);
		void loadXSurface(Game::XSurface* surf, Utils::Stream::Reader* reader, Components::ZoneBuilder::Zone* builder);
		void loadXSurfaceCollisionTree(Game::XSurfaceCollisionTree* entry, Utils::Stream::Reader* reader);
	};
}
