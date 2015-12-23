#include "STDInclude.hpp"

Utils::Hook EntryPointHook;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		FreeConsole();

		DWORD oldProtect;
		VirtualProtect(GetModuleHandle(NULL), 0x6C73000, PAGE_EXECUTE_READWRITE, &oldProtect);

		EntryPointHook.Initialize(0x6BAC0F, static_cast<void __declspec(naked)(*)()>([] ()
		{
			EntryPointHook.Uninstall();
			Components::Loader::Initialize();
			__asm jmp EntryPointHook.Place
		}))->Install();
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		Components::Loader::Uninitialize();
	}

	return TRUE;
}
