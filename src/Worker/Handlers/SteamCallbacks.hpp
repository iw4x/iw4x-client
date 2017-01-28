#pragma once

namespace Handlers
{
	class SteamCallbacks : public Worker::Runner::Handler
	{
	public:
		typedef std::function<void(Worker::Endpoint, std::vector<std::string>)> Callback;

		SteamCallbacks();
		~SteamCallbacks();

		std::string getCommand() override { return "steamCallbacks"; };
		void handle(Worker::Endpoint endpoint, std::string data) override;

		static void HandleCallback(int32_t callId, void* data, size_t size);

	private:
		std::unordered_map<std::string, Callback> functions;
		void addFunction(std::string function, Callback callback);
	};
}
