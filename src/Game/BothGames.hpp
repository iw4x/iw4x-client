#pragma once

namespace Game
{
	typedef unsigned int(*BG_GetNumWeapons_t)();
	extern BG_GetNumWeapons_t BG_GetNumWeapons;

	typedef const char*(*BG_GetWeaponName_t)(unsigned int index);
	extern BG_GetWeaponName_t BG_GetWeaponName;

	typedef void*(*BG_LoadWeaponDef_LoadObj_t)(const char* name);
	extern BG_LoadWeaponDef_LoadObj_t BG_LoadWeaponDef_LoadObj;

	typedef WeaponCompleteDef*(*BG_LoadWeaponCompleteDefInternal_t)(const char* folder, const char* name);
	extern BG_LoadWeaponCompleteDefInternal_t BG_LoadWeaponCompleteDefInternal;

	typedef WeaponDef*(*BG_GetWeaponDef_t)(unsigned int weaponIndex);
	extern BG_GetWeaponDef_t BG_GetWeaponDef;

	typedef const char*(*BG_GetEntityTypeName_t)(int eType);
	extern BG_GetEntityTypeName_t BG_GetEntityTypeName;
}
