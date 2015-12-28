namespace Components
{
	class ServerList : public Component
	{
	public:
		struct ServerInfo
		{
			Network::Address Addr;
			bool Visible;
			std::string Hostname;
			std::string Mapname;
			std::string Gametype;
			std::string Mod;
			int Clients;
			int MaxClients;
			bool Password;
			int Ping;
			int MatchType;
			bool Hardcore;
		};

		ServerList();
		~ServerList();
		const char* GetName() { return "ServerList"; };

		static void Refresh();
		static void Insert(Network::Address address, Utils::InfoString info);

	private:
		enum Column
		{
			Password,
			Hostname,
			Mapname,
			Players,
			Gametype,
			Ping,
		};

		struct Container
		{
			struct ServerContainer
			{
				bool Sent;
				int SendTime;
				std::string Challenge;
				Network::Address Target;
			};

			bool AwaitingList;
			Network::Address Host;
			std::vector<ServerContainer> Servers;
			std::mutex Mutex;
		};

		static int GetServerCount();
		static const char* GetServerText(int index, int column);
		static void SelectServer(int index);

		static int CurrentServer;
		static Container RefreshContainer;
		static std::vector<ServerInfo> OnlineList;
	};
}
