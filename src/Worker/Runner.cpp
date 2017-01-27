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

			this->terminate = true;
			printf("Awaiting worker termination...\n");
			if (workerThread.joinable()) workerThread.join();
			printf("Worker terminated\n");
		}
	}

	void Runner::worker()
	{
		printf("Worker started\n");
		Utils::IPC::BidirectionalChannel channel("iw4xChannel", !Worker::IsWorker());

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
