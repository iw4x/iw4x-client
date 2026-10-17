#pragma once

namespace Assets
{
  class Isnd_alias_list_t : public Components::AssetHandler::IAsset
  {
  public:
    Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_SOUND; }

    void load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder) override;
    void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
    void mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
    void dump(Game::XAssetHeader header) override;

    Isnd_alias_list_t();

    // Return true if name can be joined onto the dump directory without
    // leaving it.
    //
    static bool IsSafeDumpName(const char* name);

    // Return true if the alias list and every loaded sound it references have
    // safe names. See IsSafeDumpName() for details.
    //
    static bool IsSafeToDump(const Game::snd_alias_list_t* asset);
  };
}
