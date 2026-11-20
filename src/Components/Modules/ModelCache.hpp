#pragma once

#include "Weapon.hpp"

namespace Components
{
  class ModelCache : public Component
  {
  public:
    static const int BASE_GMODEL_COUNT = 512;

    // Double the limit to allow loading of some heavy-duty MW3 maps
    static const int ADDITIONAL_GMODELS = 512;

    static const int G_MODELINDEX_LIMIT = (BASE_GMODEL_COUNT + Weapon::WEAPON_LIMIT - Weapon::BASEGAME_WEAPON_LIMIT + ADDITIONAL_GMODELS);

    // Server
    //
    // Note that on x64 the game reaches the models through 32-bit
    // displacements, and so we allocate them where the game can reach them.
    //
    static Game::XModel** cached_models_reallocated;

    // Client game
    static Game::XModel** gameModels_reallocated;

    static bool modelsHaveBeenReallocated;

    ModelCache();
  };
}
