#include <STDInclude.hpp>

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
			printf("%d\n", REVISION);
			ExitProcess(0);
		}

		Console::FreeNativeConsole();

		if (Loader::IsPerformingUnitTests() || Dedicated::IsEnabled() || ZoneBuilder::IsEnabled() || Monitor::IsEnabled()) return;

		Singleton::FirstInstance = (CreateMutexA(nullptr, FALSE, "iw4x_mutex") && GetLastError() != ERROR_ALREADY_EXISTS);

		if (!Singleton::FirstInstance && !ConnectProtocol::Used() && MessageBoxA(nullptr, "Do you want to start another instance?\nNot all features will be available!", "Game already running", MB_ICONEXCLAMATION | MB_YESNO) == IDNO)
		{
			ExitProcess(0);
		}
	}
}
