namespace Components
{
	class ServerList : public Component
	{
	public:
		typedef int(SortCallback)(const void*, const void*);

		struct ServerInfo
		{
			Network::Address Addr;
			std::string Hostname;
			std::string Mapname;
			std::string Gametype;
			std::string Mod;
			std::string Shortversion;
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

#pragma pack(push, 1)
		union MasterEntry
		{
			char Token[7];
			struct
			{
				uint32_t IP;
				uint16_t Port;
			};

			bool IsEndToken()
			{
				// End of transmission or file token
				return (Token[0] == 'E' && Token[1] == 'O' && (Token[2] == 'T' || Token[2] == 'F'));
			}

			bool HasSeparator()
			{
				return (Token[6] == '\\');
			}
		};
#pragma pack(pop)

		struct Container
		{
			struct ServerContainer
			{
				bool Sent;
				int SendTime;
				std::string Challenge;
				Network::Address Target;
			};

			bool AwatingList;
			int AwaitTime;

			int SentCount;
			int SendCount;

			Network::Address Host;
			std::vector<ServerContainer> Servers;
			std::mutex Mutex;
		};

		static int GetServerCount();
		static const char* GetServerText(int index, int column);
		static const char* GetServerText(ServerInfo* server, int column);
		static void SelectServer(int index);

		static void Frame();

		static void SortListByKey(int key);

		static ServerInfo* GetServer(int index);

		static int SortKey;
		static bool SortAsc;

		static unsigned int CurrentServer;
		static Container RefreshContainer;
		static std::vector<ServerInfo> OnlineList;
		static std::vector<int> VisibleList;
	};
}
