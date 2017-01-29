#pragma once

namespace Worker
{
	class Endpoint
	{
	public:
		Endpoint() : Endpoint(nullptr) {}
		Endpoint(Utils::IPC::BidirectionalChannel* _channel) : channel(_channel) {}
		Endpoint(const Endpoint& obj) : Endpoint(obj.channel) {}

		void send(std::string message, std::string data)
		{
			if (this->channel)
			{
				Proto::IPC::Command command;
				command.set_name(message);
				command.set_data(data);

				this->channel->send(command.SerializeAsString());
			}
		}

	private:
		Utils::IPC::BidirectionalChannel* channel;
	};

	class Runner
	{
	public:
		class Handler
		{
		public:
			virtual ~Handler() {};
			virtual std::string getCommand() = 0;
			virtual void handle(Endpoint endpoint, std::string data) = 0;
		};

		Runner(int pid);
		~Runner();

		void run();

		void attachHandler(Runner::Handler* handler);

		static Utils::IPC::BidirectionalChannel* Channel;

	private:
		void worker();

		int processId;
		bool terminate;
		std::unordered_map<std::string, std::shared_ptr<Handler>> handlers;
	};
}

#include "Handlers/Friends.hpp"
#include "Handlers/SteamCallbacks.hpp"
