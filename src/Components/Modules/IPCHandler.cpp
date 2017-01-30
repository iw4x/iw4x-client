#include "STDInclude.hpp"

namespace Components
{
	std::unordered_map<std::string, IPCHandler::Callback> IPCHandler::ClientCallbacks;
	std::unique_ptr<Utils::IPC::BidirectionalChannel> IPCHandler::ClientChannel;

	void IPCHandler::SendClient(std::string message, std::string data)
	{
		IPCHandler::InitChannel();

		Proto::IPC::Command command;
		command.set_name(message);
		command.set_data(data);

		IPCHandler::ClientChannel->send(command.SerializeAsString());
	}

	void IPCHandler::OnClient(std::string message, IPCHandler::Callback callback)
	{
		IPCHandler::ClientCallbacks[message] = callback;
	}

	void IPCHandler::InitChannel()
	{
		if (!IPCHandler::ClientChannel)
		{
			IPCHandler::ClientChannel.reset(new Utils::IPC::BidirectionalChannel("IW4x-Client-Channel", Singleton::IsFirstInstance()));
		}
	}

	void IPCHandler::HandleClient()
	{
		IPCHandler::InitChannel();

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

	IPCHandler::IPCHandler()
	{
		if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled() || Loader::PerformingUnitTests()) return;

		IPCHandler::InitChannel();

		QuickPatch::OnFrame(IPCHandler::HandleClient);
	}

	IPCHandler::~IPCHandler()
	{
		IPCHandler::ClientCallbacks.clear();
		IPCHandler::ClientChannel.release();
	}
}
