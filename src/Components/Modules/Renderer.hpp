#pragma once

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
		static void BackendFrameStub();
		static void BackendFrameHandler();

		static void PreVidRestart();
		static void PostVidRestart();
		static void PostVidRestartStub();

		static void R_TextureFromCodeError(const char* sampler, Game::GfxCmdBufState* state, int samplerCode);
		static void StoreGfxBufContextPtrStub1();
		static void StoreGfxBufContextPtrStub2();

		static int DrawTechsetForMaterial(int a1, float a2, float a3, const char* material, Game::vec4_t* color, int a6);

		static void DebugDrawTriggers();
		static void DebugDrawSceneModelCollisions();
		static void DebugDrawModelBoundingBoxes();
		static void DebugDrawModelNames();
		static void DebugDrawRunners();
		static void DebugDrawAABBTrees();
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
	};
}
