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

    // Note that we count the added weapons from the 1200 of x86, the same as
    // an x86 server does. The client state sends a model index in as many
    // bits as this limit takes, which is 12 on x86. So what if we counted
    // from x64's own limit instead? Then the width would no longer agree with
    // an x86 server's and every model index would come out garbled.
    //
    static const int G_MODELINDEX_LIMIT = (BASE_GMODEL_COUNT + Weapon::WEAPON_LIMIT - 1200 + ADDITIONAL_GMODELS);

    static_assert(std::bit_width(static_cast<unsigned int>(G_MODELINDEX_LIMIT - 1)) == 12);

    // Server
    //
    // Note that on x64 the game reaches the models through 32-bit
    // displacements, so we allocate them where it can reach them.
    //
    static Game::XModel** cached_models_reallocated;

    // Client game
    static Game::XModel** gameModels_reallocated;

    static bool modelsHaveBeenReallocated;

    ModelCache();
  };
}
