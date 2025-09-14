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

BOOL APIENTRY DllMain(HINSTANCE /*hinstDLL*/, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		SetProcessDEPPolicy(PROCESS_DEP_ENABLE);

#ifndef DISABLE_BINARY_CHECK
		const auto* binary = reinterpret_cast<const char*>(0x6F9358);
		if (!binary || std::memcmp(binary, BASEGAME_NAME, 14) != 0)
		{
			MessageBoxA(nullptr,
			            "Failed to load game binary.\n"
			            "You did not install the iw4x-rawfiles!\n"
			            "Please use the iw4x to run the game. For support, please visit https://iw4x.io/install",
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
		// For `DLL_PROCESS_DETACH`, the `lpReserved` parameter is used to
		// determine the context:
		//
		//   - `lpReserved == nullptr` when `FreeLibrary()` is called.
		//   - `lpReserved != nullptr` when the process is being terminated.
		//
		// When `FreeLibrary()` is called, worker threads remain alive. That is,
		// runtime's state is consistent, and executing proper shutdown is
		// acceptable.
		//
		// When process is terminated, worker threads have either exited or been
		// forcefully terminated by the OS, leaving only the shutdown thread.
		// This situation leave runtime in an inconsistent state.
		//
		// Hence, proper cleanup should only be attempted when `FreeLibrary()`
		// is called. Otherwise, the process should rely on the OS to reclaim
		// resources.
		//
		if (lpvReserved != nullptr)
			return TRUE;

		Main::Uninitialize();
	}

	return TRUE;
}
