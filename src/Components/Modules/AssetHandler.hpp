#pragma once

extern "C"
{
  void AssetHandler_OverrideAssetStub();

  bool AssetHandler_ShouldOverrideAsset(Game::XAssetType type);
}

namespace Components
{
  class AssetHandler : public Component
  {
  public:
    typedef Game::XAssetHeader(Callback)(Game::XAssetType type, const std::string& name);
    typedef void(RestrictCallback)(Game::XAssetType type, Game::XAssetHeader asset, const std::string& name, bool* restrict);

    AssetHandler();
    ~AssetHandler();

    static void OnFind(Game::XAssetType type, Utils::Slot<Callback> callback);
    static std::function<void()> OnLoad(Utils::Slot<RestrictCallback> callback);

    static Game::XAssetHeader FindOriginalAsset(Game::XAssetType type, const char* filename);

    static void ResetBypassState();

  private:
    friend bool ::AssetHandler_ShouldOverrideAsset(Game::XAssetType type);

    static thread_local int BypassState;

    static std::map<Game::XAssetType, Utils::Slot<Callback>> TypeCallbacks;
    static Utils::Signal<RestrictCallback> RestrictSignal;

    static std::mutex EmptyAssetsMutex;
    static std::vector<std::pair<Game::XAssetType, std::string>> EmptyAssets;

    static Game::XAssetHeader(*DB_FindXAssetHeader_Original)(Game::XAssetType type, const char* name);
    static Game::XAssetEntry*(*DB_AddXAsset_Original)(Game::XAssetType type, Game::XAssetHeader* header);

    static Game::XAssetHeader DB_FindXAssetHeader_Hk(Game::XAssetType type, const char* name);
    static Game::XAssetEntry* DB_AddXAsset_Hk(Game::XAssetType type, Game::XAssetHeader* header);
    static Game::XAssetEntry* DB_CreateDefaultEntry_Hk(Game::XAssetType type, const char* name);

    static Game::XAssetHeader FindAsset(Game::XAssetType type, const char* filename);
    static bool IsAssetEligible(Game::XAssetType type, Game::XAssetHeader* asset);
    static bool ShouldOverrideAsset(Game::XAssetType type);

    static void StoreEmptyAsset(Game::XAssetType type, const char* name);

    static void ModifyAsset(Game::XAssetType type, Game::XAssetHeader asset, const std::string& name);

    static int HasThreadBypass();
    static void SetBypassState(bool value);

    static void ReallocateEntryPool();
  };
}
