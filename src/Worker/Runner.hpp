#pragma once

namespace Worker
{
	class Runner
	{
	public:
		Runner(int pid);
		~Runner();

		void run();

	private:
		void worker();

		int processId;
		bool terminate;
	};
}
