#pragma once

extern "C"
{
  extern Game::dvar_t* QuickPatch_g_antilag;

  void QuickPatch_ClientEventsFireWeapon_Stub();
  void QuickPatch_ClientEventsFireWeaponMelee_Stub();
  void QuickPatch_SetAspectRatio_Stub();
  void QuickPatch_CL_KeyEvent_ConsoleEscape_Stub();
  void QuickPatch_SND_GetAliasOffset_Stub();
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

    static Dvar::Var UIMousePitch;

    static Dvar::Var r_customAspectRatio;
    static Game::dvar_t* Dvar_RegisterAspectRatioDvar(const char* dvarName, const char** valueList, int defaultIndex, unsigned __int16 flags, const char* description);
    static void SetAspectRatio();

    static void CL_InitRef_Hk(Game::GfxConfiguration* config);
    static void R_EnumDisplayModes_Hk(unsigned int adapterIndex);

    static BOOL IsDynClassname_Stub(const char* classname);

    static void CL_KeyEvent_OnEscape();

    static void R_AddImageToList_Hk(Game::XAssetHeader header, void* data);

    static void Sys_SpawnQuitProcess_Hk();

    static Game::dvar_t* Dvar_RegisterConMinicon(const char* dvarName, bool value, unsigned __int16 flags, const char* description);

    static int FX_ImpactMarkCallback_Hk(void* context, int pointCount, void* points, int triCount, void* tris, const float* origin, const float* texCoordAxis);
  };
}
