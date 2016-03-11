namespace Components
{
	class AntiCheat : public Component
	{
	public:
		AntiCheat();
		~AntiCheat();
		const char* GetName() { return "Component"; }; // Wrong name :P

		static void CrashClient();
		static void EmptyHash();

		static void InitLoadLibHook();

	private:
		static int LastCheck;
		static std::string Hash;

		static void Frame();
		static void PerformCheck();
		static void PatchWinAPI();

		static void NullSub();

		static Utils::Hook LoadLibHook[4];
		static bool InjectPatches;
	};
}
