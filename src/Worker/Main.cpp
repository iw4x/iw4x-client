#include "STDInclude.hpp"
#include <conio.h>

void worker(bool* terminator)
{
	printf("Worker started\n");
	Utils::IPC::Channel channel("iw4xChannel");
	Utils::IPC::Channel channel2("iw4xChannel2");

	while(!*terminator)
	{
		std::string buffer;
		if(channel.receive(&buffer))
		{
			printf("Data received: %s\n", buffer.data());
			channel2.send("OK " + buffer);
		}

		std::this_thread::sleep_for(1ms);
	}

	printf("Terminating worker\n");
}

int main()
{
	bool terminator = false;
	char* command = "-parentProc ";
	char* parentProc = strstr(GetCommandLineA(), command);

	if (!parentProc)
	{
		printf("No parent process argument found\n");
		return 0;
	}

	parentProc += strlen(command);
	int pid = atoi(parentProc);

	printf("Attaching to process %d...\n", pid);

	HANDLE processHandle = OpenProcess(SYNCHRONIZE, FALSE, pid);
	if (!processHandle || processHandle == INVALID_HANDLE_VALUE)
	{
		printf("Unable to attach to parent process\n");
		return 0;
	}

	printf("Successfully attached to parent process\n");
	printf("Starting worker...\n");

	std::thread runner(worker, &terminator);

	WaitForSingleObject(processHandle, INFINITE);
	CloseHandle(processHandle);

	terminator = true;

	printf("Awaiting worker termination...\n");
	if (runner.joinable()) runner.join();
	printf("Worker terminated\n");

	//_getch();

	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
