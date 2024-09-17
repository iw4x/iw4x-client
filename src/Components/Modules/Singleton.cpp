#include <STDInclude.hpp>
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
			printf("%s", "IW4x " VERSION " (built " __DATE__ " " __TIME__ ")\n");
#ifdef EXPERIMENTAL_BUILD
			printf("Revision: %i - develop\n", REVISION);
#else
			printf("Revision: %i\n", REVISION);
#endif

			ExitProcess(EXIT_SUCCESS);
		}

		Console::FreeNativeConsole();

		if (Loader::IsPerformingUnitTests() || Dedicated::IsEnabled() || ZoneBuilder::IsEnabled()) return;

		Mutex = CreateMutexA(nullptr, FALSE, "iw4x_mutex");
		FirstInstance = ((INVALID_HANDLE_VALUE != Mutex) && GetLastError() != ERROR_ALREADY_EXISTS);

		if (!FirstInstance && !ConnectProtocol::Used() && MessageBoxA(nullptr, "Do you want to start another instance?\nNot all features will be available!", "Game already running", MB_ICONEXCLAMATION | MB_YESNO) == IDNO)
		{
			ExitProcess(EXIT_SUCCESS);
		}
	}
}
