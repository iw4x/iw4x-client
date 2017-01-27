#include "STDInclude.hpp"

namespace Components
{
	std::unordered_map<std::string, IPCHandler::Callback> IPCHandler::WorkerCallbacks;
	std::unordered_map<std::string, IPCHandler::Callback> IPCHandler::ClientCallbacks;

	std::unique_ptr<Utils::IPC::BidirectionalChannel> IPCHandler::WorkerChannel;
	std::unique_ptr<Utils::IPC::BidirectionalChannel> IPCHandler::ClientChannel;

	void IPCHandler::SendWorker(std::string message, std::string data)
	{
		IPCHandler::InitChannels();

	}

	void IPCHandler::SendClient(std::string message, std::string data)
	{
		IPCHandler::InitChannels();
		//IPCHandler::ClientChannel->send()
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

	IPCHandler::IPCHandler()
	{
		if (Dedicated::IsEnabled()) return;

		IPCHandler::InitChannels();
		IPCHandler::StartWorker();

		QuickPatch::OnFrame([]()
		{
			std::string buffer;
			if(IPCHandler::WorkerChannel->receive(&buffer))
			{
				Logger::Print("Data received: %s\n", buffer.data());
			}
		});
	}

	IPCHandler::~IPCHandler()
	{
		IPCHandler::WorkerCallbacks.clear();
		IPCHandler::ClientCallbacks.clear();
	}
}
