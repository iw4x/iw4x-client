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

	private:
		static int LastCheck;
		static std::string Hash;

		static void Frame();
		static void PerformCheck();

		static void NullSub();
	};
}
