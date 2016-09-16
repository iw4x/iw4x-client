namespace Components
{
	class Renderer : public Component
	{
	public:
		typedef void(Callback)();
		typedef void(BackendCallback)(IDirect3DDevice9*);

		Renderer();
		~Renderer();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* GetName() { return "Renderer"; };
#endif

		static int Width();
		static int Height();

		static void Once(Callback* callback);
		static void OnFrame(Callback* callback);
		static void OnBackendFrame(BackendCallback* callback);

		static void OnDeviceRecoveryEnd(Callback* callback);
		static void OnDeviceRecoveryBegin(Callback* callback);

	private:
		static void FrameStub();
		static void FrameHandler();

		static void BackendFrameStub();
		static void BackendFrameHandler();

		static wink::signal<wink::slot<Callback>> FrameSignal;
		static wink::signal<wink::slot<Callback>> FrameOnceSignal;

		static wink::signal<wink::slot<Callback>> EndRecoverDeviceSignal;
		static wink::signal<wink::slot<Callback>> BeginRecoverDeviceSignal;

		static wink::signal<wink::slot<BackendCallback>> BackendFrameSignal;
		static Utils::Hook DrawFrameHook;
	};
}
