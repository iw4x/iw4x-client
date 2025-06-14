namespace Main
{
	void Initialize()
	{
		Utils::Cryptography::Initialize();
		Components::Loader::Initialize();
	}

	void Uninitialize()
	{
		Components::Loader::Uninitialize();
	}

	__declspec(naked) void EntryPoint()
	{
		__asm
		{
			pushad
			call Main::Initialize
			popad

			push 6BAA2Fh // Continue init routine
			push 6CA062h // __security_init_cookie
			retn
		}
	}
}

BOOL APIENTRY DllMain(HINSTANCE /*hinstDLL*/, DWORD fdwReason, LPVOID /*lpvReserved*/)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		SetProcessDEPPolicy(PROCESS_DEP_ENABLE);

		std::srand(std::uint32_t(std::time(nullptr)) ^ ~(GetTickCount() * GetCurrentProcessId()));

#ifndef DISABLE_BINARY_CHECK
		// Ensure we're working with our desired binary

#ifndef DEBUG_BINARY_CHECK
		const auto* binary = reinterpret_cast<const char*>(0x6F9358);
		if (!binary || std::memcmp(binary, BASEGAME_NAME, 14) != 0)
#endif
		{
			MessageBoxA(nullptr,
			            "Failed to load game binary.\n"
			            "You did not install the iw4x-rawfiles!\n"
			            "Please use the iw4x-launcher to run the game. For support, please visit https://iw4x.dev/install",
			            "ERROR",
			            MB_ICONERROR
			);
			return FALSE;
		}
#endif

		Utils::SetEnvironment();
		Steam::Proxy::RunMod();
		// Install entry point hook
		Utils::Hook(0x6BAC0F, Main::EntryPoint, HOOK_JUMP).install()->quick();
	}
	else if (fdwReason == DLL_PROCESS_DETACH)
	{
		Main::Uninitialize();
	}

	return TRUE;
}
