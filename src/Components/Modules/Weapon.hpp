#pragma once

extern "C"
{
  extern const Game::dvar_t* Weapon_BGWeaponOffHandFix;

  void Weapon_LoadNoneWeaponHookStub();
  void Weapon_CG_UpdatePrimaryForAltModeWeapon_Stub();
  void Weapon_CG_SelectWeaponIndex_Stub();
  void Weapon_WeaponEntCanBeGrabbed_Stub();
  void Weapon_CG_DrawItem_Stub();
  void Weapon_CG_CreateViewModelTree_Stub();
  void Weapon_JavelinResetHook_Stub();

  void* Weapon_LoadNoneWeaponHook();
}

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
    static const Game::dvar_t* BGDisableDoubleTaps;

  private:
    friend void* (::Weapon_LoadNoneWeaponHook)();

    static const Game::dvar_t* CGRecoilMultiplier;

    static Game::WeaponCompleteDef* LoadWeaponCompleteDef(const char* name);
    static void PatchLimit();
    static void* LoadNoneWeaponHook();

    static void SaveRegisteredWeapons();

    static void PatchHintStrings();

    static void BG_WeaponFireRecoil_Stub(void* ps, float* recoilSpeed, float* kickAVel, unsigned int* holdrand, Game::PlayerHandIndex hand);

    static Game::gentity_s* Drop_Weapon_Hk(Game::gentity_s* ent, unsigned int weapon, unsigned int model);

    static void PlayerCmd_InitialWeaponRaise(Game::scr_entref_t entref);
    static void PlayerCmd_FreezeControlsAllowLook(Game::scr_entref_t entref);

    static void AddScriptMethods();

    static void PM_Weapon_stub(Game::pmove_s* pm, Game::pml_t* pml);
  };
}
