#include "STDInclude.hpp"

namespace Worker
{
	Utils::IPC::BidirectionalChannel* Runner::Channel;

	Runner::Runner(int pid) : processId(pid), terminate(false)
	{
		Runner::Channel = nullptr;
	}

	Runner::~Runner()
	{
		Runner::Channel = nullptr;
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

	void Runner::attachHandler(Runner::Handler* handler)
	{
		this->handlers[handler->getCommand()] = std::shared_ptr<Runner::Handler>(handler);
	}

	void Runner::worker()
	{
		printf("Worker started\n");
		Utils::IPC::BidirectionalChannel channel("IW4x-Worker-Channel", !Worker::IsWorker());
		Runner::Channel = &channel;

		while (!this->terminate)
		{
			Steam::Proxy::RunFrame();

			std::string buffer;
			if (channel.receive(&buffer))
			{
				Proto::IPC::Command command;
				if(command.ParseFromString(buffer))
				{
					auto handler = this->handlers.find(command.name());
					if (handler != this->handlers.end())
					{
						printf("Dispatching command %s to handler\n", command.name().data());
						handler->second->handle(&channel, command.data());
					}
					else
					{
						printf("No handler found for command %s\n", command.name().data());
					}
				}
			}

			std::this_thread::sleep_for(1ms);
		}

		printf("Terminating worker\n");
		Runner::Channel = nullptr;
	}
}
