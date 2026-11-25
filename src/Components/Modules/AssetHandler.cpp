#include "AssetHandler.hpp"
#include "FastFiles.hpp"
#include "Weapon.hpp"

namespace Components
{
  namespace
  {
    constexpr uintptr_t ImageBase = 0x140000000;

    // An instruction that reaches the game's address target.
    //
    struct Reference
    {
      uintptr_t place;
      size_t length;
      uintptr_t target;
    };
  }

  thread_local int AssetHandler::BypassState = 0;
  std::map<Game::XAssetType, Utils::Slot<AssetHandler::Callback>> AssetHandler::TypeCallbacks;
  Utils::Signal<AssetHandler::RestrictCallback> AssetHandler::RestrictSignal;
  Utils::Signal<AssetHandler::AddCallback> AssetHandler::AddSignal;

  std::mutex AssetHandler::EmptyAssetsMutex;
  std::vector<std::pair<Game::XAssetType, std::string>> AssetHandler::EmptyAssets;

  Game::XAssetHeader(*AssetHandler::DB_FindXAssetHeader_Original)(Game::XAssetType type, const char* name);
  Game::XAssetEntry*(*AssetHandler::DB_AddXAsset_Original)(Game::XAssetType type, Game::XAssetHeader* header);

  Game::XAssetHeader AssetHandler::FindAsset(Game::XAssetType type, const char* filename)
  {
    Game::XAssetHeader header = { nullptr };

    if (filename)
    {
      // Allow call DB_FindXAssetHeader within the hook
      AssetHandler::SetBypassState(true);

      if (AssetHandler::TypeCallbacks.contains(type))
      {
        header = AssetHandler::TypeCallbacks[type](type, filename);
      }

      // Disallow calling DB_FindXAssetHeader ;)
      AssetHandler::SetBypassState(false);
    }

    return header;
  }

  int AssetHandler::HasThreadBypass()
  {
    return AssetHandler::BypassState > 0;
  }

  void AssetHandler::SetBypassState(bool value)
  {
    AssetHandler::BypassState += (value ? 1 : -1);
    if (AssetHandler::BypassState < 0)
    {
      AssetHandler::BypassState = 0;
      Logger::Error(Game::ERR_FATAL, "Bypass state is below 0!");
    }
  }

  void AssetHandler::ResetBypassState()
  {
    if (AssetHandler::HasThreadBypass())
    {
      // Maybe just decrement it?
      AssetHandler::BypassState = 0;
    }
  }

  void AssetHandler::ModifyAsset(Game::XAssetType type, Game::XAssetHeader asset, const std::string& name)
  {
    // Note that on x86 we also fix up the sort keys of a few materials, the
    // turret of vehicles, and the sort keys of the world, going by the
    // version of the IW4x zone they came in. On x64 we have no Zones
    // component, so there is no version to go by and we leave them be. If it
    // turns out some of these are needed after all, then ZoneConvert is
    // probably where they belong.
    //
    if (*Game::com_developer && (*Game::com_developer)->current.enabled)
    {
      if (type == Game::XAssetType::ASSET_TYPE_IMAGE && name[0] != ',')
      {
        AssertOffset(Game::GfxImage, category, 0xA);

        const Game::GfxImage* image = asset.image;
        const Game::ImageCategory cat = static_cast<Game::ImageCategory>(image->category);
        if (cat == Game::ImageCategory::IMG_CATEGORY_UNKNOWN)
        {
          Logger::Warning(Game::CON_CHANNEL_GFX, "Image {} has wrong category IMG_CATEGORY_UNKNOWN, this is an IMPORTANT ISSUE that should be fixed!\n", name);
        }
      }
    }

    if (type == Game::ASSET_TYPE_MATERIAL && (name == "wc/codo_ui_viewer_black_decal3" || name == "wc/codo_ui_viewer_black_decal2" || name == "wc/hint_arrows01" || name == "wc/hint_arrows02"))
    {
      AssertOffset(Game::MaterialInfo, sortKey, 9);

      asset.material->info.sortKey = 0xE;
    }
  }

  bool AssetHandler::IsAssetEligible(Game::XAssetType type, Game::XAssetHeader* asset)
  {
    const char* name = Game::DB_GetXAssetNameHandlers[type](asset);

    if (!name) return false;

    {
      // If a zone brings an asset we created an empty one for earlier, then it
      // wasn't missing after all, so forget about it.
      //
      std::scoped_lock lock(AssetHandler::EmptyAssetsMutex);

      std::erase_if(AssetHandler::EmptyAssets, [type, name] (const std::pair<Game::XAssetType, std::string>& e)
      {
        return e.first == type && e.second == name;
      });
    }

    if (Flags::HasFlag("entries"))
    {
      OutputDebugStringA(Utils::String::VA("%s: %d: %s\n", FastFiles::Current().data(), type, name));
    }

    bool restrict = false;
    AssetHandler::RestrictSignal(type, *asset, name, &restrict);

    if (!restrict)
    {
      AssetHandler::ModifyAsset(type, *asset, name);
    }

    // If no slot restricts the loading, we can load the asset
    return (!restrict);
  }

  // Return true if the zone we are loading should replace the asset that
  // entry refers to, the one already in the database.
  //
  bool AssetHandler::ShouldOverrideAsset(Game::XAssetType type, const Game::XAssetEntry* entry)
  {
    const auto zone (FastFiles::Current());

    // Never let our zones replace what mod.ff brings. With an override the
    // zone that loads last wins, and our UI zones load again whenever the UI
    // restarts (after a mod change, say), possibly after mod.ff. So without
    // this they would take the mod's menus back.
    //
    if (zone.starts_with("iw4x_") && Game::DB_GetZoneName(*entry) == "mod")
      return false;

    switch (type)
    {
    // The game itself always overrides these on async loads (see the stub at
    // 0x14012E97E), so we keep doing that.
    //
    case Game::ASSET_TYPE_LIGHT_DEF:
    case Game::ASSET_TYPE_IMPACT_FX:
      return true;

    // Only let mod.ff replace these.
    //
    case Game::ASSET_TYPE_STRINGTABLE:
      return zone == "mod";

    // Let mod.ff and our zones replace the stock menus, along with the menu
    // lists that load them.
    //
    // Without an override DB_AddXAsset() keeps the menu that came first. If
    // the new menu's zone has the same flags, it drops the new one, and
    // otherwise it chains the new one behind, where nothing looks for it.
    // In both cases the stock menu stays, since the stock UI zones load before
    // ours (and have a higher priority than mod.ff to begin with).
    //
    // With an override our menu goes into the stock menu's slot, so whatever
    // already points to the stock menu (the UI, the stock menu lists) gets
    // ours. Which also means our zones only need to ship the menus they
    // change.
    //
    case Game::ASSET_TYPE_MENU:
    case Game::ASSET_TYPE_MENULIST:
      return zone == "mod" || zone.starts_with("iw4x_");

    // Let iw4x_code_post_gfx_mp replace IW4's mp/playerdata.def with ours.
    //
    // We use 15 custom classes (see StructuredData), but IW4's def only has
    // room for 10, so writing to any of the other five fails with "Error
    // setting persistent data". Our def adds the IW4x versions that have the
    // extra classes. On x86 ZoneBuilder patches them into IW4's def when it
    // builds the zones, but x64 has no ZoneBuilder, which is why our zone
    // ships the whole def instead. And since DB_AddXAsset() keeps the first
    // def it gets, which is IW4's, ours only takes effect if we let it
    // override.
    //
    // Note that mod.ff seems to load after our zone (see
    // R_LoadGraphicsAssets()), so a mod can still replace the def with its
    // own.
    //
    case Game::ASSET_TYPE_STRUCTURED_DATA_DEF:
      return zone == "mod" || zone == "iw4x_code_post_gfx_mp";

    // Our localized zone loads after the stock ones and is there to replace
    // some of their strings, not only to add new ones. Note that some of the
    // stock strings are actually broken: the Microsoft Store French, Spanish,
    // and Russian zones define NULL_EMPTY, which menus use for "no text", as
    // an English map pack notice. Without this the first definition wins and
    // every such replacement is silently ignored.
    //
    case Game::ASSET_TYPE_LOCALIZE_ENTRY:
      return zone.starts_with("iw4x_localized_");

    default:
      return false;
    }
  }

  void AssetHandler::OnFind(Game::XAssetType type, Utils::Slot<AssetHandler::Callback> callback)
  {
    AssetHandler::TypeCallbacks[type] = callback;
  }

  void AssetHandler::OnAdd(Utils::Slot<AssetHandler::AddCallback> callback)
  {
    AssetHandler::AddSignal.connect(callback);
  }

  std::function<void()> AssetHandler::OnLoad(Utils::Slot<AssetHandler::RestrictCallback> callback)
  {
    AssetHandler::RestrictSignal.connect(callback);

    return [callback]() {
      AssetHandler::RestrictSignal.disconnect(callback);
      };
  }

  Game::XAssetHeader AssetHandler::FindOriginalAsset(Game::XAssetType type, const char* filename)
  {
    AssetHandler::SetBypassState(true);
    Game::XAssetHeader header = Game::DB_FindXAssetHeader(type, filename);
    AssetHandler::SetBypassState(false);

    return header;
  }

  void AssetHandler::StoreEmptyAsset(Game::XAssetType type, const char* name)
  {
    std::scoped_lock lock(AssetHandler::EmptyAssetsMutex);
    AssetHandler::EmptyAssets.emplace_back(type, name);
  }

  Game::XAssetHeader AssetHandler::DB_FindXAssetHeader_Hk(Game::XAssetType type, const char* name)
  {
    if (!AssetHandler::HasThreadBypass())
    {
      Game::XAssetHeader header = AssetHandler::FindAsset(type, name);

      if (header.data)
      {
        return header;
      }
    }

    return DB_FindXAssetHeader_Original(type, name);
  }

  Game::XAssetEntry* AssetHandler::DB_AddXAsset_Hk(Game::XAssetType type, Game::XAssetHeader* header)
  {
    if (AssetHandler::IsAssetEligible(type, header))
    {
      const Game::XAssetHeader loaded = *header;
      Game::XAssetEntry* entry = DB_AddXAsset_Original(type, header);

      if (entry)
      {
        AssetHandler::AddSignal(type, loaded, entry->asset.header);
      }

      return entry;
    }

    // We've decided to keep this asset out of the database, but the callers
    // will copy the header of whatever entry we return back into the asset
    // they just loaded. So hand them a scratch entry with the very header
    // they gave us, which leaves the asset untouched. Slightly sneaky, but it
    // saves us hooking every caller.
    //
    thread_local Game::XAssetEntry entry;

    entry = {};
    entry.asset.type = type;
    entry.asset.header = *header;

    return &entry;
  }

  Game::XAssetEntry* AssetHandler::DB_CreateDefaultEntry_Hk(Game::XAssetType type, const char* name)
  {
    AssetHandler::StoreEmptyAsset(type, name);
    return Utils::Hook::Call<Game::XAssetEntry*(Game::XAssetType, const char*)>(0x14012CFE0)(type, name);
  }

  void AssetHandler::ReallocateEntryPool()
  {
    AssertSize(Game::XAssetEntry, 0x18);
    AssertOffset(Game::XAssetEntry, nextHash, 0x12);
    AssertOffset(Game::XAssetEntry, nextOverride, 0x14);

    // Our zones bring a lot more assets than the stock 37000 entries leave
    // room for, so move the asset entry pool to one with as many entries as
    // x86 has.
    //
    // Note that the entries link to each other by 16-bit indices, which caps
    // the pool at 65535 entries (thus the static_assert below). The x86 count
    // is comfortably below that, so we are fine for now.
    //
    constexpr uintptr_t pool = 0x14151F4E0;
    constexpr size_t gameCount = 37000;
    constexpr size_t count = 49332;

    static_assert(count <= std::numeric_limits<uint16_t>::max());

    Game::XAssetEntry* entries = static_cast<Game::XAssetEntry*>(Utils::Hook::AllocateNear(ImageBase, count * sizeof(Game::XAssetEntry)));
    const uintptr_t to = reinterpret_cast<uintptr_t>(entries);

    auto moved = [to] (uintptr_t a)
    {
      assert(a >= pool && a < pool + gameCount * sizeof(Game::XAssetEntry));
      return to + (a - pool);
    };

    // Point the game at the new pool wherever it reaches the old one relative
    // to the next instruction.
    //
    // And wherever it reaches it relative to the image base, some of these at
    // a field of the first entry rather than the entry itself.
    //
    for (const Reference& r: std::initializer_list<Reference>
    {
      {0x14012D103, 7, 0x14151F4E0}, {0x14012D2EE, 7, 0x14151F4E0}, {0x14012D334, 7, 0x14151F4E0},
      {0x14012D365, 7, 0x14151F4E0}, {0x14012DC14, 7, 0x14151F4E0}, {0x14012E034, 7, 0x14151F4F8},
      {0x14012E047, 7, 0x14151F510}, {0x14012E2E9, 7, 0x14151F4E0}, {0x14012E45A, 7, 0x14151F4E0},
      {0x14012E4FC, 7, 0x14151F4E0}, {0x14012E76B, 7, 0x14151F4E0}, {0x14012E7B5, 7, 0x14151F4E0},
      {0x14012E8F4, 7, 0x14151F4E0}, {0x14012EA05, 7, 0x14151F4E0}, {0x14012F600, 7, 0x14151F4E0}
    })
    {
      Utils::Hook::Displace(r.place, r.length, r.target, moved(r.target));
    }

    for (const Reference& r: std::initializer_list<Reference>
    {
      {0x14012D554, 7, 0x14151F4E0}, {0x14012D5B3, 7, 0x14151F4E0}, {0x14012D656, 7, 0x14151F4E0},
      {0x14012DA54, 9, 0x14151F4F1}, {0x14012DA5F, 8, 0x14151F4F1}, {0x14012DA67, 9, 0x14151F4F2},
      {0x14012DA94, 7, 0x14151F4E0}, {0x14012DAF3, 7, 0x14151F4E0}, {0x14012DF64, 7, 0x14151F4E0},
      {0x14012E204, 7, 0x14151F4E0}, {0x14012EE74, 8, 0x14151F4E0}, {0x14012EF12, 8, 0x14151F4E8},
      {0x14012EF7D, 9, 0x14151F4F4}, {0x14012F84A, 7, 0x14151F4E0}, {0x14012FDD8, 8, 0x14151F4E0},
      {0x14012FF1C, 8, 0x14151F4E0}, {0x140130017, 8, 0x14151F4E0}
    })
    {
      Utils::Hook::Rebase(r.place, r.length, ImageBase, r.target, moved(r.target));
    }

    // At startup the game links the free entries past the first into a list
    // that ends at the last one. So both the count and the end of that list
    // have to follow the new pool, or the extra entries would never be
    // handed out.
    //
    Utils::Hook::Replace32(0x14012E03B, 5, gameCount - 2, count - 2);
    Utils::Hook::Displace(0x14012E068, 7,
                          pool + (gameCount - 1) * sizeof(Game::XAssetEntry),
                          to + (count - 1) * sizeof(Game::XAssetEntry));

    Game::g_assetEntryPool = entries;
  }

  AssetHandler::AssetHandler()
  {
    AssetHandler::ReallocateEntryPool();

    Dvar::Register<bool>("r_noVoid", false, Game::DVAR_ARCHIVE, "Disable void model (red fx)");

    // Note that DB_FindXAssetHeader() starts with a 5-byte store of rbx to its
    // home slot and DB_AddXAsset() with one of rdx, which is what the detours
    // move out of the way.
    //
    DB_FindXAssetHeader_Original = Utils::Hook::Detour(0x14012D6B0, 5, &AssetHandler::DB_FindXAssetHeader_Hk);
    DB_AddXAsset_Original = Utils::Hook::Detour(0x14012E6E0, 5, &AssetHandler::DB_AddXAsset_Hk);

    Utils::Hook(0x14012E97E, AssetHandler_OverrideAssetStub, HOOK_JUMP).install()->quick();

    // Keep track of the empty assets. DB_AddXAsset() creates a default entry
    // for an asset that a zone only references (its name starts with ',') and
    // that no zone has brought yet, which is what we want to hear about.
    //
    Utils::Hook(0x14012E7DA, AssetHandler::DB_CreateDefaultEntry_Hk, HOOK_CALL).install()->quick();

    // Note that on x86 we also hook the game's error about a missing asset to
    // keep it quiet for technique sets on a dedicated server. x64 doesn't seem
    // to have that error at all (its "Could not load" message is gone), so
    // there is nothing to hook.
    //

    // Log missing empty assets
    Scheduler::Loop([]
      {
        if (!FastFiles::Ready()) return;

        std::scoped_lock lock(AssetHandler::EmptyAssetsMutex);

        for (const std::pair<Game::XAssetType, std::string>& asset : AssetHandler::EmptyAssets)
        {
          Logger::Warning(Game::CON_CHANNEL_FILES, "Could not load {} \"{}\".\n", Game::DB_GetXAssetTypeName(asset.first), asset.second);
        }

        AssetHandler::EmptyAssets.clear();
      }, Scheduler::Pipeline::MAIN);

    AssetHandler::OnLoad([](Game::XAssetType type, Game::XAssetHeader asset, std::string name, bool*)
      {
        if (Dvar::Var("r_noVoid").get<bool>() && type == Game::ASSET_TYPE_XMODEL && name == "void")
        {
          AssertOffset(Game::XModel, numLods, 0x141);

          asset.model->numLods = 0;
        }
      });

    // Make room in the pools for what our zones bring on top of the stock
    // ones. We use the same sizes as x86, which seem to be plenty.
    //
    // Note that, unlike x86, we leave the material pool alone. The game
    // reaches it directly to turn a material into its index and back
    // (0x14012DD10 and 0x14012DD30) and presumably keeps such indices around
    // in arrays sized for the stock pool, so moving it would take a lot more
    // than this. So far the stock pool has been enough, so let's not go
    // there unless we have to.
    //
    Game::ReallocateAssetPool(Game::ASSET_TYPE_GAMEWORLD_SP, 1);
    Game::ReallocateAssetPool(Game::ASSET_TYPE_IMAGE, 7168);
    Game::ReallocateAssetPool(Game::ASSET_TYPE_LOADED_SOUND, 2700 * 2);
    Game::ReallocateAssetPool(Game::ASSET_TYPE_FX, 1200 * 2);
    Game::ReallocateAssetPool(Game::ASSET_TYPE_LOCALIZE_ENTRY, 14000);
    Game::ReallocateAssetPool(Game::ASSET_TYPE_XANIMPARTS, 8192 * 2);
    Game::ReallocateAssetPool(Game::ASSET_TYPE_XMODEL, 5125 * 2);
    Game::ReallocateAssetPool(Game::ASSET_TYPE_PHYSPRESET, 128);
    Game::ReallocateAssetPool(Game::ASSET_TYPE_PIXELSHADER, 10000);
    Game::ReallocateAssetPool(Game::ASSET_TYPE_VERTEXSHADER, 3072);
    Game::ReallocateAssetPool(Game::ASSET_TYPE_VERTEXDECL, 196);
    Game::ReallocateAssetPool(Game::ASSET_TYPE_WEAPON, Weapon::WEAPON_LIMIT);
    Game::ReallocateAssetPool(Game::ASSET_TYPE_STRINGTABLE, 800);
    Game::ReallocateAssetPool(Game::ASSET_TYPE_IMPACT_FX, 8);

    // Double the stock menu pools, which x86 leaves alone.
    //
    // While our zones load, DB_AddXAsset() takes a slot for every menu it
    // comes across and only then decides to override the stock one (see
    // ShouldOverrideAsset()). So for a while we need room for both copies,
    // and with as many stock menus as our zones replace, the stock pools
    // presumably wouldn't be enough.
    //
    Game::ReallocateAssetPool(Game::ASSET_TYPE_MENULIST, 128 * 2);
    Game::ReallocateAssetPool(Game::ASSET_TYPE_MENU, 740 * 2);
  }

  AssetHandler::~AssetHandler()
  {
    AssetHandler::RestrictSignal.clear();
    AssetHandler::AddSignal.clear();
    AssetHandler::TypeCallbacks.clear();
  }
}

bool AssetHandler_ShouldOverrideAsset(Game::XAssetType type, const Game::XAssetEntry* entry)
{
  return Components::AssetHandler::ShouldOverrideAsset(type, entry);
}
