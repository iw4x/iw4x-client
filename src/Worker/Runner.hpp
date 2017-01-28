#pragma once

namespace Worker
{
	class Runner
	{
	public:
		class Handler
		{
		public:
			virtual ~Handler() {};
			virtual std::string getCommand() = 0;
			virtual void handle(Utils::IPC::BidirectionalChannel* channel, std::string data) = 0;
		};

		Runner(int pid);
		~Runner();

		void run();

		void attachHandler(std::shared_ptr<Handler> handler);

	private:
		void worker();

		int processId;
		bool terminate;
		std::map<std::string, std::shared_ptr<Handler>> handlers;
	};
}
