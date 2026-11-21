#pragma once

extern "C"
{
  void Renderer_BackendFrameStub();
  void Renderer_RB_SwapBuffers_PresentFailedStub();
  void Renderer_TextureFromCodeErrorStub1();
  void Renderer_TextureFromCodeErrorStub2();

  void Renderer_BackendFrameHandler();
  bool Renderer_RB_RecoverDevice(HRESULT result);
  void Renderer_R_TextureFromCodeError(const char* sampler, Game::GfxCmdBufState* state, int samplerCode);
}

namespace Components
{
  class Renderer : public Component
  {
  public:
    typedef void(BackendCallback)(IDirect3DDevice9*);
    typedef void(Callback)();

    Renderer();
    ~Renderer();

    static int Width();
    static int Height();

    static void OnBackendFrame(Utils::Slot<BackendCallback> callback);
    static void OnNextBackendFrame(Utils::Slot<BackendCallback> callback);

    static void OnDeviceRecoveryEnd(Utils::Slot<Renderer::Callback> callback);
    static void OnDeviceRecoveryBegin(Utils::Slot<Renderer::Callback> callback);

  private:
    friend void (::Renderer_BackendFrameHandler)();
    friend bool (::Renderer_RB_RecoverDevice)(HRESULT result);
    friend void (::Renderer_R_TextureFromCodeError)(const char* sampler, Game::GfxCmdBufState* state, int samplerCode);

    static void BackendFrameHandler();

    static void PreVidRestart();
    static void PostVidRestart();
    static void R_Shutdown_Hk(int destroyWindow);
    static void CL_InitRenderer_Hk();

    static bool RB_RecoverDevice (HRESULT result);

    static void R_Cinematic_FreeMemory(void* memory);

    static Game::dvar_t* RegisterDisplayModeDvar(const char* dvarName, const char** valueList, int defaultIndex, unsigned __int16 flags, const char* description);

    static void R_TextureFromCodeError(const char* sampler, Game::GfxCmdBufState* state, int samplerCode);

    static void DebugDrawTriggers();
    static void DebugDrawSceneModelCollisions();
    static void DebugDrawModelBoundingBoxes();
    static void DebugDrawModelNames();
    static void DebugDrawRunners();
    static void DebugDrawAABBTrees();
    static void DebugDrawClipmap();
    static void ForceTechnique();
    static void ListSamplers();
    static void DrawPrimaryLights();

    static int FixSunShadowPartitionSize(Game::GfxCamera* camera, Game::GfxSunShadowMapMetrics* mapMetrics, Game::GfxSunShadow* sunShadow, Game::GfxSunShadowClip* clip, float* partitionFraction);

    static Utils::Signal<Renderer::Callback> EndRecoverDeviceSignal;
    static Utils::Signal<Renderer::Callback> BeginRecoverDeviceSignal;

    static Utils::Signal<BackendCallback> BackendFrameSignal;
    static Utils::Signal<BackendCallback> SingleBackendFrameSignal;

    static Dvar::Var r_drawTriggers;
    static Dvar::Var r_drawSceneModelCollisions;
    static Dvar::Var r_drawModelBoundingBoxes;
    static Dvar::Var r_drawModelNames;
    static Dvar::Var r_drawRunners;
    static Dvar::Var r_drawAABBTrees;
    static Dvar::Var r_playerDrawDebugDistance;
    static Dvar::Var r_forceTechnique;
    static Dvar::Var r_listSamplers;
    static Dvar::Var r_drawLights;
    static Dvar::Var r_drawClipmap;
  };
}
