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
	if (fdwReason != DLL_PROCESS_ATTACH)
		return TRUE;

	SetProcessDEPPolicy(PROCESS_DEP_ENABLE);

#ifndef DISABLE_BINARY_CHECK
	const auto* binary = reinterpret_cast<const char*> (0x6F9358);
	if (!binary || std::memcmp (binary, BASEGAME_NAME, 14) != 0)
	{
		MessageBoxA(nullptr,
									"Failed to load game binary.\n"
									"You did not install the iw4x-rawfiles!\n"
									"For support, please visit https://iw4x.io/install",
									"ERROR",
									MB_ICONERROR
			);
		return FALSE;
	}
#endif

	Utils::Hook (0x6BAC0F, Main::EntryPoint, HOOK_JUMP).install ()->quick ();

	return TRUE;
}
