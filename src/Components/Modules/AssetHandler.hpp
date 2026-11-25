#pragma once

extern "C"
{
  void AssetHandler_OverrideAssetStub();

  bool AssetHandler_ShouldOverrideAsset(Game::XAssetType type, const Game::XAssetEntry* entry);
}

namespace Components
{
  class AssetHandler : public Component
  {
  public:
    typedef Game::XAssetHeader(Callback)(Game::XAssetType type, const std::string& name);
    typedef void(RestrictCallback)(Game::XAssetType type, Game::XAssetHeader asset, const std::string& name, bool* restrict);
    typedef void(AddCallback)(Game::XAssetType type, Game::XAssetHeader loaded, Game::XAssetHeader added);

    AssetHandler();
    ~AssetHandler();

    static void OnFind(Game::XAssetType type, Utils::Slot<Callback> callback);
    static std::function<void()> OnLoad(Utils::Slot<RestrictCallback> callback);

    // Call back once DB_AddXAsset() added an asset, with the header the zone
    // loaded and the header that the database now has. The two differ since
    // the database copies the asset into its pool, and from then on IW4
    // refers to the copy. Runs on the thread that loads the zone.
    //
    static void OnAdd(Utils::Slot<AddCallback> callback);

    static Game::XAssetHeader FindOriginalAsset(Game::XAssetType type, const char* filename);

    static void ResetBypassState();

  private:
    friend bool ::AssetHandler_ShouldOverrideAsset(Game::XAssetType type, const Game::XAssetEntry* entry);

    static thread_local int BypassState;

    static std::map<Game::XAssetType, Utils::Slot<Callback>> TypeCallbacks;
    static Utils::Signal<RestrictCallback> RestrictSignal;
    static Utils::Signal<AddCallback> AddSignal;

    static std::mutex EmptyAssetsMutex;
    static std::vector<std::pair<Game::XAssetType, std::string>> EmptyAssets;

    static Game::XAssetHeader(*DB_FindXAssetHeader_Original)(Game::XAssetType type, const char* name);
    static Game::XAssetEntry*(*DB_AddXAsset_Original)(Game::XAssetType type, Game::XAssetHeader* header);

    static Game::XAssetHeader DB_FindXAssetHeader_Hk(Game::XAssetType type, const char* name);
    static Game::XAssetEntry* DB_AddXAsset_Hk(Game::XAssetType type, Game::XAssetHeader* header);
    static Game::XAssetEntry* DB_CreateDefaultEntry_Hk(Game::XAssetType type, const char* name);

    static Game::XAssetHeader FindAsset(Game::XAssetType type, const char* filename);
    static bool IsAssetEligible(Game::XAssetType type, Game::XAssetHeader* asset);
    static bool ShouldOverrideAsset(Game::XAssetType type, const Game::XAssetEntry* entry);

    static void StoreEmptyAsset(Game::XAssetType type, const char* name);

    static void ModifyAsset(Game::XAssetType type, Game::XAssetHeader asset, const std::string& name);

    static int HasThreadBypass();
    static void SetBypassState(bool value);

    static void ReallocateEntryPool();
  };
}
