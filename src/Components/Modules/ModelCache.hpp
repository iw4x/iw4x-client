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
		static Game::XModel* cached_models_reallocated[G_MODELINDEX_LIMIT];

		// Client game
		static Game::XModel* gameModels_reallocated[G_MODELINDEX_LIMIT];

		static bool modelsHaveBeenReallocated;

		static void R_RegisterModel_InitGraphics(const char* name, void* atAddress);
		static void R_RegisterModel_Hook();

		ModelCache();
	};
}
