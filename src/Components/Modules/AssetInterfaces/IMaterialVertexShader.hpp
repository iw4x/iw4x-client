#pragma once

namespace Assets
{
	class IMaterialVertexShader : public Components::AssetHandler::IAsset
	{
	public:
		Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_VERTEXSHADER; }

		void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		void load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder) override;

		void loadNative(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder);
		void loadBinary(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder);
	};
}
