#pragma once

namespace Handlers
{
	class Friends : public Worker::Runner::Handler
	{
	public:
		typedef std::function<void(Worker::Endpoint, std::vector<std::string>)> Callback;

		Friends();
		~Friends();

		std::string getCommand() override { return "friends"; };
		void handle(Worker::Endpoint endpoint, std::string data) override;

	private:
		std::unordered_map<std::string, Callback> functions;
		void addFunction(std::string function, Callback callback);

		void getFriends(Worker::Endpoint endpoint, std::vector<std::string> params);
		void getName(Worker::Endpoint endpoint, std::vector<std::string> params);
		void setPresence(Worker::Endpoint endpoint, std::vector<std::string> params);
		void getPresence(Worker::Endpoint endpoint, std::vector<std::string> params);
		void requestPresence(Worker::Endpoint endpoint, std::vector<std::string> params);
	};
}
