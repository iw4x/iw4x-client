namespace Assets
{
	class IGameWorldSp : public Components::AssetHandler::IAsset
	{
	public:
		virtual Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_GAME_MAP_SP; };

		virtual void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		virtual void mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;

	private:
		void savepathnode_tree_info_t(Game::pathnode_tree_t* nodeTree, Game::pathnode_tree_t* destNodeTree, Components::ZoneBuilder::Zone* builder);
	};
}
