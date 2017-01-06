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
		const char* getName() { return "Renderer"; };
#endif

		static int Width();
		static int Height();

		static void Once(Utils::Slot<Callback> callback);
		static void OnFrame(Utils::Slot<Callback> callback);
		static void OnBackendFrame(Utils::Slot<BackendCallback> callback);

		static void OnDeviceRecoveryEnd(Utils::Slot<Callback> callback);
		static void OnDeviceRecoveryBegin(Utils::Slot<Callback> callback);

	private:
		static void FrameStub();
		static void FrameHandler();

		static void BackendFrameStub();
		static void BackendFrameHandler();

		static Utils::Signal<Callback> FrameSignal;
		static Utils::Signal<Callback> FrameOnceSignal;

		static Utils::Signal<Callback> EndRecoverDeviceSignal;
		static Utils::Signal<Callback> BeginRecoverDeviceSignal;

		static Utils::Signal<BackendCallback> BackendFrameSignal;
		static Utils::Hook DrawFrameHook;
	};
}
