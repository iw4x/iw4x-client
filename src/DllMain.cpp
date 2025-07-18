namespace Main
{
	void Initialize()
	{
		std::srand(std::uint32_t(std::time(nullptr)) ^ ~(GetTickCount() * GetCurrentProcessId()));

		Utils::SetEnvironment();
		Steam::Proxy::RunMod();
		Utils::Cryptography::Initialize();
		Components::Loader::Initialize();
	}

	void Uninitialize()
	{
		Components::Loader::Uninitialize();
	}

 	int EntryPoint()
  {
		// /GS security cookie must be initialized before any exception-handling
		// constructs are registered in the current module.
		//
		Game::__security_init_cookie();

		// Perform IW4x-specific initialization before transferring control
    // to the original C runtime startup. See DllMain() for context.
    //
		Initialize();

		return Game::__tmainCRTStartup();
	}
}

BOOL APIENTRY DllMain(HINSTANCE /*hinstDLL*/, DWORD fdwReason, LPVOID /*lpvReserved*/)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		SetProcessDEPPolicy(PROCESS_DEP_ENABLE);

		// Under normal circumstances, a DLL is unloaded via FreeLibrary once its
		// reference count reaches zero. This is acceptable for auxiliary libraries
		// but unsuitable for modules like ours, which embed deeply into the host
		// process.
		//
		// To prevent FreeLibrary call on our module, whether accidental or
		// deliberate, we informs the Windows loader that our module is a permanent
		// part of the process.
		//
		HMODULE mod;
		if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN |
															 GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
													 reinterpret_cast<LPCTSTR>(DllMain), &mod) == 0)
		{
			MessageBoxA(nullptr,
									"Failed to mark iw4x module as permanent. For support, please "
									"visit https://iw4x.dev/install",
									"ERROR",
									MB_ICONERROR
			);
			return FALSE;
		}

#ifndef DISABLE_BINARY_CHECK
		const auto* binary = reinterpret_cast<const char*>(0x6F9358);
		if (!binary || std::memcmp(binary, BASEGAME_NAME, 14) != 0)
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

		Utils::Hook(0x6BAC0F, Main::EntryPoint, HOOK_JUMP).install()->quick();
	}
	else if (fdwReason == DLL_PROCESS_DETACH)
	{
		Main::Uninitialize();
	}

	return TRUE;
}
