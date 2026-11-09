#pragma once

extern "C"
{
  void Renderer_BackendFrameStub();
  void Renderer_RB_SwapBuffers_PresentFailedStub();
  void Renderer_PostVidRestartStub();
  void Renderer_StoreGfxBufContextPtrStub1();
  void Renderer_StoreGfxBufContextPtrStub2();

  void Renderer_BackendFrameHandler();
  bool Renderer_RB_RecoverDevice(HRESULT result);
  void Renderer_PostVidRestart();
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
    friend void (::Renderer_PostVidRestart)();
    friend void (::Renderer_R_TextureFromCodeError)(const char* sampler, Game::GfxCmdBufState* state, int samplerCode);

    static void BackendFrameHandler();

    static void PreVidRestart();
    static void PostVidRestart();

    static bool RB_RecoverDevice (HRESULT result);

    static void R_Cinematic_FreeMemory(void* memory);

    static Game::dvar_t* RegisterDisplayModeDvar(const char* dvarName, const char** valueList, int defaultIndex, unsigned __int16 flags, const char* description);

    static void R_TextureFromCodeError(const char* sampler, Game::GfxCmdBufState* state, int samplerCode);

    static int DrawTechsetForMaterial(int a1, float a2, float a3, const char* material, Game::vec4_t* color, int a6);

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
