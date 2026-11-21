#pragma once

extern "C"
{
  void QuickPatch_SetAspectRatio_Stub();
  void QuickPatch_CL_KeyEvent_ConsoleEscape_Stub();
  void QuickPatch_VehicleFx_PlayerIndexCheck_Stub();
  void QuickPatch_VehicleCl_SetPlayerIndex_UpdateEntity_Stub();
  void QuickPatch_VehicleCl_SetPlayerIndex_ResetEntity_Stub();

  void QuickPatch_SetAspectRatio();
  void QuickPatch_CL_KeyEvent_OnEscape();
}

namespace Components
{
  class QuickPatch : public Component
  {
  public:
    QuickPatch();

    static void UnlockStats();

  private:
    friend void (::QuickPatch_SetAspectRatio)();
    friend void (::QuickPatch_CL_KeyEvent_OnEscape)();

    static Game::dvar_t* g_antilag;
    static void FireWeapon_Hk(Game::gentity_s* ent, int gameTime, int eventParm);
    static void FireWeaponMelee_Hk(Game::gentity_s* ent, int gameTime);

    static Dvar::Var UIMousePitch;

    static Dvar::Var r_customAspectRatio;
    static Game::dvar_t* Dvar_RegisterAspectRatioDvar(const char* dvarName, const char** valueList, int defaultIndex, unsigned __int16 flags, const char* description);
    static void SetAspectRatio();

    static void CL_InitRef_Hk(Game::GfxConfiguration* config);
    static void R_EnumDisplayModes_Hk(unsigned int adapterIndex);

    static void CL_KeyEvent_OnEscape();

    static void R_AddImageToList_Hk(Game::XAssetHeader header, void* data);

    static int SND_GetAliasOffset(const Game::snd_alias_t* alias);

    static void Sys_SpawnQuitProcess_Hk();

    static Game::dvar_t* Dvar_RegisterScoreboardPingText(const char* dvarName, bool value, unsigned __int16 flags, const char* description);

#ifdef DEBUG
    static Game::dvar_t* Dvar_RegisterDeveloper(const char* dvarName, int value, int min, int max, unsigned __int16 flags, const char* description);
    static Game::dvar_t* Dvar_RegisterDeveloperScript(const char* dvarName, bool value, unsigned __int16 flags, const char* description);
#endif

    static int FX_ImpactMarkCallback_Hk(void* context, int pointCount, void* points, int triCount, void* tris, const float* origin, const float* texCoordAxis);
  };
}
