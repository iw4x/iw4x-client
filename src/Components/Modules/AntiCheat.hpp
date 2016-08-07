// Uncomment that to see if we are preventing necessary libraries from being loaded
//#define DEBUG_LOAD_LIBRARY

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

		static void IntegrityCheck();

	private:
		static int LastCheck;
		static std::string Hash;

		static void Frame();
		static void PerformCheck();
		static void PatchWinAPI();

		static void NullSub();

		static void AssertLibraryCall(void* callee);
		static void AssertProcessCall(void* callee);
		static void AssertModuleCall(HMODULE module, void* callee);

		static void UninstallLibHook();
		static void InstallLibHook();

#ifdef DEBUG_LOAD_LIBRARY
		static HANDLE LoadLibary(std::wstring library, void* callee);
		static HANDLE WINAPI LoadLibaryAStub(const char* library);
		static HANDLE WINAPI LoadLibaryWStub(const wchar_t* library);
#endif

		static void CinematicStub();
		static void SoundInitStub(int a1, int a2, int a3);
		static void SoundInitDriverStub();

		static void AimTargetGetTagPosStub();

		static Utils::Hook LoadLibHook[4];
	};
}
