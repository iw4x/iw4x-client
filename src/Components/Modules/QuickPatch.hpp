namespace Components
{
	class QuickPatch : public Component
	{
	public:
		typedef void(*Callback)();

		QuickPatch();
		~QuickPatch();
		const char* GetName() { return "QuickPatch"; };

		static void UnlockStats();
		static void OnShutdown(Callback callback);

	private:
		static std::vector<Callback> ShutdownCallbacks;

		static int64_t* GetStatsID();
		static void ShutdownStub(int channel, const char* message);
	};
}
