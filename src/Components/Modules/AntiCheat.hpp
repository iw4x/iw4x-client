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

		static void UninstallLibHook();
		static void InstallLibHook();

		static void SoundInitStub();
		static BOOL WINAPI VirtualProtectStub(LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect);

		static Utils::Hook LoadLibHook[4];
		static Utils::Hook VirtualProtectHook;
	};
}
