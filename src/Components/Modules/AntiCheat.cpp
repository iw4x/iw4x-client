#include "STDInclude.hpp"

namespace Components
{
	int AntiCheat::LastCheck;
	std::string AntiCheat::Hash;
	Utils::Hook AntiCheat::LoadLibHook[4];
	Utils::Hook AntiCheat::VirtualProtectHook;

	// This function does nothing, it only adds the two passed variables and returns the value
	// The only important thing it does is to clean the first parameter, and then return
	// By returning, the crash procedure will be called, as it hasn't been cleaned from the stack
	void __declspec(naked) AntiCheat::NullSub()
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

	void __declspec(naked) AntiCheat::CrashClient()
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

	// This has to be called when doing .text changes during runtime
	void AntiCheat::EmptyHash()
	{
		AntiCheat::LastCheck = 0;
		AntiCheat::Hash.clear();
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

		AntiCheat::LoadLibHook[0].Initialize(loadLibA, loadLibStub, HOOK_JUMP);
		AntiCheat::LoadLibHook[1].Initialize(loadLibW, loadLibStub, HOOK_JUMP);
		//AntiCheat::LoadLibHook[2].Initialize(LoadLibraryExA, loadLibExStub, HOOK_JUMP);
		//AntiCheat::LoadLibHook[3].Initialize(LoadLibraryExW, loadLibExStub, HOOK_JUMP);
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
			AntiCheat::CrashClient();
		}
	}

	void AntiCheat::Frame()
	{
		// Perform check only every 30 seconds
		if (AntiCheat::LastCheck && (Game::Sys_Milliseconds() - AntiCheat::LastCheck) < 1000 * 30) return;
		AntiCheat::LastCheck = Game::Sys_Milliseconds();

		AntiCheat::PerformCheck();
	}

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

	void AntiCheat::SoundInitStub()
	{
		AntiCheat::UninstallLibHook();

		Game::SND_InitDriver();

		AntiCheat::InstallLibHook();
	}

// 	BOOL WINAPI AntiCheat::VirtualProtectStub(LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect)
// 	{
// 		AntiCheat::VirtualProtectHook.Uninstall(false);
// 
// 		if (flNewProtect == PAGE_WRITECOPY || flNewProtect == PAGE_READWRITE || flNewProtect == PAGE_EXECUTE_READWRITE || flNewProtect == PAGE_WRITECOMBINE)
// 		{
// 			DWORD addr = (DWORD)lpAddress;
// 			DWORD start = 0x401000;
// 			DWORD end = start + 0x2D6000;
// 
// 			if (addr > start && addr < end)
// 			{
// 				OutputDebugStringA(Utils::VA("Write access to address %X", lpAddress));
// 			}
// 		}
// 
// 		BOOL retVal = VirtualProtect(lpAddress, dwSize, flNewProtect, lpflOldProtect);
// 		AntiCheat::VirtualProtectHook.Install(false);
// 		return retVal;
// 	}

	AntiCheat::AntiCheat()
	{
		// This is required for debugging...in release mode :P
		//AntiCheat::VirtualProtectHook.Initialize(VirtualProtect, VirtualProtectStub, HOOK_JUMP);
		//AntiCheat::VirtualProtectHook.Install(true, true);

		AntiCheat::EmptyHash();

#ifdef DEBUG
		Command::Add("penis", [] (Command::Params)
		{
			AntiCheat::CrashClient();
		});
#else
		Utils::Hook(0x60BE8E, AntiCheat::SoundInitStub, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x418204, AntiCheat::SoundInitStub, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x507BD5, AntiCheat::PatchWinAPI, HOOK_CALL).Install()->Quick();
		QuickPatch::OnFrame(AntiCheat::Frame);

		// TODO: Probably move that :P
		AntiCheat::InitLoadLibHook();
#endif
	}

	AntiCheat::~AntiCheat()
	{
		AntiCheat::EmptyHash();

		AntiCheat::VirtualProtectHook.Uninstall(false);
		for (int i = 0; i < ARRAYSIZE(AntiCheat::LoadLibHook); ++i)
		{
			AntiCheat::LoadLibHook[i].Uninstall();
		}
	}
}
