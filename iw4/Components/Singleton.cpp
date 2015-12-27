#include "..\STDInclude.hpp"

namespace Components
{
	bool Singleton::FirstInstance = true;

	bool Singleton::IsFirstInstance()
	{
		return Singleton::FirstInstance;
	}

	Singleton::Singleton()
	{
		if (Dedicated::IsDedicated()) return;

		Singleton::FirstInstance = !(CreateMutex(NULL, FALSE, "iw4x_mutex") && GetLastError() == ERROR_ALREADY_EXISTS);

		if (!Singleton::FirstInstance && MessageBoxA(0, "Do you want to start a second instance?", "Game already running", MB_ICONEXCLAMATION | MB_YESNO) == IDNO)
		{
			ExitProcess(0);
		}
	}
}
