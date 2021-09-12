#include "STDInclude.hpp"

namespace Components
{
	Utils::Time::Interval AntiCheat::LastCheck;
	Utils::Hook AntiCheat::CreateThreadHook;
	Utils::Hook AntiCheat::LoadLibHook[6];
	Utils::Hook AntiCheat::VirtualProtectHook[2];
	unsigned long AntiCheat::Flags = NO_FLAG;

	std::mutex AntiCheat::ThreadMutex;
	std::vector<DWORD> AntiCheat::OwnThreadIds;
	std::map<DWORD, std::shared_ptr<Utils::Hook>> AntiCheat::ThreadHookMap;

	// This function does nothing, it only adds the two passed variables and returns the value
	// The only important thing it does is to clean the first parameter, and then return
	// By returning, the crash procedure will be called, as it hasn't been cleaned from the stack
	__declspec(naked) void AntiCheat::NullSub()
	{
		__asm
		{
			push ebp
			push ecx
			mov ebp, esp

			xor eax, eax
			mov eax, [ebp + 8h]
			mov ecx, [ebp + 0Ch]
			add eax, ecx

			pop ecx
			pop ebp
			retn 4
		}
	}

	void AntiCheat::CrashClient()
	{
		__VMProtectBeginUltra("");
#ifdef DEBUG_DETECTIONS
		Logger::Flush();
		MessageBoxA(nullptr, "Check the log for more information!", "AntiCheat triggered", MB_ICONERROR);
		ExitProcess(0xFFFFFFFF);
#else
		static std::thread triggerThread;
		if (!triggerThread.joinable())
		{
			triggerThread = std::thread([]()
			{
				std::this_thread::sleep_for(43s);
				Utils::Hook::Set<BYTE>(0x41BA2C, 0xEB);
			});
		}
#endif
		__VMProtectEnd;
	}

	void AntiCheat::AssertCalleeModule(void* callee)
	{
		__VMProtectBeginUltra("");
		HMODULE hModuleSelf = nullptr, hModuleTarget = nullptr, hModuleProcess = GetModuleHandleA(nullptr);
		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<char*>(callee), &hModuleTarget);
		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<char*>(AntiCheat::AssertCalleeModule), &hModuleSelf);

		if (!hModuleSelf || !hModuleTarget || !hModuleProcess || (hModuleTarget != hModuleSelf && hModuleTarget != hModuleProcess))
		{
#ifdef DEBUG_DETECTIONS
			char buffer[MAX_PATH] = { 0 };
			GetModuleFileNameA(hModuleTarget, buffer, sizeof buffer);

			Logger::Print(Utils::String::VA("AntiCheat: Callee assertion failed: %X %s", reinterpret_cast<uint32_t>(callee), buffer));
#endif

			AntiCheat::CrashClient();
		}
		__VMProtectEnd;
	}

	void AntiCheat::InitLoadLibHook()
	{
		__VMProtectBeginUltra("");
		static uint8_t kernel32Str[] = { 0xB4, 0x9A, 0x8D, 0xB1, 0x9A, 0x93, 0xCC, 0xCD, 0xD1, 0x9B, 0x93, 0x93 }; // KerNel32.dll
		static uint8_t loadLibAStr[] = { 0xB3, 0x90, 0x9E, 0x9B, 0xB3, 0x96, 0x9D, 0x8D, 0x9E, 0x8D, 0x86, 0xBE }; // LoadLibraryA
		static uint8_t loadLibWStr[] = { 0xB3, 0x90, 0x9E, 0x9B, 0xB3, 0x96, 0x9D, 0x8D, 0x9E, 0x8D, 0x86, 0xA8 }; // LoadLibraryW

		HMODULE kernel32 = GetModuleHandleA(Utils::String::XOR(std::string(reinterpret_cast<char*>(kernel32Str), sizeof kernel32Str), -1).data());
		if (kernel32)
		{
			FARPROC loadLibA = GetProcAddress(kernel32, Utils::String::XOR(std::string(reinterpret_cast<char*>(loadLibAStr), sizeof loadLibAStr), -1).data());
			FARPROC loadLibW = GetProcAddress(kernel32, Utils::String::XOR(std::string(reinterpret_cast<char*>(loadLibWStr), sizeof loadLibWStr), -1).data());

			std::string libExA = Utils::String::XOR(std::string(reinterpret_cast<char*>(loadLibAStr), sizeof loadLibAStr), -1);
			std::string libExW = Utils::String::XOR(std::string(reinterpret_cast<char*>(loadLibWStr), sizeof loadLibWStr), -1);

			libExA.insert(libExA.end() - 1, 'E');
			libExA.insert(libExA.end() - 1, 'x');

			libExW.insert(libExW.end() - 1, 'E');
			libExW.insert(libExW.end() - 1, 'x');

			FARPROC loadLibExA = GetProcAddress(kernel32, libExA.data());
			FARPROC loadLibExW = GetProcAddress(kernel32, libExW.data());

			if (loadLibA && loadLibW && loadLibExA && loadLibExW)
			{
#ifdef DEBUG_LOAD_LIBRARY
				AntiCheat::LoadLibHook[0].initialize(loadLibA, LoadLibaryAStub, HOOK_JUMP);
				AntiCheat::LoadLibHook[1].initialize(loadLibW, LoadLibaryWStub, HOOK_JUMP);
				AntiCheat::LoadLibHook[2].initialize(loadLibExA, LoadLibaryExAStub, HOOK_JUMP);
				AntiCheat::LoadLibHook[3].initialize(loadLibExW, LoadLibaryExWStub, HOOK_JUMP);
#else
				static uint8_t loadLibStub[] = { 0x33, 0xC0, 0xC2, 0x04, 0x00 }; // xor eax, eax; retn 04h
				static uint8_t loadLibExStub[] = { 0x33, 0xC0, 0xC2, 0x0C, 0x00 }; // xor eax, eax; retn 0Ch
				AntiCheat::LoadLibHook[0].initialize(loadLibA, loadLibStub, HOOK_JUMP);
				AntiCheat::LoadLibHook[1].initialize(loadLibW, loadLibStub, HOOK_JUMP);
				AntiCheat::LoadLibHook[2].initialize(loadLibExA, loadLibExStub, HOOK_JUMP);
				AntiCheat::LoadLibHook[3].initialize(loadLibExW, loadLibExStub, HOOK_JUMP);
#endif
			}
		}

		static uint8_t ldrLoadDllStub[] = { 0x33, 0xC0, 0xC2, 0x10, 0x00 };
		static uint8_t ldrLoadDll[] = { 0xB3, 0x9B, 0x8D, 0xB3, 0x90, 0x9E, 0x9B, 0xBB, 0x93, 0x93 }; // LdrLoadDll

		HMODULE ntdll = Utils::GetNTDLL();
		//AntiCheat::LoadLibHook[4].initialize(GetProcAddress(ntdll, Utils::String::XOR(std::string(reinterpret_cast<char*>(ldrLoadDll), sizeof ldrLoadDll), -1).data()), ldrLoadDllStub, HOOK_JUMP);

		// Patch LdrpLoadDll
		Utils::Hook::Signature::Container container;
		container.signature = "\x8B\xFF\x55\x8B\xEC\x83\xE4\xF8\x81\xEC\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x33\xC4\x89\x84\x24\x00\x00\x00\x00\x53\x8B\x5D\x10\x56\x57";
		container.mask = "xxxxxxxxxx????x????xxxxx????xxxxxx";
		container.callback = [](char* addr)
		{
			static uint8_t ldrpLoadDllStub[] = { 0x33, 0xC0, 0xC2, 0x0C, 0x00 };
			AntiCheat::LoadLibHook[5].initialize(addr, ldrpLoadDllStub, HOOK_JUMP);
		};

		Utils::Hook::Signature signature(ntdll, Utils::GetModuleSize(ntdll));
		signature.add(container);
		//signature.process();

		__VMProtectEnd;
	}

	void AntiCheat::ReadIntegrityCheck()
	{
		__VMProtectBeginUltra("");
#ifdef PROCTECT_PROCESS
		static Utils::Time::Interval check;

		if (check.elapsed(20s))
		{
			check.update();

			if (HANDLE h = OpenProcess(PROCESS_VM_READ, FALSE, GetCurrentProcessId()))
			{
#ifdef DEBUG_DETECTIONS
				Logger::Print("AntiCheat: Process integrity check failed");
#endif

				CloseHandle(h);
				AntiCheat::CrashClient();
			}
		}

		// Set the integrity flag
		AntiCheat::Flags |= AntiCheat::IntergrityFlag::READ_INTEGRITY_CHECK;
#endif
		__VMProtectEnd;
	}

	void AntiCheat::FlagIntegrityCheck()
	{
		__VMProtectBeginUltra("");
		static Utils::Time::Interval check;

		if (check.elapsed(30s))
		{
			check.update();

			unsigned long flags = ((AntiCheat::IntergrityFlag::MAX_FLAG - 1) << 1) - 1;

			if (AntiCheat::Flags != flags)
			{
#ifdef DEBUG_DETECTIONS
				Logger::Print(Utils::String::VA("AntiCheat: Flag integrity check failed: %X", AntiCheat::Flags));
#endif

				AntiCheat::CrashClient();
			}
		}
		__VMProtectEnd;
	}

	void AntiCheat::ScanIntegrityCheck()
	{
		__VMProtectBeginUltra("");
		// If there was no check within the last 40 seconds, crash!
		if (AntiCheat::LastCheck.elapsed(40s))
		{
#ifdef DEBUG_DETECTIONS
			Logger::Print("AntiCheat: Integrity check failed");
#endif

			AntiCheat::CrashClient();
		}

		// Set the integrity flag
		AntiCheat::Flags |= AntiCheat::IntergrityFlag::SCAN_INTEGRITY_CHECK;
		__VMProtectEnd;
	}

	void AntiCheat::PerformScan()
	{
		__VMProtectBeginUltra("");
		static std::optional<unsigned int> hashVal;

		// Perform check only every 20 seconds
		if (!AntiCheat::LastCheck.elapsed(20s)) return;
		AntiCheat::LastCheck.update();

		// Hash .text segment
		// Add 1 to each value, so searching in memory doesn't reveal anything
		size_t textSize = 0x2D6001;
		char* textBase = reinterpret_cast<char*>(0x401001);

		unsigned int hash = Utils::Cryptography::JenkinsOneAtATime::Compute(textBase - 1, textSize - 1);

		// Set the hash, if none is set
		if (!hashVal.has_value())
		{
			hashVal.emplace(hash);
		}
		// Crash if the hashes don't match
		else if (hashVal.value() != hash)
		{
#ifdef DEBUG_DETECTIONS
			Logger::Print("AntiCheat: Memory scan failed");
#endif

			AntiCheat::CrashClient();
		}

		// Set the memory scan flag
		AntiCheat::Flags |= AntiCheat::IntergrityFlag::MEMORY_SCAN;
		__VMProtectEnd;
	}

	void AntiCheat::QuickCodeScanner1()
	{
		__VMProtectBeginUltra("");
		static Utils::Time::Interval interval;
		static std::optional<unsigned int> hashVal;

		if (!interval.elapsed(32s)) return;
		interval.update();

		// Hash .text segment
		// Add 1 to each value, so searching in memory doesn't reveal anything
		size_t textSize = 0x2D5FFF;
		char* textBase = reinterpret_cast<char*>(0x400FFF);
		unsigned int hash = Utils::Cryptography::JenkinsOneAtATime::Compute(textBase + 1, textSize + 1);

		if (hashVal.has_value() && hash != hashVal.value())
		{
			Utils::Hook::Set<BYTE>(0x42A667, 0x90); // Crash
		}

		hashVal.emplace(hash);
		__VMProtectEnd;
	}

	void AntiCheat::QuickCodeScanner2()
	{
		__VMProtectBeginUltra("");
		static Utils::Time::Interval interval;
		static std::optional<unsigned int> hashVal;

		if (!interval.elapsed(42s)) return;
		interval.update();

		// Hash .text segment
		unsigned int hash = Utils::Cryptography::JenkinsOneAtATime::Compute(reinterpret_cast<char*>(0x401000), 0x2D6000);
		if (hashVal.has_value() && hash != hashVal.value())
		{
			Utils::Hook::Set<BYTE>(0x40797C, 0x90); // Crash
		}

		hashVal.emplace(hash);
		__VMProtectEnd;
	}

#ifdef DEBUG_LOAD_LIBRARY
	HANDLE AntiCheat::LoadLibary(std::wstring library, HANDLE file, DWORD flags, void* callee)
	{
		HMODULE module;
		char buffer[MAX_PATH] = { 0 };

		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<char*>(callee), &module);
		GetModuleFileNameA(module, buffer, sizeof buffer);

		MessageBoxA(nullptr, Utils::String::VA("Loading library %s via %s %X", std::string(library.begin(), library.end()).data(), buffer, reinterpret_cast<uint32_t>(callee)), nullptr, 0);

		AntiCheat::LoadLibHook[3].uninstall();
		HANDLE h = LoadLibraryExW(library.data(), file, flags);
		AntiCheat::LoadLibHook[3].install();
		return h;
	}

	HANDLE WINAPI AntiCheat::LoadLibaryAStub(const char* library)
	{
		std::string lib(library);
		return AntiCheat::LoadLibary(std::wstring(lib.begin(), lib.end()), nullptr, 0, _ReturnAddress());
	}

	HANDLE WINAPI AntiCheat::LoadLibaryWStub(const wchar_t* library)
	{
		return AntiCheat::LoadLibary(library, nullptr, 0, _ReturnAddress());
	}

	HANDLE WINAPI AntiCheat::LoadLibaryExAStub(const char* library, HANDLE file, DWORD flags)
	{
		std::string lib(library);
		return AntiCheat::LoadLibary(std::wstring(lib.begin(), lib.end()), file, flags, _ReturnAddress());
	}

	HANDLE WINAPI AntiCheat::LoadLibaryExWStub(const wchar_t* library, HANDLE file, DWORD flags)
	{
		return AntiCheat::LoadLibary(library, file, flags, _ReturnAddress());
	}
#endif

	void AntiCheat::UninstallLibHook()
	{
		for (int i = 0; i < ARRAYSIZE(AntiCheat::LoadLibHook); ++i)
		{
			AntiCheat::LoadLibHook[i].uninstall();
		}
	}

	void AntiCheat::InstallLibHook()
	{
		for (int i = 0; i < ARRAYSIZE(AntiCheat::LoadLibHook); ++i)
		{
			AntiCheat::LoadLibHook[i].install();
		}
	}

	void AntiCheat::PatchWinAPI()
	{
		LibUnlocker _;

		// Initialize directx
		Utils::Hook::Call<void()>(0x5078C0)();
	}

	void AntiCheat::SoundInitStub(int a1, int a2, int a3)
	{
		LibUnlocker _;
		Game::SND_Init(a1, a2, a3);
	}

	void AntiCheat::SoundInitDriverStub()
	{
		LibUnlocker _;
		Game::SND_InitDriver();
	}

	void AntiCheat::LostD3DStub()
	{
		LibUnlocker _;

		// Reset directx
		Utils::Hook::Call<void()>(0x508070)();
	}

	__declspec(naked) void AntiCheat::CinematicStub()
	{
		__asm
		{
			pushad
			call AntiCheat::UninstallLibHook
			popad

			call Game::R_Cinematic_StartPlayback_Now

			pushad
			call AntiCheat::InstallLibHook
			popad

			retn
		}
	}

	__declspec(naked) void AntiCheat::DObjGetWorldTagPosStub()
	{
		__asm
		{
			pushad
			push [esp + 20h]

			call AntiCheat::AssertCalleeModule

			pop esi
			popad

			push ecx
			mov ecx, [esp + 10h]

			push 426585h
			retn
		}
	}

	__declspec(naked) void AntiCheat::AimTargetGetTagPosStub()
	{
		__asm
		{
			pushad
			push [esp + 20h]

			call AntiCheat::AssertCalleeModule

			pop esi
			popad

			sub esp, 14h
			cmp dword ptr[esi + 0E0h], 1
			push 56AC6Ah
			ret
		}
	}

	bool AntiCheat::IsPageChangeAllowed(void* callee, void* addr, size_t len)
	{
		__VMProtectBeginUltra("");
		HMODULE hModuleSelf = nullptr, hModuleTarget = nullptr, hModuleMain = GetModuleHandle(nullptr);
		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<char*>(callee), &hModuleTarget);
		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<char*>(AntiCheat::IsPageChangeAllowed), &hModuleSelf);

		size_t mainSize = Utils::GetModuleSize(hModuleMain), selfSize = Utils::GetModuleSize(hModuleSelf);
		DWORD self = DWORD(hModuleSelf), main = DWORD(hModuleMain), address = DWORD(addr);

		// If the address that should be changed is within our module or the main binary, then we need to check if we are changing it or someone else
		if (Utils::HasIntercection(self, selfSize, address, len) || Utils::HasIntercection(main, mainSize, address, len))
		{
			if (!hModuleSelf || !hModuleTarget || (hModuleTarget != hModuleSelf))
			{
				return false;
			}
		}

		__VMProtectEnd;
		return true;
	}

	BOOL WINAPI AntiCheat::VirtualProtectStub(LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect)
	{
		__VMProtectBeginUltra("");
		if (!AntiCheat::IsPageChangeAllowed(_ReturnAddress(), lpAddress, dwSize)) return FALSE;

		AntiCheat::VirtualProtectHook[0].uninstall(false);
		BOOL result = VirtualProtect(lpAddress, dwSize, flNewProtect, lpflOldProtect);
		AntiCheat::VirtualProtectHook[0].install(false);

		__VMProtectEnd;
		return result;
	}

	BOOL WINAPI AntiCheat::VirtualProtectExStub(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect)
	{
		__VMProtectBeginUltra("");
		if (GetCurrentProcessId() == GetProcessId(hProcess) && !AntiCheat::IsPageChangeAllowed(_ReturnAddress(), lpAddress, dwSize)) return FALSE;

		AntiCheat::VirtualProtectHook[1].uninstall(false);
		BOOL result = VirtualProtectEx(hProcess, lpAddress, dwSize, flNewProtect, lpflOldProtect);
		AntiCheat::VirtualProtectHook[1].install(false);

		__VMProtectEnd;
		return result;
	}

	unsigned long AntiCheat::ProtectProcess()
	{
#ifdef PROCTECT_PROCESS
		__VMProtectBeginUltra("");

		Utils::Memory::Allocator allocator;

		HANDLE hToken = nullptr;
		if (!OpenProcessToken(GetCurrentProcess(), /*TOKEN_ADJUST_PRIVILEGES | */TOKEN_READ, &hToken))
		{
			if (!OpenThreadToken(GetCurrentThread(), /*TOKEN_ADJUST_PRIVILEGES | */TOKEN_READ, TRUE, &hToken))
			{
				return GetLastError();
			}
		}

		auto freeSid = [](void* sid)
		{
			if (sid)
			{
				FreeSid(reinterpret_cast<PSID>(sid));
			}
		};

		allocator.reference(hToken, [](void* hToken)
		{
			if (hToken)
			{
				CloseHandle(hToken);
			}
		});

		//AntiCheat::AcquireDebugPrivilege(hToken);

		DWORD dwSize = 0;
		PVOID pTokenInfo = nullptr;
		if (GetTokenInformation(hToken, TokenUser, nullptr, 0, &dwSize) || GetLastError() != ERROR_INSUFFICIENT_BUFFER) return GetLastError();

		if (dwSize)
		{
			pTokenInfo = allocator.allocate(dwSize);
			if (!pTokenInfo) return GetLastError();
		}

		if (!GetTokenInformation(hToken, TokenUser, pTokenInfo, dwSize, &dwSize) || !pTokenInfo) return GetLastError();

		PSID psidCurUser = reinterpret_cast<TOKEN_USER*>(pTokenInfo)->User.Sid;

		PSID psidEveryone = nullptr;
		SID_IDENTIFIER_AUTHORITY sidEveryone = SECURITY_WORLD_SID_AUTHORITY;
		if (!AllocateAndInitializeSid(&sidEveryone, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &psidEveryone) || !psidEveryone) return GetLastError();
		allocator.reference(psidEveryone, freeSid);

		PSID psidSystem = nullptr;
		SID_IDENTIFIER_AUTHORITY sidSystem = SECURITY_NT_AUTHORITY;
		if (!AllocateAndInitializeSid(&sidSystem, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, &psidSystem) || !psidSystem) return GetLastError();
		allocator.reference(psidSystem, freeSid);

		PSID psidAdmins = nullptr;
		SID_IDENTIFIER_AUTHORITY sidAdministrators = SECURITY_NT_AUTHORITY;
		if (!AllocateAndInitializeSid(&sidAdministrators, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &psidAdmins) || !psidAdmins) return GetLastError();
		allocator.reference(psidAdmins, freeSid);

		const PSID psidArray[] =
		{
			psidEveryone, /* Deny most rights to everyone */
			psidCurUser, /* Allow what was not denied */
			psidSystem, /* Full control */
			psidAdmins, /* Full control */
		};

		// Determine required size of the ACL
		dwSize = sizeof(ACL);

		// First the DENY, then the ALLOW
		dwSize += GetLengthSid(psidArray[0]);
		dwSize += sizeof(ACCESS_DENIED_ACE) - sizeof(DWORD);

		for (UINT i = 1; i < _countof(psidArray); ++i)
		{
			// DWORD is the SidStart field, which is not used for absolute format
			dwSize += GetLengthSid(psidArray[i]);
			dwSize += sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD);
		}

		PACL pDacl = reinterpret_cast<PACL>(allocator.allocate(dwSize));
		if (!pDacl || !InitializeAcl(pDacl, dwSize, ACL_REVISION)) return GetLastError();

		// Just give access to what steam needs
		//static const DWORD dwPoison = 0UL | ~(SYNCHRONIZE | GENERIC_EXECUTE | GENERIC_ALL);
		static const DWORD dwPoison =
			/*READ_CONTROL |*/ WRITE_DAC | WRITE_OWNER |
			PROCESS_CREATE_PROCESS | PROCESS_CREATE_THREAD |
			PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION |
			PROCESS_SET_QUOTA | PROCESS_SET_INFORMATION |
			PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE |
			// In addition to protected process
			PROCESS_SUSPEND_RESUME | PROCESS_TERMINATE;

		if (!AddAccessDeniedAce(pDacl, ACL_REVISION, dwPoison, psidArray[0])) return GetLastError();

		// Standard and specific rights not explicitly denied
		//static const DWORD dwAllowed = 0UL | SYNCHRONIZE;
		static const DWORD dwAllowed = (~dwPoison & 0x1FFF) | SYNCHRONIZE;
		if (!AddAccessAllowedAce(pDacl, ACL_REVISION, dwAllowed, psidArray[1])) return GetLastError();

		// Because of ACE ordering, System will effectively have dwAllowed even
		// though the ACE specifies PROCESS_ALL_ACCESS (unless software uses
		// SeDebugPrivilege or SeTcbName and increases access).
		// As an exercise, check behavior of tools such as Process Explorer under XP,
		// Vista, and above. Vista and above should exhibit slightly different behavior
		// due to Restricted tokens.
		if (!AddAccessAllowedAce(pDacl, ACL_REVISION, PROCESS_ALL_ACCESS, psidArray[2])) return GetLastError();

		// Because of ACE ordering, Administrators will effectively have dwAllowed
		// even though the ACE specifies PROCESS_ALL_ACCESS (unless the Administrator
		// invokes 'discretionary security' by taking ownership and increasing access).
		// As an exercise, check behavior of tools such as Process Explorer under XP,
		// Vista, and above. Vista and above should exhibit slightly different behavior
		// due to Restricted tokens.
		if (!AddAccessAllowedAce(pDacl, ACL_REVISION, PROCESS_ALL_ACCESS, psidArray[3])) return GetLastError();

		PSECURITY_DESCRIPTOR pSecDesc = allocator.allocate<SECURITY_DESCRIPTOR>();
		if (!pSecDesc) return GetLastError();

		// InitializeSecurityDescriptor initializes a security descriptor in
		// absolute format, rather than self-relative format. See
		// http://msdn.microsoft.com/en-us/library/aa378863(VS.85).aspx
		if (!InitializeSecurityDescriptor(pSecDesc, SECURITY_DESCRIPTOR_REVISION)) return GetLastError();
		if (!SetSecurityDescriptorDacl(pSecDesc, TRUE, pDacl, FALSE)) return GetLastError();

		__VMProtectEnd;

		return SetSecurityInfo(
			GetCurrentProcess(),
			SE_KERNEL_OBJECT, // process object
			OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
			psidCurUser, // NULL, // Owner SID
			nullptr, // Group SID
			pDacl,
			nullptr // SACL
		);
#else
		return 0;
#endif
	}

	void AntiCheat::AcquireDebugPrivilege(HANDLE hToken)
	{
		__VMProtectBeginUltra("");

		LUID luid;
		TOKEN_PRIVILEGES tp = { 0 };
		DWORD cb = sizeof(TOKEN_PRIVILEGES);
		if (!LookupPrivilegeValueW(nullptr, SE_DEBUG_NAME, &luid)) return;

		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = luid;
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		AdjustTokenPrivileges(hToken, FALSE, &tp, cb, nullptr, nullptr);
		//if (GetLastError() != ERROR_SUCCESS) return;

		__VMProtectEnd;
	}

	void AntiCheat::PatchVirtualProtect(void* vp, void* vpex)
	{
		__VMProtectBeginUltra("");
		AntiCheat::VirtualProtectHook[1].initialize(vpex, AntiCheat::VirtualProtectExStub, HOOK_JUMP)->install(true, true);
		AntiCheat::VirtualProtectHook[0].initialize(vp, AntiCheat::VirtualProtectStub, HOOK_JUMP)->install(true, true);
		__VMProtectEnd;
	}

	NTSTATUS NTAPI AntiCheat::NtCreateThreadExStub(PHANDLE phThread, ACCESS_MASK desiredAccess, LPVOID objectAttributes, HANDLE processHandle, LPTHREAD_START_ROUTINE startAddress, LPVOID parameter, BOOL createSuspended, DWORD stackZeroBits, DWORD sizeOfStackCommit, DWORD sizeOfStackReserve, LPVOID bytesBuffer)
	{
		__VMProtectBeginUltra("");

		HANDLE hThread = nullptr;
		std::lock_guard<std::mutex> _(AntiCheat::ThreadMutex);

		AntiCheat::CreateThreadHook.uninstall();
		NTSTATUS result = NtCreateThreadEx_t(AntiCheat::CreateThreadHook.getAddress())(&hThread, desiredAccess, objectAttributes, processHandle, startAddress, parameter, createSuspended, stackZeroBits, sizeOfStackCommit, sizeOfStackReserve, bytesBuffer);
		AntiCheat::CreateThreadHook.install();

		if (phThread) *phThread = hThread;

		if (GetProcessId(processHandle) == GetCurrentProcessId())
		{
			AntiCheat::OwnThreadIds.push_back(GetThreadId(hThread));
		}

		__VMProtectEnd;

		return result;
	}

	void AntiCheat::PatchThreadCreation()
	{
		__VMProtectBeginUltra("");

		HMODULE ntdll = Utils::GetNTDLL();
		if (ntdll)
		{
			static uint8_t ntCreateThreadEx[] = { 0xB1, 0x8B, 0xBC, 0x8D, 0x9A, 0x9E, 0x8B, 0x9A, 0xAB, 0x97, 0x8D, 0x9A, 0x9E, 0x9B, 0xBA, 0x87 }; // NtCreateThreadEx
			FARPROC createThread = GetProcAddress(ntdll, Utils::String::XOR(std::string(reinterpret_cast<char*>(ntCreateThreadEx), sizeof ntCreateThreadEx), -1).data());
			if (createThread)
			{
				AntiCheat::CreateThreadHook.initialize(createThread, AntiCheat::NtCreateThreadExStub, HOOK_JUMP)->install();
			}
		}

		__VMProtectEnd;
	}

	int AntiCheat::ValidateThreadTermination(void* addr)
	{
		__VMProtectBeginUltra("");
		{
			std::lock_guard<std::mutex> _(AntiCheat::ThreadMutex);

			DWORD id = GetCurrentThreadId();
			auto threadHook = AntiCheat::ThreadHookMap.find(id);
			if (threadHook != AntiCheat::ThreadHookMap.end())
			{
				threadHook->second->uninstall(false);
				AntiCheat::ThreadHookMap.erase(threadHook); // Uninstall and delete the hook
				return 1; // Kill
			}
		}

		while (true)
		{
			std::lock_guard<std::mutex> _(AntiCheat::ThreadMutex);

			// It would be better to wait for the thread
			// but we don't know if there are multiple hooks at the same address
			bool found = false;
			for (auto threadHook : AntiCheat::ThreadHookMap)
			{
				if (threadHook.second->getAddress() == addr)
				{
					found = true;
					break;
				}
			}

			if (!found) break;
			std::this_thread::sleep_for(10ms);
		}

		__VMProtectEnd;

		return 0; // Don't kill
	}

	__declspec(naked) void AntiCheat::ThreadEntryPointStub()
	{
		__asm
		{
			push eax
			push eax
			pushad

			// Reinitialize the return address
			mov eax, [esp + 28h]
			sub eax, 5
			mov [esp + 28h], eax

			push eax
			call AntiCheat::ValidateThreadTermination
			add esp, 4h

			mov [esp + 20h], eax

			popad

			pop eax

			test eax, eax
			jz dontKill

			pop eax
			add esp, 4h // Remove return address (simulate a jump hook)
			retn

		dontKill:
			pop eax
			retn
		}
	}

	void AntiCheat::VerifyThreadIntegrity()
	{
		__VMProtectBeginUltra("");
		bool kill = true;
		{
			std::lock_guard<std::mutex> _(AntiCheat::ThreadMutex);

			auto threadHook = std::find(AntiCheat::OwnThreadIds.begin(), AntiCheat::OwnThreadIds.end(), GetCurrentThreadId());
			if (threadHook != AntiCheat::OwnThreadIds.end())
			{
				AntiCheat::OwnThreadIds.erase(threadHook);
				kill = false;
			}
		}

		if (kill)
		{
			static bool first = true;
			if (first) first = false; // We can't control the main thread, as it's spawned externally
			else
			{
				std::lock_guard<std::mutex> _(AntiCheat::ThreadMutex);

				HMODULE ntdll = Utils::GetNTDLL(), targetModule;
				if (!ntdll) return; // :(

				void* address = Utils::GetThreadStartAddress(GetCurrentThread());
				if (address)
				{
					GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<char*>(address), &targetModule);
					if (targetModule == ntdll) return; // Better not kill kernel threads

					DWORD id = GetCurrentThreadId();
					{
						auto threadHook = AntiCheat::ThreadHookMap.find(id);
						if (threadHook != AntiCheat::ThreadHookMap.end())
						{
							threadHook->second->uninstall(false);
							AntiCheat::ThreadHookMap.erase(threadHook);
						}
					}

					std::shared_ptr<Utils::Hook> hook = std::make_shared<Utils::Hook>();
					AntiCheat::ThreadHookMap[id] = hook;

					// Hook the entry point of the thread to properly terminate it
					hook->initialize(address, AntiCheat::ThreadEntryPointStub, HOOK_CALL)->install(true, true);
				}
			}
		}
		__VMProtectEnd;
	}

    void AntiCheat::SystemTimeDiff(LPSYSTEMTIME stA, LPSYSTEMTIME stB, LPSYSTEMTIME stC) {
        FILETIME ftA, ftB, ftC;
        ULARGE_INTEGER uiA, uiB, uiC;

        SystemTimeToFileTime(stA, &ftA);
        SystemTimeToFileTime(stB, &ftB);
        uiA.HighPart = ftA.dwHighDateTime;
        uiA.LowPart = ftA.dwLowDateTime;
        uiB.HighPart = ftB.dwHighDateTime;
        uiB.LowPart = ftB.dwLowDateTime;

        uiC.QuadPart = uiA.QuadPart - uiB.QuadPart;

        ftC.dwHighDateTime = uiC.HighPart;
        ftC.dwLowDateTime = uiC.LowPart;
        FileTimeToSystemTime(&ftC, stC);
    }

    void AntiCheat::CheckStartupTime()
    {
        __VMProtectBeginUltra("");
        FILETIME creation, exit, kernel, user;
        SYSTEMTIME current, creationSt, diffSt;

        GetSystemTime(&current);
        GetProcessTimes(GetCurrentProcess(), &creation, &exit, &kernel, &user);

        FileTimeToSystemTime(&creation, &creationSt);
        AntiCheat::SystemTimeDiff(&current, &creationSt, &diffSt);

#ifdef DEBUG
        char buf[512];
        snprintf(buf, 512, "creation: %d:%d:%d:%d\n", creationSt.wHour, creationSt.wMinute, creationSt.wSecond, creationSt.wMilliseconds);
        OutputDebugStringA(buf);

        snprintf(buf, 512, "current: %d:%d:%d:%d\n", current.wHour, current.wMinute, current.wSecond, current.wMilliseconds);
        OutputDebugStringA(buf);

        snprintf(buf, 512, "diff: %d:%d:%d:%d\n", diffSt.wHour, diffSt.wMinute, diffSt.wSecond, diffSt.wMilliseconds);
        OutputDebugStringA(buf);
#endif

        // crash client if they are using process suspension to inject dlls during startup (aka before we got to here)
        // maybe tweak this value depending on what the above logging reveals during testing,
        // but 5 seconds seems about right for now
        int time = diffSt.wMilliseconds + (diffSt.wSecond * 1000) + (diffSt.wMinute * 1000 * 60);
        if (time > 5000) {
            Components::AntiCheat::CrashClient();
        }

        // use below for logging when using StartSuspended.exe
        // FILE* f = fopen("times.txt", "a");
        // fwrite(buf, 1, strlen(buf), f);
        // fclose(f);

        __VMProtectEnd;
    }

	AntiCheat::AntiCheat()
	{
		__VMProtectBeginUltra("");

		time(nullptr);
		AntiCheat::Flags = NO_FLAG;

#ifndef DISABLE_ANTICHEAT

		Utils::Hook(0x507BD5, AntiCheat::PatchWinAPI, HOOK_CALL).install()->quick();
		Utils::Hook(0x5082FD, AntiCheat::LostD3DStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x51C76C, AntiCheat::CinematicStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x418209, AntiCheat::SoundInitStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x60BE9D, AntiCheat::SoundInitStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x60BE8E, AntiCheat::SoundInitDriverStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x418204, AntiCheat::SoundInitDriverStub, HOOK_CALL).install()->quick();
		Scheduler::OnFrame(AntiCheat::PerformScan, true);

		// Detect aimbots
		Utils::Hook(0x426580, AntiCheat::DObjGetWorldTagPosStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x56AC60, AntiCheat::AimTargetGetTagPosStub, HOOK_JUMP).install()->quick();

		// TODO: Probably move that :P
		if (!Dedicated::IsEnabled())
		{
			AntiCheat::InitLoadLibHook();
		}

		// Prevent external processes from accessing our memory
		AntiCheat::ProtectProcess();
		Renderer::OnDeviceRecoveryEnd([]()
		{
			AntiCheat::ProtectProcess();
		});

		// Set the integrity flag
		AntiCheat::Flags |= AntiCheat::IntergrityFlag::INITIALIZATION;

#endif

		__VMProtectEnd;
	}

	AntiCheat::~AntiCheat()
	{
		AntiCheat::Flags = NO_FLAG;
		AntiCheat::OwnThreadIds.clear();
		AntiCheat::ThreadHookMap.clear();

		for (int i = 0; i < ARRAYSIZE(AntiCheat::LoadLibHook); ++i)
		{
			AntiCheat::LoadLibHook[i].uninstall();
		}

		for (int i = 0; i < ARRAYSIZE(AntiCheat::VirtualProtectHook); ++i)
		{
			AntiCheat::VirtualProtectHook[i].uninstall(false);
		}
	}
}
