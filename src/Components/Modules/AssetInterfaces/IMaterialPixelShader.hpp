#pragma once

namespace Assets
{
	class IMaterialPixelShader : public Components::AssetHandler::IAsset
	{
	public:
		virtual Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_PIXELSHADER; };

		virtual void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
        virtual void load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder) override;

        void loadNative(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder);
        void loadBinary(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder);
	};
}
