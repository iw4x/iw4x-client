#pragma once

// Increase the weapon limit
// Was 1200 before
#define WEAPON_LIMIT 2400
#define MAX_CONFIGSTRINGS (4139 - 1200 + WEAPON_LIMIT)
#define G_MODELINDEX_LIMIT (526 + WEAPON_LIMIT - 1200) 

namespace Components
{
	class Weapon : public Component
	{
	public:
		Weapon();
		static Game::XModel* G_ModelIndexReallocated[G_MODELINDEX_LIMIT];

		static bool GModelIndexHasBeenReallocated;

	private:
		static const Game::dvar_t* BGWeaponOffHandFix;

		static Game::WeaponCompleteDef* LoadWeaponCompleteDef(const char* name);
		static void PatchLimit();
		static void* LoadNoneWeaponHook();
		static void LoadNoneWeaponHookStub();
		static void PatchConfigStrings();

		static const char* GetWeaponConfigString(int index);
		static void SaveRegisteredWeapons();

		static void ParseConfigStrings();
		static int ParseWeaponConfigStrings();
		static int ClearConfigStrings(void* dest, int value, int size);

		static void CG_UpdatePrimaryForAltModeWeapon_Stub();
		static void CG_SelectWeaponIndex_Stub();

		static void JavelinResetHook_Stub();

		static void WeaponEntCanBeGrabbed_Stub();

		static void PlayerCmd_InitialWeaponRaise(Game::scr_entref_t entref);
		static void PlayerCmd_FreezeControlsAllowLook(Game::scr_entref_t entref);

		static void AddScriptMethods();
	};
}
