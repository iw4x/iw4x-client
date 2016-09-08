#include "STDInclude.hpp"

namespace Components
{
	bool Singleton::FirstInstance = true;

	bool Singleton::IsFirstInstance()
	{
		return Singleton::FirstInstance;
	}

	Singleton::Singleton()
	{
		if (Flags::HasFlag("version"))
		{
			printf("IW4x " VERSION " (built " __DATE__ " " __TIME__ ")\n");
			ExitProcess(0);
		}

		Console::FreeNativeConsole();

		if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled()) return;

		Singleton::FirstInstance = (CreateMutexA(NULL, FALSE, "iw4x_mutex") && GetLastError() != ERROR_ALREADY_EXISTS);

		if (!Singleton::FirstInstance && !ConnectProtocol::Used() && MessageBoxA(0, "Do you want to start another instance?", "Game already running", MB_ICONEXCLAMATION | MB_YESNO) == IDNO)
		{
			ExitProcess(0);
		}
	}
}
