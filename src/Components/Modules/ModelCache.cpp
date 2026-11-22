#include "ModelCache.hpp"

namespace Components
{
  namespace
  {
    constexpr uintptr_t ImageBase = 0x140000000;

    // The game's model arrays: the server's G_ModelIndex() cache and the
    // cgame's cgs_t::gameModels (which cgsArray holds at 0x190).
    //
    constexpr uintptr_t CachedModels = 0x1419F0E40;
    constexpr uintptr_t GameModels = 0x140585520;
    constexpr uintptr_t ClientGameStatic = 0x140585390;

    constexpr uint32_t BaseModelsSize = ModelCache::BASE_GMODEL_COUNT * sizeof(Game::XModel*);

    // An instruction that reaches one of the game's addresses above.
    //
    struct Reference
    {
      uintptr_t place;
      size_t length;
      uintptr_t target;
    };
  }

  Game::XModel** ModelCache::cached_models_reallocated;
  Game::XModel** ModelCache::gameModels_reallocated; // Partt of cgs_t
  bool ModelCache::modelsHaveBeenReallocated;

  ModelCache::ModelCache()
  {
    if (Flags::HasFlag("steamdemo") || Flags::HasFlag("retaildemo") || Flags::HasFlag("iw4x_legacydemo"))
    {
      // don't patch the netfields because it breaks demo compatibility
      return;
    }

    // To push the model limit we need to update the network protocol because it uses custom integer size
    // (currently 9 bits per model, not enough)
    const auto oldBitLength = static_cast<int>(std::floor(std::log2(BASE_GMODEL_COUNT - 1)) + 1);
    const auto newBitLength = static_cast<int>(std::floor(std::log2(G_MODELINDEX_LIMIT - 1)) + 1);

    assert(oldBitLength == 9);

    if (oldBitLength != newBitLength)
    {
      static const std::unordered_set<std::string> fieldsToUpdate = {
        "modelindex",
        "attachModelIndex[0]",
        "attachModelIndex[1]",
        "attachModelIndex[2]",
        "attachModelIndex[3]",
        "attachModelIndex[4]",
        "attachModelIndex[5]",
      };

      for (int i = 0; i < Game::clientStateFieldsCount; i++)
      {
        auto field = & Game::clientStateFields[i];

        if (fieldsToUpdate.contains(field->name))
        {
          assert(field->bits == oldBitLength);

          Utils::Hook::Set(&field->bits, newBitLength);
        }
      }
    }

    cached_models_reallocated = static_cast<Game::XModel**>(Utils::Hook::AllocateNear(ImageBase, G_MODELINDEX_LIMIT * sizeof(Game::XModel*)));
    gameModels_reallocated = static_cast<Game::XModel**>(Utils::Hook::AllocateNear(ImageBase, G_MODELINDEX_LIMIT * sizeof(Game::XModel*)));

    const auto cachedModels (reinterpret_cast<uintptr_t>(cached_models_reallocated));
    const auto gameModels (reinterpret_cast<uintptr_t>(gameModels_reallocated));

    // Reallocate G_ModelIndex
    //
    // The game mostly reaches the cache relative to the next instruction,
    // which Displace() points at our copy. Two of its lookups add an offset
    // to the image base instead, so those we rebase. Why the compiler mixed
    // the two we don't know, but missing either kind would leave the game
    // reading the old array. Note that the loop that registers the models of
    // the configstrings keeps walking the game's count of them, the same as
    // on x86.
    //
    for (const Reference& r: std::initializer_list<Reference>
    {
      {0x1401AA2D2, 7, CachedModels}, {0x1401AB1D4, 7, CachedModels}, {0x1401ABC8D, 7, CachedModels},
      {0x1401ABCD3, 7, CachedModels}, {0x1401AC086, 7, CachedModels}, {0x1401AC9D3, 7, CachedModels},
      {0x1401AC174, 7, CachedModels + sizeof(Game::XModel*)}, {0x1401AC17B, 7, CachedModels + BaseModelsSize}
    })
    {
      Utils::Hook::Displace(r.place, r.length, r.target, cachedModels + (r.target - CachedModels));
    }

    for (const uintptr_t place: {0x1401AACA2, 0x1401AAD00})
    {
      Utils::Hook::Rebase(place, 8, ImageBase, CachedModels, cachedModels);
    }

    // Reallocate cg models
    //
    // The same again, with two more lookups that reach the models from
    // cgsArray. Among these is also the loop in CG_RegisterGraphics() that
    // registers the models, where on x86 we hook its R_RegisterModel() call
    // instead.
    //
    for (const Reference& r: std::initializer_list<Reference>
    {
      {0x1400D4C78, 7, GameModels}, {0x1400D57FF, 7, ClientGameStatic}, {0x1400D5850, 7, ClientGameStatic},
      {0x1400D9C7A, 7, GameModels + sizeof(Game::XModel*)}, {0x1400D9C81, 7, GameModels + BaseModelsSize}
    })
    {
      Utils::Hook::Displace(r.place, r.length, r.target, gameModels + (r.target - GameModels));
    }

    for (const uintptr_t place: {0x1400D533A, 0x1400D546B, 0x1400E6185, 0x1400EAEC9, 0x1402965E2})
    {
      Utils::Hook::Rebase(place, 8, ImageBase, GameModels, gameModels);
    }

    // Patch limit
    Utils::Hook::Replace32(0x1401AA2D9, 6, BaseModelsSize, G_MODELINDEX_LIMIT * sizeof(Game::XModel*));
    Utils::Hook::Replace32(0x1401AC04C, 6, BASE_GMODEL_COUNT, G_MODELINDEX_LIMIT);
    Utils::Hook::Replace32(0x1401AC062, 6, BASE_GMODEL_COUNT, G_MODELINDEX_LIMIT);

    modelsHaveBeenReallocated = true;
  }
}
