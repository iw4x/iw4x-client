#include <STDInclude.hpp>

namespace Game
{
	G_LogPrintf_t G_LogPrintf = G_LogPrintf_t(0x4B0150);
	G_GetWeaponIndexForName_t G_GetWeaponIndexForName = G_GetWeaponIndexForName_t(0x49E540);
	G_SpawnEntitiesFromString_t G_SpawnEntitiesFromString = G_SpawnEntitiesFromString_t(0x4D8840);
	G_Spawn_t G_Spawn = G_Spawn_t(0x4226F0);
	G_FreeEntity_t G_FreeEntity = G_FreeEntity_t(0x44C9D0);
	G_SpawnItem_t G_SpawnItem = G_SpawnItem_t(0x403770);
	G_GetItemClassname_t G_GetItemClassname = G_GetItemClassname_t(0x492630);
	G_PrintEntities_t G_PrintEntities = G_PrintEntities_t(0x4E6A50);
	G_GetEntityTypeName_t G_GetEntityTypeName = G_GetEntityTypeName_t(0x4EB810);

	gentity_t* g_entities = reinterpret_cast<gentity_t*>(0x18835D8);

	XModel* G_GetModel(const int index)
	{
		assert(index > 0);
		assert(index < MAX_MODELS);
		return cached_models[index];
	}
}
