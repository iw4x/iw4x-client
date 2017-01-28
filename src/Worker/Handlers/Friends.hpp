#pragma once

namespace Handlers
{
	class Friends : public Worker::Runner::Handler
	{
	public:
		typedef std::function<void(Worker::Endpoint, std::vector<std::string>)> Callback;

		Friends();

		std::string getCommand() override { return "friends"; };
		void handle(Worker::Endpoint endpoint, std::string data) override;

	private:
		void addFunction(std::string function, Callback callback);
		std::unordered_map<std::string, Callback> functions;
	};
}
