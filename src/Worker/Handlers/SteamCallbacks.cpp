#include "STDInclude.hpp"

namespace Handlers
{
	void SteamCallbacks::handle(Worker::Endpoint endpoint, std::string data)
	{
		Proto::IPC::Function function;
		if (function.ParseFromString(data))
		{
			auto handler = this->functions.find(function.name());
			if (handler != this->functions.end())
			{
				printf("Handing function %s\n", function.name().data());

				auto params = function.params();
				handler->second(endpoint, std::vector<std::string>(params.begin(), params.end()));
			}
			else
			{
				printf("No handler for function %s\n", function.name().data());
			}
		}
	}

	void SteamCallbacks::addFunction(std::string function, Friends::Callback callback)
	{
		this->functions[function] = callback;
	}

	void SteamCallbacks::HandleCallback(int32_t callId, void* data, size_t size)
	{
		if(Worker::Runner::Channel)
		{
			Proto::IPC::Function response;
			response.set_name(Utils::String::VA("%d", callId));
			response.add_params()->append(static_cast<char*>(data), size);

			Proto::IPC::Command command;
			command.set_name(SteamCallbacks().getCommand());
			command.set_data(response.SerializeAsString());

			Worker::Runner::Channel->send(command.SerializeAsString());
		}
	}

	SteamCallbacks::SteamCallbacks()
	{

	}

	SteamCallbacks::~SteamCallbacks()
	{

	}
}
