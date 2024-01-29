#pragma once

namespace Assets
{
	class IXModel : public Components::AssetHandler::IAsset
	{
	public:
		Game::XAssetType getType() override { return Game::ASSET_TYPE_XMODEL; }

		void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		void mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		void load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder) override;

		static void ConvertPlayerModelFromSingleplayerToMultiplayer(Game::XModel* model,  Utils::Memory::Allocator& allocator);

	private:
		static uint8_t GetIndexOfBone(const Game::XModel* model, std::string name);
		static uint8_t GetParentIndexOfBone(const Game::XModel* model, uint8_t index);
		static void SetParentIndexOfBone(Game::XModel* model, uint8_t boneIndex, uint8_t parentIndex);
		static std::string GetParentOfBone(Game::XModel* model, uint8_t index);
		static uint8_t GetHighestAffectingBoneIndex(const Game::XModelLodInfo* lod);
		static void RebuildPartBits(Game::XModel* model);
		static uint8_t InsertBone(Game::XModel* model, const std::string& boneName, const std::string& parentName,  Utils::Memory::Allocator& allocator);
		static void TransferWeights(Game::XModel* model, const uint8_t origin, const uint8_t destination);

		static void SetBoneTrans(Game::XModel* model, uint8_t boneIndex, bool baseMat, float x, float y, float z);
		static void SetBoneQuaternion(Game::XModel* model, uint8_t boneIndex, bool baseMat, float x, float y, float z, float w);
	};
}
