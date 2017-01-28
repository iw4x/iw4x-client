#include "STDInclude.hpp"

namespace Handlers
{
	void Friends::handle(Worker::Endpoint endpoint, std::string data)
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

	void Friends::addFunction(std::string function, Friends::Callback callback)
	{
		this->functions[function] = callback;
	}

	Friends::Friends()
	{
		this->addFunction("getFriends", [&](Worker::Endpoint endpoint, std::vector<std::string> params)
		{
			if (params.size() >= 1 && Steam::Proxy::SteamFriends)
			{
				int flag = atoi(params[0].data());
				int count = Steam::Proxy::SteamFriends->GetFriendCount(flag);

				Proto::IPC::Function response;
				response.set_name("friendsResponse");

				for (int i = 0; i < count; ++i)
				{
					std::string* param = response.add_params();
					SteamID id = Steam::Proxy::SteamFriends->GetFriendByIndex(i, flag);

					param->clear();
					param->append(Utils::String::VA("%llX", id.Bits));
				}

				endpoint.send(this->getCommand(), response.SerializeAsString());
			}
		});
	}
}
