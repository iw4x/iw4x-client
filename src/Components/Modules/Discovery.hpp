#pragma once

namespace Components
{
	class Discovery : public Component
	{
	public:
		Discovery();
		~Discovery();

		void preDestroy() override;

		static void Perform();

		static std::vector<Network::Address> GetLocalServers();

	private:
		static bool IsTerminating;
		static bool IsPerforming;
		static std::thread Thread;
		static std::string Challenge;

		static std::mutex Mutex;
		static std::vector<Network::Address> LocalServers;

		static void InsertServer(Network::Address server);
	};
}
