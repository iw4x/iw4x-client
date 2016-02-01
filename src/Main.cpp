#include "STDInclude.hpp"

namespace Main
{
	static Utils::Hook EntryPointHook;

	void Initialize()
	{
		Main::EntryPointHook.Uninstall();
		Components::Loader::Initialize();
	}

	void Uninitialize()
	{
		Components::Loader::Uninitialize();
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		// Ensure we're working with our desired binary
		if (Utils::Hook::Get<DWORD>(0x4C0FFF) != 0x6824748B)
		{
			return FALSE;
		}

		DWORD oldProtect;
		VirtualProtect(GetModuleHandle(NULL), 0x6C73000, PAGE_EXECUTE_READWRITE, &oldProtect);

		Main::EntryPointHook.Initialize(0x6BAC0F, [] ()
		{
			__asm
			{
				// This has to be called, otherwise the hook is not uninstalled and we're deadlocking
				call Main::Initialize

				mov eax, 6BAC0Fh
				jmp eax
			}

		})->Install();

// 		auto key = Utils::Cryptography::RSA::GenerateKey(2048);
// 		std::string message = "ZOB1234543253253452345";
// 		std::string signature = Utils::Cryptography::RSA::SignMessage(key, message);
// 
// 		// Invalidate the signature
// 		//signature[0] ^= 0xFF;
// 
// 		if (Utils::Cryptography::RSA::VerifyMessage(key, message, signature))
// 		{
// 			MessageBoxA(0, "Valid", 0, 0);
// 		}
// 		else
// 		{
// 			MessageBoxA(0, "Invalid!", 0, 0);
// 		}
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		Main::Uninitialize();
	}

	return TRUE;
}
