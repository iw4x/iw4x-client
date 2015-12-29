#include "..\..\STDInclude.hpp"

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

		Singleton::FirstInstance = (CreateMutex(NULL, FALSE, "iw4x_mutex") && GetLastError() != ERROR_ALREADY_EXISTS);
		//Checking if the instance is for the connect protocol
		if (!Singleton::FirstInstance)
		{
			if (ConnectProtocol::InvokeConnect() == TRUE)
			{
				//Connect command was successfuly sent to the first instance, exiting the second game instance now.
				ExitProcess(0);
			}
			else
			{
				//No connect command was provided, continuing with normal processing.
				if (!Singleton::FirstInstance && MessageBoxA(0, "Do you want to start a second instance?", "Game already running", MB_ICONEXCLAMATION | MB_YESNO) == IDNO)
				{
					ExitProcess(0);
				}
			}
		}


	}
}
