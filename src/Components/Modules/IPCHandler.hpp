#pragma once

namespace Components
{
	class IPCHandler : public Component
	{
	public:
		class FunctionInterface
		{
		public:
			typedef std::function<void(std::vector<std::string>)> Callback;

			void map(std::string name, Callback function)
			{
				this->functions[name] = function;
			}

			void handle(std::string data)
			{
				Proto::IPC::Function function;
				if (function.ParseFromString(data))
				{
					auto handler = this->functions.find(function.name());
					if (handler != this->functions.end())
					{
						auto params = function.params();
						handler->second(std::vector<std::string>(params.begin(), params.end()));
					}
				}
			}

		private:
			std::unordered_map<std::string, Callback> functions;
		};

		typedef Utils::Slot<void(std::string)> Callback;

		IPCHandler();
		~IPCHandler();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "IPCHandler"; };
#endif

		static void SendWorker(std::string message, std::string data);
		static void SendClient(std::string message, std::string data);

		static void OnWorker(std::string message, Callback callback);
		static void OnClient(std::string message, Callback callback);

		static std::shared_ptr<FunctionInterface> NewInterface(std::string command);

	private:
		static std::unique_ptr<Utils::IPC::BidirectionalChannel> WorkerChannel;
		static std::unique_ptr<Utils::IPC::BidirectionalChannel> ClientChannel;

		static std::unordered_map<std::string, Callback> WorkerCallbacks;
		static std::unordered_map<std::string, Callback> ClientCallbacks;

		static std::unordered_map<std::string, std::shared_ptr<FunctionInterface>> FunctionInterfaces;
		
		static void InitChannels();
		static void StartWorker();

		static void HandleClient();
		static void HandleWorker();
	};
}
