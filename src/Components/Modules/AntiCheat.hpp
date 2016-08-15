namespace Components
{
	class AntiCheat : public Component
	{
	public:
		AntiCheat();
		~AntiCheat();

#ifdef DEBUG
		const char* GetName() { return "AntiCheat"; };
#endif

		static void CrashClient();
		static void EmptyHash();

		static void InitLoadLibHook();

		static void ReadIntegrityCheck();
		static void ScanIntegrityCheck();
		static void FlagIntegrityCheck();

	private:
		enum IntergrityFlag
		{
			NO_FLAG = (0),
			INITIALIZATION = (1 << 0),
			MEMORY_SCAN = (1 << 1),
			SCAN_INTEGRITY_CHECK = (1 << 2),
			READ_INTEGRITY_CHECK = (1 << 3),

			MAX_FLAG,
		};

		static int LastCheck;
		static std::string Hash;
		static unsigned long Flags;

		static void Frame();
		static void PerformCheck();
		static void PatchWinAPI();

		static DWORD ProtectProcess();

		static void NullSub();

		static void AssertCalleeModule(void* callee);

		static void UninstallLibHook();
		static void InstallLibHook();

#ifdef DEBUG_LOAD_LIBRARY
		static HANDLE LoadLibary(std::wstring library, void* callee);
		static HANDLE WINAPI LoadLibaryAStub(const char* library);
		static HANDLE WINAPI LoadLibaryWStub(const wchar_t* library);
#endif

		static void LostD3DStub();
		static void CinematicStub();
		static void SoundInitStub(int a1, int a2, int a3);
		static void SoundInitDriverStub();

		static void DObjGetWorldTagPosStub();
		static void AimTargetGetTagPosStub();

		static Utils::Hook LoadLibHook[4];
	};
}
