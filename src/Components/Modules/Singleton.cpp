#include "ConnectProtocol.hpp"
#include "Console.hpp"

#include <version.hpp>

namespace Components
{
	HANDLE Singleton::Mutex;

	bool Singleton::FirstInstance = true;

	bool Singleton::IsFirstInstance()
	{
		return FirstInstance;
	}

	void Singleton::preDestroy()
	{
		if (INVALID_HANDLE_VALUE != Mutex)
		{
			CloseHandle(Mutex);
		}
	}

	Singleton::Singleton()
	{
		if (Flags::HasFlag("version"))
		{
#ifdef EXPERIMENTAL_BUILD
			printf("%s", "IW4x " REVISION_STR "-develop (built " __DATE__ " " __TIME__ ")\n");
#else
			printf("%s", "IW4x " REVISION_STR " (built " __DATE__ " " __TIME__ ")\n");
#endif

			ExitProcess(EXIT_SUCCESS);
		}

		Console::FreeNativeConsole();

		if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled()) return;

		Mutex = CreateMutexA(nullptr, FALSE, "iw4x_mutex");
		FirstInstance = ((INVALID_HANDLE_VALUE != Mutex) && GetLastError() != ERROR_ALREADY_EXISTS);

		if (!FirstInstance && !ConnectProtocol::Used() && MessageBoxA(nullptr, "Do you want to start another instance?\nNot all features will be available!", "Game already running", MB_ICONEXCLAMATION | MB_YESNO) == IDNO)
		{
			ExitProcess(EXIT_SUCCESS);
		}
	}
}
