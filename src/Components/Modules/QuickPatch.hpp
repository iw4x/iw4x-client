namespace Components
{
	class QuickPatch : public Component
	{
	public:
		typedef void(Callback)();

		QuickPatch();
		~QuickPatch();
		const char* GetName() { return "QuickPatch"; };

		static void UnlockStats();
		static void OnShutdown(Callback* callback);

	private:
		static wink::signal<wink::slot<Callback>> ShutdownSignal;

		static int64_t* GetStatsID();
		static void ShutdownStub(int channel, const char* message);
	};
}
