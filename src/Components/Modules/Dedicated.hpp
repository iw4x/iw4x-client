namespace Components
{
	class Dedicated : public Component
	{
	public:
		typedef void(Callback)();

		Dedicated();
		~Dedicated();
		const char* GetName() { return "Dedicated"; };

		static bool IsDedicated();

		static void Heartbeat();

		static void OnFrame(Callback* callback);

	private:
		static wink::signal<wink::slot<Callback>> FrameSignal;

		static void MapRotate();
		static void FrameStub();
		static void InitDedicatedServer();

		static void PostInitialization();
		static void PostInitializationStub();
	};
}
