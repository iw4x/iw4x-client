#include "STDInclude.hpp"

namespace Worker
{
	Runner::Runner(int pid) : processId(pid), terminate(false)
	{
		
	}

	Runner::~Runner()
	{
		
	}

	void Runner::run()
	{
		printf("Attaching to parent process %d...\n", this->processId);
		HANDLE processHandle = OpenProcess(SYNCHRONIZE, FALSE, this->processId);
		if (!processHandle || processHandle == INVALID_HANDLE_VALUE)
		{
			printf("Unable to attach to parent process\n");
		}
		else
		{
			printf("Successfully attached to parent process\n");
			printf("Starting worker...\n");

			std::thread workerThread(&Runner::worker, this);

			WaitForSingleObject(processHandle, INFINITE);
			CloseHandle(processHandle);

			printf("Awaiting worker termination...\n");
			this->terminate = true;
			if (workerThread.joinable()) workerThread.join();
			printf("Worker terminated\n");
		}
	}

	void Runner::worker()
	{
		printf("Worker started\n");
		Utils::IPC::BidirectionalChannel channel("IW4x-Worker-Channel", !Worker::IsWorker());

		while (!this->terminate)
		{
			Steam::Proxy::RunFrame();

			std::string buffer;
			if (channel.receive(&buffer))
			{
				printf("Data received: %s\n", buffer.data());
				channel.send("OK " + buffer);
			}

			std::this_thread::sleep_for(1ms);
		}

		printf("Terminating worker\n");
	}
}
