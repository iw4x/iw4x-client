#include "STDInclude.hpp"

namespace Main
{
	Utils::Hook EntryPointHook;

	void SetEnvironment()
	{
		wchar_t exeName[512];
		GetModuleFileName(GetModuleHandle(nullptr), exeName, sizeof(exeName) / 2);

		wchar_t* exeBaseName = wcsrchr(exeName, L'\\');
		exeBaseName[0] = L'\0';

		SetCurrentDirectory(exeName);
	}

	void Initialize()
	{
		Main::EntryPointHook.uninstall();

		Main::SetEnvironment();
		Utils::Cryptography::Initialize();

		if(Worker::IsWorker())
		{
			Worker::Initialize();
		}
		else
		{
			Components::Loader::Initialize();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
			if (Components::Loader::PerformingUnitTests())
			{
				DWORD result = (Components::Loader::PerformUnitTests() ? 0 : -1);
				Components::Loader::Uninitialize();
				ExitProcess(result);
			}
#else
			if (Components::Flags::HasFlag("tests"))
			{
				Components::Logger::Print("Unit tests are disabled outside the debug environment!\n");
			}
#endif
		}
	}

	void Uninitialize()
	{
		if(Worker::IsWorker())
		{
			Worker::Uninitialize();
		}
		else
		{
			Components::Loader::Uninitialize();
		}

		Utils::Cache::Uninitialize();
		google::protobuf::ShutdownProtobufLibrary();
	}
}

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD  ul_reason_for_call, LPVOID /*lpReserved*/)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		// Ensure we're working with our desired binary
		if (Utils::Hook::Get<DWORD>(0x4C0FFF) != 0x6824748B)
		{
			return FALSE;
		}

		DWORD oldProtect;
		std::uint8_t* module = reinterpret_cast<std::uint8_t*>(GetModuleHandle(nullptr));
		//VirtualProtect(module, 0x6C73000, PAGE_EXECUTE_READWRITE, &oldProtect); // Unprotect the entire process
		VirtualProtect(module + 0x1000, 0x2D6000, PAGE_EXECUTE_READ, &oldProtect); // Protect the .text segment

		Main::EntryPointHook.initialize(0x6BAC0F, [] ()
		{
			__asm
			{
				// This has to be called, otherwise the hook is not uninstalled and we're looping into infinity
				call Main::Initialize

				mov eax, 6BAC0Fh
				jmp eax
			}
		})->install();
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		Main::Uninitialize();
	}

	return TRUE;
}
