namespace Assets
{
	class IGameWorldSp : public Components::AssetHandler::IAsset
	{
	public:
		virtual Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_GAMEWORLD_SP; };

		virtual void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		virtual void mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;

	private:
		void savepathnode_tree_info_t(Game::pathnode_tree_t* nodeTree, Game::pathnode_tree_t* destNodeTree, Components::ZoneBuilder::Zone* builder);
		void saveVehicleTrackSegment(Game::VehicleTrackSegment* trackSegment, Game::VehicleTrackSegment* destTrackSegment, Components::ZoneBuilder::Zone* builder);
		void saveVehicleTrackSegment_ptrArray(Game::VehicleTrackSegment** trackSegmentPtrs, int count, Components::ZoneBuilder::Zone* builder);
	};
}
