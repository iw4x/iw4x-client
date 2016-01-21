#include "STDInclude.hpp"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	static Utils::Hook EntryPointHook;

	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DWORD oldProtect;
		VirtualProtect(GetModuleHandle(NULL), 0x6C73000, PAGE_EXECUTE_READWRITE, &oldProtect);

		EntryPointHook.Initialize(0x6BAC0F, [] ()
		{
			EntryPointHook.Uninstall();
			Components::Loader::Initialize();
			__asm jmp EntryPointHook.Place

		})->Install();
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		Components::Loader::Uninitialize();
	}

	return TRUE;
}
