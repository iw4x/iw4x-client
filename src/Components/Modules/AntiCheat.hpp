#pragma once

#ifndef DEBUG
// Hide AntiCheat in embeded symbol names
#define AntiCheat SubComponent
#else
# ifndef DISABLE_ANTICHEAT
#  define DISABLE_ANTICHEAT
# endif
#endif

// Uncomment to enable process protection (conflicts with steam!)
#define PROCTECT_PROCESS

namespace Components
{
	class AntiCheat : public Component
	{
	public:
		AntiCheat();
		~AntiCheat();

		class LibUnlocker
		{
		public:
			LibUnlocker()
			{
				UninstallLibHook();
			}
			~LibUnlocker()
			{
				InstallLibHook();
			}
		};

		static void CrashClient();

		static void InitLoadLibHook();

		static void ReadIntegrityCheck();
		static void ScanIntegrityCheck();
		static void FlagIntegrityCheck();

		static unsigned long ProtectProcess();

		static void PatchVirtualProtect(void* vp, void* vpex);
		static void PatchThreadCreation();

		static void VerifyThreadIntegrity();

		static void QuickCodeScanner1();
		static void QuickCodeScanner2();

		static void UninstallLibHook();
		static void InstallLibHook();

        static void CheckStartupTime();
        static void SystemTimeDiff(LPSYSTEMTIME stA, LPSYSTEMTIME stB, LPSYSTEMTIME stC);

	private:
		enum IntergrityFlag
		{
			NO_FLAG = (0),
			INITIALIZATION = (1 << 0),
			MEMORY_SCAN = (1 << 1),
			SCAN_INTEGRITY_CHECK = (1 << 2),

#ifdef PROCTECT_PROCESS
			READ_INTEGRITY_CHECK = (1 << 3),
#endif

			MAX_FLAG,
		};

		static Utils::Time::Interval LastCheck;
		static unsigned long Flags;

		static void PerformScan();
		static void PatchWinAPI();

		static void NullSub();

		static bool IsPageChangeAllowed(void* callee, void* addr, size_t len);
		static void AssertCalleeModule(void* callee);

#ifdef DEBUG_LOAD_LIBRARY
		static HANDLE LoadLibary(std::wstring library, HANDLE file, DWORD flags, void* callee);
		static HANDLE WINAPI LoadLibaryAStub(const char* library);
		static HANDLE WINAPI LoadLibaryWStub(const wchar_t* library);
		static HANDLE WINAPI LoadLibaryExAStub(const char* library, HANDLE file, DWORD flags);
		static HANDLE WINAPI LoadLibaryExWStub(const wchar_t* library, HANDLE file, DWORD flags);
#endif

		static BOOL WINAPI VirtualProtectStub(LPVOID lpAddress, SIZE_T dwSize, DWORD  flNewProtect, PDWORD lpflOldProtect);
		static BOOL WINAPI VirtualProtectExStub(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect);

		static void LostD3DStub();
		static void CinematicStub();
		static void SoundInitStub(int a1, int a2, int a3);
		static void SoundInitDriverStub();

		static void DObjGetWorldTagPosStub();
		static void AimTargetGetTagPosStub();

		static void AcquireDebugPrivilege(HANDLE hToken);

		static NTSTATUS NTAPI NtCreateThreadExStub(PHANDLE hThread, ACCESS_MASK desiredAccess, LPVOID objectAttributes, HANDLE processHandle, LPTHREAD_START_ROUTINE startAddress, LPVOID parameter, BOOL createSuspended, DWORD stackZeroBits, DWORD sizeOfStackCommit, DWORD sizeOfStackReserve, LPVOID bytesBuffer);
		static int ValidateThreadTermination(void* addr);
		static void ThreadEntryPointStub();

		static std::mutex ThreadMutex;
		static std::vector<DWORD> OwnThreadIds;
		static std::map<DWORD, std::shared_ptr<Utils::Hook>> ThreadHookMap;

		static Utils::Hook CreateThreadHook;
		static Utils::Hook LoadLibHook[6];
		static Utils::Hook VirtualProtectHook[2];
	};
}

