#include "STDInclude.hpp"

namespace Components
{
	int AntiCheat::LastCheck = 0;
	std::string AntiCheat::Hash;
	Utils::Hook AntiCheat::LoadLibHook[4];
	unsigned long AntiCheat::Flags = NO_FLAG;

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

#if 0
	__declspec(naked) void AntiCheat::CrashClient()
	{
		static uint8_t crashProcedure[] =
		{
			// Variable space
			0xDC, 0xC1, 0xDC, 0x05, 

			// Uninstall minidump handler
			// This doesn't work anymore, due to the SetUnhandledExceptionFilter hook, but that's not important
			//0xB8, 0x63, 0xE7, 0x2F, 0x00,           // mov  eax, 2FE763h
			//0x05, 0xAD, 0xAD, 0x3C, 0x00,           // add  eax, 3CADADh
			//0x6A, 0x58,                             // push 88
			//0x8B, 0x80, 0xEA, 0x01, 0x00, 0x00,     // mov  eax, [eax + 1EAh]
			//0xFF, 0x10,                             // call dword ptr [eax]

			// Crash me.
			0xB8, 0x4F, 0x91, 0x27, 0x00,           // mov  eax, 27914Fh
			0x05, 0xDD, 0x28, 0x1A, 0x00,           // add  eax, 1A28DDh
			0x80, 0x00, 0x68,                       // add  byte ptr [eax], 68h
			0xC3,                                   // retn

			// Random stuff
			0xBE, 0xFF, 0xC2, 0xF4, 0x3A,
		};

		__asm
		{
			// This does absolutely nothing :P
			xor eax, eax
			mov ebx, [esp + 4h]
			shl ebx, 4h
			setz bl

			// Push the fake var onto the stack
			push ebx

			// Save the address to our crash procedure
			mov eax, offset crashProcedure
			push eax

			// Unprotect the .text segment
			push eax
			push 40h
			push 2D5FFFh
			push 401001h
			call VirtualProtect

			// Increment to our crash procedure
			// Skip variable space
			add dword ptr [esp], 4h

			// This basically removes the pushed ebx value from the stack, so returning results in a call to the procedure
			jmp AntiCheat::NullSub
		}
	}
#endif

	void AntiCheat::CrashClient()
	{
		Utils::Hook::Set<BYTE>(0x41BA2C, 0xEB);
	}

	void AntiCheat::AssertCalleeModule(void* callee)
	{
		HMODULE hModuleSelf = nullptr, hModuleTarget = nullptr, hModuleProcess = GetModuleHandleA(NULL);
		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<char*>(callee), &hModuleTarget);
		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<char*>(AntiCheat::AssertCalleeModule), &hModuleSelf);

		if (!hModuleSelf || !hModuleTarget || !hModuleProcess || (hModuleTarget != hModuleSelf && hModuleTarget != hModuleProcess))
		{
#ifdef DEBUG_DETECTIONS
			char buffer[MAX_PATH] = { 0 };
			GetModuleFileNameA(hModuleTarget, buffer, sizeof buffer);

			OutputDebugStringA(Utils::String::VA("AntiCheat: Callee assertion failed: %X %s", reinterpret_cast<uint32_t>(callee), buffer));
#endif

			//AntiCheat::CrashClient();
			AntiCheat::Hash.append("\0", 1);
		}
	}

	// This has to be called when doing .text changes during runtime
	__declspec(noinline) void AntiCheat::EmptyHash()
	{
		AntiCheat::LastCheck = 0;
		AntiCheat::Hash.clear();

		AntiCheat::AssertCalleeModule(_ReturnAddress());
	}

	void AntiCheat::InitLoadLibHook()
	{
		static uint8_t loadLibStub[] = { 0x33, 0xC0, 0xC2, 0x04, 0x00 }; // xor eax, eax; retn 04h
		static uint8_t loadLibExStub[] = { 0x33, 0xC0, 0xC2, 0x0C, 0x00 }; // xor eax, eax; retn 0Ch

		static uint8_t kernel32Str[] = { 0xB4, 0x9A, 0x8D, 0xB1, 0x9A, 0x93, 0xCC, 0xCD, 0xD1, 0x9B, 0x93, 0x93 }; // KerNel32.dll
		static uint8_t loadLibAStr[] = { 0xB3, 0x90, 0x9E, 0x9B, 0xB3, 0x96, 0x9D, 0x8D, 0x9E, 0x8D, 0x86, 0xBE }; // LoadLibraryA
		static uint8_t loadLibWStr[] = { 0xB3, 0x90, 0x9E, 0x9B, 0xB3, 0x96, 0x9D, 0x8D, 0x9E, 0x8D, 0x86, 0xA8 }; // LoadLibraryW

		HMODULE kernel32 = GetModuleHandleA(Utils::String::XOR(std::string(reinterpret_cast<char*>(kernel32Str), sizeof kernel32Str), -1).data());
		FARPROC loadLibA = GetProcAddress(kernel32, Utils::String::XOR(std::string(reinterpret_cast<char*>(loadLibAStr), sizeof loadLibAStr), -1).data());
		FARPROC loadLibW = GetProcAddress(kernel32, Utils::String::XOR(std::string(reinterpret_cast<char*>(loadLibWStr), sizeof loadLibWStr), -1).data());

#ifdef DEBUG_LOAD_LIBRARY
		AntiCheat::LoadLibHook[0].Initialize(loadLibA, LoadLibaryAStub, HOOK_JUMP);
		AntiCheat::LoadLibHook[1].Initialize(loadLibW, LoadLibaryWStub, HOOK_JUMP);
#else
		AntiCheat::LoadLibHook[0].Initialize(loadLibA, loadLibStub, HOOK_JUMP);
		AntiCheat::LoadLibHook[1].Initialize(loadLibW, loadLibStub, HOOK_JUMP);
#endif
		//AntiCheat::LoadLibHook[2].Initialize(LoadLibraryExA, loadLibExStub, HOOK_JUMP);
		//AntiCheat::LoadLibHook[3].Initialize(LoadLibraryExW, loadLibExStub, HOOK_JUMP);
	}

	void AntiCheat::ReadIntegrityCheck()
	{
		static int lastCheck = Game::Sys_Milliseconds();
		
		if ((Game::Sys_Milliseconds() - lastCheck) > 1000 * 70)
		{
			// TODO: Move that elsewhere
			if (HANDLE h = OpenProcess(PROCESS_VM_READ, TRUE, GetCurrentProcessId()))
			{
#ifdef DEBUG_DETECTIONS
				OutputDebugStringA("AntiCheat: Process integrity check failed");
#endif

				CloseHandle(h);
				AntiCheat::Hash.append("\0", 1);
			}
		}

		// Set the integrity flag
		AntiCheat::Flags |= AntiCheat::IntergrityFlag::READ_INTEGRITY_CHECK;
	}

	void AntiCheat::FlagIntegrityCheck()
	{
		static int lastCheck = Game::Sys_Milliseconds();

		if ((Game::Sys_Milliseconds() - lastCheck) > 1000 * 180)
		{
			lastCheck = Game::Sys_Milliseconds();

			unsigned long flags = ((AntiCheat::IntergrityFlag::MAX_FLAG - 1) << 1) - 1;

			if (AntiCheat::Flags != flags)
			{
#ifdef DEBUG_DETECTIONS
				OutputDebugStringA(Utils::String::VA("AntiCheat: Flag integrity check failed: %X", AntiCheat::Flags));
#endif

				AntiCheat::CrashClient();
			}
		}
	}

	void AntiCheat::ScanIntegrityCheck()
	{
		static int count = 0;
		int lastCheck = AntiCheat::LastCheck;
		int milliseconds = Game::Sys_Milliseconds();

		if (lastCheck) count = 0;
		else ++count;

		if (milliseconds < 1000 * 40) return;

		// If there was no check within the last 120 seconds, crash!
		if ((lastCheck && (milliseconds - lastCheck) > 1000 * 120) || count > 1)
		{
#ifdef DEBUG_DETECTIONS
			OutputDebugStringA("AntiCheat: Integrity check failed");
#endif

			AntiCheat::CrashClient();
		}

		// Set the integrity flag
		AntiCheat::Flags |= AntiCheat::IntergrityFlag::SCAN_INTEGRITY_CHECK;
	}

	void AntiCheat::PerformCheck()
	{
		// Hash .text segment
		// Add 1 to each value, so searching in memory doesn't reveal anything
		size_t textSize = 0x2D6001;
		uint8_t* textBase = reinterpret_cast<uint8_t*>(0x401001);
		std::string hash = Utils::Cryptography::SHA512::Compute(textBase - 1, textSize - 1, false);

		// Set the hash, if none is set
		if (AntiCheat::Hash.empty())
		{
			AntiCheat::Hash = hash;
		}
		// Crash if the hashes don't match
		else if (AntiCheat::Hash != hash)
		{
#ifdef DEBUG_DETECTIONS
			OutputDebugStringA("AntiCheat: Memory scan failed");
#endif

			AntiCheat::CrashClient();
		}

		// Set the memory scan flag
		AntiCheat::Flags |= AntiCheat::IntergrityFlag::MEMORY_SCAN;
	}

	void AntiCheat::Frame()
	{
		// Perform check only every 30 seconds
		if (AntiCheat::LastCheck && (Game::Sys_Milliseconds() - AntiCheat::LastCheck) < 1000 * 30) return;
		AntiCheat::LastCheck = Game::Sys_Milliseconds();

		AntiCheat::PerformCheck();
	}

#ifdef DEBUG_LOAD_LIBRARY
	HANDLE AntiCheat::LoadLibary(std::wstring library, void* callee)
	{
		HMODULE module;
		char buffer[MAX_PATH] = { 0 };

		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<char*>(callee), &module);
		GetModuleFileNameA(module, buffer, sizeof buffer);

		MessageBoxA(0, fmt::sprintf("Loading library %s via %s %X", std::string(library.begin(), library.end()).data(), buffer, reinterpret_cast<uint32_t>(callee)).data(), 0, 0);

		return LoadLibraryExW(library.data(), NULL, 0);
	}

	HANDLE WINAPI AntiCheat::LoadLibaryAStub(const char* library)
	{
		std::string lib(library);
		return AntiCheat::LoadLibary(std::wstring(lib.begin(), lib.end()), _ReturnAddress());
	}

	HANDLE WINAPI AntiCheat::LoadLibaryWStub(const wchar_t* library)
	{
		return AntiCheat::LoadLibary(library, _ReturnAddress());
	}
#endif

	void AntiCheat::UninstallLibHook()
	{
		for (int i = 0; i < ARRAYSIZE(AntiCheat::LoadLibHook); ++i)
		{
			AntiCheat::LoadLibHook[i].Uninstall();
		}
	}

	void AntiCheat::InstallLibHook()
	{
		AntiCheat::LoadLibHook[0].Install();
		AntiCheat::LoadLibHook[1].Install();
		//AntiCheat::LoadLibHook[2].Install();
		//AntiCheat::LoadLibHook[3].Install();
	}

	void AntiCheat::PatchWinAPI()
	{
		AntiCheat::UninstallLibHook();

		// Initialize directx :P
		Utils::Hook::Call<void()>(0x5078C0)();

		AntiCheat::InstallLibHook();
	}

	void AntiCheat::SoundInitStub(int a1, int a2, int a3)
	{
		AntiCheat::UninstallLibHook();

		Game::SND_Init(a1, a2, a3);

		AntiCheat::InstallLibHook();
	}

	void AntiCheat::SoundInitDriverStub()
	{
		AntiCheat::UninstallLibHook();

		Game::SND_InitDriver();

		AntiCheat::InstallLibHook();
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
			push[esp + 20h]

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

	// TODO: Beautify that
	DWORD AntiCheat::ProtectProcess()
	{
		// Returned to caller
		DWORD dwResult = (DWORD)-1;

		// Released on exit
		HANDLE hToken = NULL;
		PVOID pTokenInfo = NULL;

		PSID psidEveryone = NULL;
		PSID psidSystem = NULL;
		PSID psidAdmins = NULL;

		PACL pDacl = NULL;
		PSECURITY_DESCRIPTOR pSecDesc = NULL;

		__try
		{
			// Scratch
			DWORD dwSize = 0;
			BOOL bResult = FALSE;

			// If this fails, you can try to fallback to OpenThreadToken
			if (!OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &hToken)) 
			{
				dwResult = GetLastError();
				assert(FALSE);
				__leave; /*failed*/
			}

			bResult = GetTokenInformation(hToken, TokenUser, NULL, 0, &dwSize);
			dwResult = GetLastError();
			assert(bResult == FALSE && ERROR_INSUFFICIENT_BUFFER == dwResult);
			if (!(bResult == FALSE && ERROR_INSUFFICIENT_BUFFER == dwResult)) { __leave; /*failed*/ }

			if (dwSize) 
			{
				pTokenInfo = HeapAlloc(GetProcessHeap(), 0, dwSize);
				dwResult = GetLastError();
				assert(NULL != pTokenInfo);
				if (NULL == pTokenInfo) { __leave; /*failed*/ }
			}

			bResult = GetTokenInformation(hToken, TokenUser, pTokenInfo, dwSize, &dwSize);
			dwResult = GetLastError();
			assert(bResult && pTokenInfo);
			if (!(bResult && pTokenInfo)) { __leave; /*failed*/ }

			PSID psidCurUser = ((TOKEN_USER*)pTokenInfo)->User.Sid;

			SID_IDENTIFIER_AUTHORITY sidEveryone = SECURITY_WORLD_SID_AUTHORITY;
			bResult = AllocateAndInitializeSid(&sidEveryone, 1,
				SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &psidEveryone);
			dwResult = GetLastError();
			assert(bResult && psidEveryone);
			if (!(bResult && psidEveryone)) { __leave; /*failed*/ }

			SID_IDENTIFIER_AUTHORITY sidSystem = SECURITY_NT_AUTHORITY;
			bResult = AllocateAndInitializeSid(&sidSystem, 1,
				SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, &psidSystem);
			dwResult = GetLastError();

			assert(bResult && psidSystem);
			if (!(bResult && psidSystem)) { __leave; /*failed*/ }

			SID_IDENTIFIER_AUTHORITY sidAdministrators = SECURITY_NT_AUTHORITY;
			bResult = AllocateAndInitializeSid(&sidAdministrators, 2,
				SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
				0, 0, 0, 0, 0, 0, &psidAdmins);

			dwResult = GetLastError();
			assert(bResult && psidAdmins);
			if (!(bResult && psidAdmins)) { __leave; /*failed*/ }

			const PSID psidArray[] = 
			{
				psidEveryone,    /* Deny most rights to everyone */
				psidCurUser,    /* Allow what was not denied */
				psidSystem,        /* Full control */
				psidAdmins,        /* Full control */
			};

			// Determine required size of the ACL
			dwSize = sizeof(ACL);

			// First the DENY, then the ALLOW
			dwSize += GetLengthSid(psidArray[0]);
			dwSize += sizeof(ACCESS_DENIED_ACE) - sizeof(DWORD);

			for (UINT i = 1; i < _countof(psidArray); i++) 
			{
				// DWORD is the SidStart field, which is not used for absolute format
				dwSize += GetLengthSid(psidArray[i]);
				dwSize += sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD);
			}

			pDacl = (PACL)HeapAlloc(GetProcessHeap(), 0, dwSize);
			dwResult = GetLastError();
			assert(NULL != pDacl);
			if (NULL == pDacl) { __leave; /*failed*/ }

			bResult = InitializeAcl(pDacl, dwSize, ACL_REVISION);
			dwResult = GetLastError();
			assert(TRUE == bResult);
			if (FALSE == bResult) { __leave; /*failed*/ }

			// Mimic Protected Process
			// http://www.microsoft.com/whdc/system/vista/process_vista.mspx
			// Protected processes allow PROCESS_TERMINATE, which is
			// probably not appropriate for high integrity software.
			static const DWORD dwPoison =
				/*READ_CONTROL |*/ WRITE_DAC | WRITE_OWNER |
				PROCESS_CREATE_PROCESS | PROCESS_CREATE_THREAD |
				PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION |
				PROCESS_SET_QUOTA | PROCESS_SET_INFORMATION |
				PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE |
				// In addition to protected process
				PROCESS_SUSPEND_RESUME | PROCESS_TERMINATE;

			bResult = AddAccessDeniedAce(pDacl, ACL_REVISION, dwPoison, psidArray[0]);
			dwResult = GetLastError();
			assert(TRUE == bResult);
			if (FALSE == bResult) { __leave; /*failed*/ }

			// Standard and specific rights not explicitly denied
			static const DWORD dwAllowed = ~dwPoison & 0x1FFF;
			bResult = AddAccessAllowedAce(pDacl, ACL_REVISION, dwAllowed, psidArray[1]);
			dwResult = GetLastError();
			assert(TRUE == bResult);
			if (FALSE == bResult) { __leave; /*failed*/ }

			// Because of ACE ordering, System will effectively have dwAllowed even
			// though the ACE specifies PROCESS_ALL_ACCESS (unless software uses
			// SeDebugPrivilege or SeTcbName and increases access).
			// As an exercise, check behavior of tools such as Process Explorer under XP,
			// Vista, and above. Vista and above should exhibit slightly different behavior
			// due to Restricted tokens.
			bResult = AddAccessAllowedAce(pDacl, ACL_REVISION, PROCESS_ALL_ACCESS, psidArray[2]);
			dwResult = GetLastError();
			assert(TRUE == bResult);
			if (FALSE == bResult) { __leave; /*failed*/ }

			// Because of ACE ordering, Administrators will effectively have dwAllowed
			// even though the ACE specifies PROCESS_ALL_ACCESS (unless the Administrator
			// invokes 'discretionary security' by taking ownership and increasing access).
			// As an exercise, check behavior of tools such as Process Explorer under XP,
			// Vista, and above. Vista and above should exhibit slightly different behavior
			// due to Restricted tokens.
			bResult = AddAccessAllowedAce(pDacl, ACL_REVISION, PROCESS_ALL_ACCESS, psidArray[3]);
			dwResult = GetLastError();
			assert(TRUE == bResult);
			if (FALSE == bResult) { __leave; /*failed*/ }

			pSecDesc = (PSECURITY_DESCRIPTOR)HeapAlloc(GetProcessHeap(), 0, SECURITY_DESCRIPTOR_MIN_LENGTH);
			dwResult = GetLastError();
			assert(NULL != pSecDesc);
			if (NULL == pSecDesc) { __leave; /*failed*/ }

			// InitializeSecurityDescriptor initializes a security descriptor in
			// absolute format, rather than self-relative format. See
			// http://msdn.microsoft.com/en-us/library/aa378863(VS.85).aspx
			bResult = InitializeSecurityDescriptor(pSecDesc, SECURITY_DESCRIPTOR_REVISION);
			dwResult = GetLastError();
			assert(TRUE == bResult);
			if (FALSE == bResult) { __leave; /*failed*/ }

			bResult = SetSecurityDescriptorDacl(pSecDesc, TRUE, pDacl, FALSE);
			dwResult = GetLastError();
			assert(TRUE == bResult);
			if (FALSE == bResult) { __leave; /*failed*/ }

			dwResult = SetSecurityInfo(
				GetCurrentProcess(),
				SE_KERNEL_OBJECT, // process object
				OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
				psidCurUser, // NULL, // Owner SID
				NULL, // Group SID
				pDacl,
				NULL // SACL
			);

			dwResult = GetLastError();
			assert(ERROR_SUCCESS == dwResult);
			if (ERROR_SUCCESS != dwResult) { __leave; /*failed*/ }

			dwResult = ERROR_SUCCESS;
		}
		__finally
		{
			if (NULL != pSecDesc) 
			{
				HeapFree(GetProcessHeap(), 0, pSecDesc);
				pSecDesc = NULL;
			}
			if (NULL != pDacl) 
			{
				HeapFree(GetProcessHeap(), 0, pDacl);
				pDacl = NULL;
			}
			if (psidAdmins) 
			{
				FreeSid(psidAdmins);
				psidAdmins = NULL;
			}
			if (psidSystem) 
			{
				FreeSid(psidSystem);
				psidSystem = NULL;
			}
			if (psidEveryone) 
			{
				FreeSid(psidEveryone);
				psidEveryone = NULL;
			}
			if (NULL != pTokenInfo) 
			{
				HeapFree(GetProcessHeap(), 0, pTokenInfo);
				pTokenInfo = NULL;
			}
			if (NULL != hToken) 
			{
				CloseHandle(hToken);
				hToken = NULL;
			}
		}

		return dwResult;
	}

	AntiCheat::AntiCheat()
	{
		AntiCheat::EmptyHash();

#ifdef DEBUG
		Command::Add("penis", [] (Command::Params)
		{
			AntiCheat::CrashClient();
		});
#else

		Utils::Hook(0x507BD5, AntiCheat::PatchWinAPI, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x51C76C, AntiCheat::CinematicStub, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x418209, AntiCheat::SoundInitStub, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x60BE9D, AntiCheat::SoundInitStub, HOOK_CALL).Install()->Quick();
 		Utils::Hook(0x60BE8E, AntiCheat::SoundInitDriverStub, HOOK_CALL).Install()->Quick();
 		Utils::Hook(0x418204, AntiCheat::SoundInitDriverStub, HOOK_CALL).Install()->Quick();
		QuickPatch::OnFrame(AntiCheat::Frame);

		// Detect aimbots
		Utils::Hook(0x426580, AntiCheat::DObjGetWorldTagPosStub, HOOK_JUMP).Install()->Quick();
		Utils::Hook(0x56AC60, AntiCheat::AimTargetGetTagPosStub, HOOK_JUMP).Install()->Quick();

		// TODO: Probably move that :P
		AntiCheat::InitLoadLibHook();

		// Prevent external processes from accessing our memory
		AntiCheat::ProtectProcess();

		// Set the integrity flag
		AntiCheat::Flags |= AntiCheat::IntergrityFlag::INITIALIZATION;
#endif
	}

	AntiCheat::~AntiCheat()
	{
		AntiCheat::Flags = NO_FLAG;
		AntiCheat::EmptyHash();

		for (int i = 0; i < ARRAYSIZE(AntiCheat::LoadLibHook); ++i)
		{
			AntiCheat::LoadLibHook[i].Uninstall();
		}
	}
}
