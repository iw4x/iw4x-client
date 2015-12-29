namespace Components
{
	class Dedicated : public Component
	{
	public:
		typedef void(*Callback)();

		Dedicated();
		~Dedicated();
		const char* GetName() { return "Dedicated"; };

		static bool IsDedicated();

		static void Heartbeat();

		static void OnFrame(Callback callback);

	private:
		static Dvar::Var Dedi;
		static std::vector<Callback> FrameCallbacks;

		static void MapRotate();
		static void FrameStub();
		static void InitDedicatedServer();
	};
}
