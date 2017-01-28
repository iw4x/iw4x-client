#include "STDInclude.hpp"

namespace Components
{
	std::unordered_map<std::string, IPCHandler::Callback> IPCHandler::WorkerCallbacks;
	std::unordered_map<std::string, IPCHandler::Callback> IPCHandler::ClientCallbacks;

	std::unique_ptr<Utils::IPC::BidirectionalChannel> IPCHandler::WorkerChannel;
	std::unique_ptr<Utils::IPC::BidirectionalChannel> IPCHandler::ClientChannel;

	std::unordered_map<std::string, std::shared_ptr<IPCHandler::FunctionInterface>> IPCHandler::FunctionInterfaces;

	std::shared_ptr<IPCHandler::FunctionInterface> IPCHandler::NewInterface(std::string command)
	{
		std::shared_ptr<IPCHandler::FunctionInterface> fInterface(new IPCHandler::FunctionInterface());
		IPCHandler::FunctionInterfaces[command] = fInterface;
		return fInterface;
	}

	void IPCHandler::SendWorker(std::string message, std::string data)
	{
		IPCHandler::InitChannels();

		Proto::IPC::Command command;
		command.set_name(message);
		command.set_data(data);

		IPCHandler::WorkerChannel->send(command.SerializeAsString());
	}

	void IPCHandler::SendClient(std::string message, std::string data)
	{
		IPCHandler::InitChannels();

		Proto::IPC::Command command;
		command.set_name(message);
		command.set_data(data);

		IPCHandler::ClientChannel->send(command.SerializeAsString());
	}

	void IPCHandler::OnWorker(std::string message, IPCHandler::Callback callback)
	{
		IPCHandler::WorkerCallbacks[message] = callback;
	}

	void IPCHandler::OnClient(std::string message, IPCHandler::Callback callback)
	{
		IPCHandler::ClientCallbacks[message] = callback;
	}

	void IPCHandler::InitChannels()
	{
		if (!IPCHandler::WorkerChannel)
		{
			IPCHandler::WorkerChannel.reset(new Utils::IPC::BidirectionalChannel("IW4x-Worker-Channel", !Worker::IsWorker()));
		}

		if (!IPCHandler::ClientChannel)
		{
			IPCHandler::ClientChannel.reset(new Utils::IPC::BidirectionalChannel("IW4x-Client-Channel", Singleton::IsFirstInstance()));
		}
	}

	void IPCHandler::StartWorker()
	{
		STARTUPINFOA        sInfo;
		PROCESS_INFORMATION pInfo;

		ZeroMemory(&sInfo, sizeof(sInfo));
		ZeroMemory(&pInfo, sizeof(pInfo));
		sInfo.cb = sizeof(sInfo);

		CreateProcessA("iw4x.exe", const_cast<char*>(Utils::String::VA("-parent %d", GetCurrentProcessId())), nullptr, nullptr, false, NULL, nullptr, nullptr, &sInfo, &pInfo);

		if (pInfo.hThread && pInfo.hThread != INVALID_HANDLE_VALUE) CloseHandle(pInfo.hThread);
		if (pInfo.hProcess && pInfo.hProcess != INVALID_HANDLE_VALUE) CloseHandle(pInfo.hProcess);
	}

	void IPCHandler::HandleClient()
	{
		IPCHandler::InitChannels();

		std::string packet;
		if(IPCHandler::ClientChannel->receive(&packet))
		{
			Proto::IPC::Command command;
			if(command.ParseFromString(packet))
			{
				auto callback = IPCHandler::ClientCallbacks.find(command.name());
				if (callback != IPCHandler::ClientCallbacks.end())
				{
					callback->second(command.data());
				}
			}
		}
	}

	void IPCHandler::HandleWorker()
	{
		IPCHandler::InitChannels();

		std::string packet;
		if (IPCHandler::WorkerChannel->receive(&packet))
		{
			Proto::IPC::Command command;
			if (command.ParseFromString(packet))
			{
				auto callback = IPCHandler::WorkerCallbacks.find(command.name());
				auto fInterface = IPCHandler::FunctionInterfaces.find(command.name());
				if (callback != IPCHandler::WorkerCallbacks.end())
				{
					callback->second(command.data());
				}
				else if(fInterface != IPCHandler::FunctionInterfaces.end())
				{
					fInterface->second->handle(command.data());
				}
			}
		}
	}

	IPCHandler::IPCHandler()
	{
		if (Dedicated::IsEnabled()) return;

		IPCHandler::InitChannels();
		IPCHandler::StartWorker();

		QuickPatch::OnFrame([]()
		{
			IPCHandler::HandleWorker();
			IPCHandler::HandleClient();
		});
	}

	IPCHandler::~IPCHandler()
	{
		IPCHandler::FunctionInterfaces.clear();

		IPCHandler::WorkerCallbacks.clear();
		IPCHandler::ClientCallbacks.clear();

		IPCHandler::WorkerChannel.release();
		IPCHandler::ClientChannel.release();
	}
}
