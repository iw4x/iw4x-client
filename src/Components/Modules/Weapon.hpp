#pragma once

extern "C"
{
  extern const Game::dvar_t* Weapon_BGWeaponOffHandFix;

  void Weapon_CG_SelectWeaponIndex_Stub();
  void Weapon_WeaponEntCanBeGrabbed_Stub();
  void Weapon_CG_DrawItem_Stub();
  void Weapon_CG_CreateViewModelTree_Stub();
  void Weapon_JavelinResetHook_Stub();
}

namespace Components
{
  class Weapon : public Component
  {
  public:
    Weapon();

    // Note that on x86 the game's limit is 1200. x64 has 1400, the same as the
    // Steam version, and keeps the configstrings of the weapons past 1200
    // where we keep them on x86, from CS_WEAPONFILES_EXTRA on.
    //
    static const int BASEGAME_WEAPON_LIMIT = 1400;

    // Increase the weapon limit
    static const int WEAPON_LIMIT = 2400;

    static const int ADDED_WEAPONS = WEAPON_LIMIT - BASEGAME_WEAPON_LIMIT;

    // The first weapon whose configstring is past CS_WEAPONFILES_LAST.
    //
    static const int EXTRA_CONFIGSTRING_WEAPONS_FIRST = 1200;

    static const Game::dvar_t* BGDisableDoubleTaps;

  private:
    static const Game::dvar_t* CGRecoilMultiplier;

    struct RawWeapon
    {
      Game::WeaponCompleteDef complete;
      Game::WeaponDef def;
    };

    static std::unordered_map<std::string, RawWeapon> RawWeapons;

    static std::uint16_t* AltModePrimaries;

    static Game::XAssetHeader LoadWeaponCompleteDef(Game::XAssetType type, const char* name);
    static void PatchLimit();
    static void* LoadNoneWeaponHook();

    static void* CG_ClearState(void* cg, int value, std::size_t size);

    static void BG_WeaponFireRecoil_Stub(void* ps, float* recoilSpeed, float* kickAVel, unsigned int* holdrand, Game::PlayerHandIndex hand);

    static Game::gentity_s* Drop_Weapon_Hk(Game::gentity_s* ent, unsigned int weapon, unsigned int model);

    static void PlayerCmd_InitialWeaponRaise(Game::scr_entref_t entref);
    static void PlayerCmd_FreezeControlsAllowLook(Game::scr_entref_t entref);

    static void AddScriptMethods();

    static void PM_Weapon_stub(Game::pmove_s* pm, Game::pml_t* pml);
  };
}
