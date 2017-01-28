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

	void Friends::getFriends(Worker::Endpoint endpoint, std::vector<std::string> params)
	{
		if (params.size() >= 1 && Steam::Proxy::SteamFriends)
		{
			int flag = atoi(params[0].data());
			int count = Steam::Proxy::SteamFriends->GetFriendCount(flag);

			Proto::IPC::Function response;
			response.set_name("friendsResponse");

			for (int i = 0; i < count; ++i)
			{
				SteamID id = Steam::Proxy::SteamFriends->GetFriendByIndex(i, flag);
				*response.add_params() = Utils::String::VA("%llX", id.Bits);
			}

			endpoint.send(this->getCommand(), response.SerializeAsString());
		}
	}

	void Friends::getName(Worker::Endpoint endpoint, std::vector<std::string> params)
	{
		if(Steam::Proxy::SteamFriends)
		{
			std::string name;
			SteamID id;

			if(params.size() >= 1)
			{
				id.Bits = strtoull(params[0].data(), nullptr, 16);
				name = Steam::Proxy::SteamFriends->GetFriendPersonaName(id);
			}
			else
			{
				id.Bits = 0;
				name = Steam::Proxy::SteamFriends->GetPersonaName();
			}

			Proto::IPC::Function response;
			response.set_name("nameResponse");
			*response.add_params() = Utils::String::VA("%llX", id.Bits);
			*response.add_params() = name;

			endpoint.send(this->getCommand(), response.SerializeAsString());
		}
	}

	void Friends::setPresence(Worker::Endpoint /*endpoint*/, std::vector<std::string> params)
	{
		if (params.size() >= 2 && Steam::Proxy::SteamFriends)
		{
			Steam::Proxy::SteamFriends->SetRichPresence(params[0].data(), params[1].data());
		}
	}

	void Friends::getPresence(Worker::Endpoint endpoint, std::vector<std::string> params)
	{
		if (params.size() >= 2 && Steam::Proxy::SteamFriends)
		{
			SteamID id;
			id.Bits = strtoull(params[0].data(), nullptr, 16);

			Proto::IPC::Function response;
			response.set_name("presenceResponse");
			*response.add_params() = Utils::String::VA("%llX", id.Bits);
			*response.add_params() = Steam::Proxy::SteamFriends->GetFriendRichPresence(id, params[1].data());

			endpoint.send(this->getCommand(), response.SerializeAsString());
		}
	}

	void Friends::requestPresence(Worker::Endpoint /*endpoint*/, std::vector<std::string> params)
	{
		if (params.size() >= 1 && Steam::Proxy::SteamFriends)
		{
			SteamID id;
			id.Bits = strtoull(params[0].data(), nullptr, 16);

			Steam::Proxy::SteamFriends->RequestFriendRichPresence(id);
		}
	}

	Friends::Friends()
	{
		using namespace std::placeholders;
		this->addFunction("getFriends", std::bind(&Friends::getFriends, this, _1, _2));
		this->addFunction("getName", std::bind(&Friends::getName, this, _1, _2));
		this->addFunction("setPresence", std::bind(&Friends::setPresence, this, _1, _2));
		this->addFunction("getPresence", std::bind(&Friends::getPresence, this, _1, _2));
		this->addFunction("requestPresence", std::bind(&Friends::requestPresence, this, _1, _2));
	}

	Friends::~Friends()
	{
		if(Steam::Proxy::SteamFriends)
		{
			Steam::Proxy::SteamFriends->ClearRichPresence();
		}
	}
}
