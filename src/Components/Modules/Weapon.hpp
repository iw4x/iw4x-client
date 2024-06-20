#pragma once



namespace Components
{
	class Weapon : public Component
	{
	public:
		Weapon();
		static const int BASEGAME_WEAPON_LIMIT = 1200;

		// Increase the weapon limit
		static const int WEAPON_LIMIT = 2400;

		static const int ADDED_WEAPONS = WEAPON_LIMIT - BASEGAME_WEAPON_LIMIT;

	private:
		static const Game::dvar_t* BGWeaponOffHandFix;

		static Game::WeaponCompleteDef* LoadWeaponCompleteDef(const char* name);
		static void PatchLimit();
		static void* LoadNoneWeaponHook();
		static void LoadNoneWeaponHookStub();

		static void SaveRegisteredWeapons();

		static void CG_UpdatePrimaryForAltModeWeapon_Stub();
		static void CG_SelectWeaponIndex_Stub();

		static void JavelinResetHook_Stub();

		static void WeaponEntCanBeGrabbed_Stub();

		static void PlayerCmd_InitialWeaponRaise(Game::scr_entref_t entref);
		static void PlayerCmd_FreezeControlsAllowLook(Game::scr_entref_t entref);

		static void AddScriptMethods();
	};
}
