#include "STDInclude.hpp"

namespace Worker
{
	int ProcessId;
	
	int __stdcall EntryPoint(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, char* /*lpCmdLine*/, int /*nCmdShow*/)
	{
		Runner runner(Worker::ProcessId);
		runner.run();
		return 0;
	}

	void Initialize()
	{
		if(!Steam::Proxy::Inititalize())
		{
			printf("Failed to initialize worker!\n");
			ExitProcess(1);
		}
		else
		{
#ifdef DEBUG
			SetConsoleTitleA("IW4x Worker");
#else
			FreeConsole();
#endif

			Utils::Hook(0x6BABA1, Worker::EntryPoint, HOOK_CALL).install()->quick();
		}
	}

	void Uninitialize()
	{
		Steam::Proxy::Uninititalize();
	}

	bool ParseWorkerFlag()
	{
		char* command = "-parent ";
		char* parentProc = strstr(GetCommandLineA(), command);

		if (parentProc)
		{
			parentProc += strlen(command);
			Worker::ProcessId = atoi(parentProc);

			return true;
		}

		return false;
	}

	bool IsWorker()
	{
		static Utils::Value<bool> flag;

		if (!flag.isValid())
		{
			flag.set(Worker::ParseWorkerFlag());
		}

		return flag.get();
	}
}
