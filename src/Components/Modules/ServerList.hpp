namespace Components
{
	class ServerList : public Component
	{
	public:
		typedef int(SortCallback)(const void*, const void*);

		class ServerInfo
		{
		public:
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
			int SecurityLevel;
			bool Hardcore;
			bool SVRunning;
		};

		ServerList();
		~ServerList();
		const char* GetName() { return "ServerList"; };

		static void Refresh();
		static void RefreshVisibleList();
		static void UpdateVisibleList();
		static void InsertRequest(Network::Address address, bool acquireMutex = true);
		static void Insert(Network::Address address, Utils::InfoString info);

		static ServerInfo* GetCurrentServer();

		static bool IsFavouriteList();
		static bool IsOfflineList();
		static bool IsOnlineList();

	private:
		enum Column
		{
			Password,
			Matchtype,
			Hostname,
			Mapname,
			Players,
			Gametype,
			Mod,
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

		class Container
		{
		public:
			class ServerContainer
			{
			public:
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

		static unsigned int GetServerCount();
		static const char* GetServerText(unsigned int index, int column);
		static const char* GetServerText(ServerInfo* server, int column);
		static void SelectServer(unsigned int index);

		static void UpdateSource();
		static void UpdateGameType();

		static void Frame();

		static void SortList();

		static void LoadFavourties();
		static void StoreFavourite(std::string server);

		static ServerInfo* GetServer(unsigned int index);
		static std::vector<ServerInfo>* GetList();

		static int SortKey;
		static bool SortAsc;

		static unsigned int CurrentServer;
		static Container RefreshContainer;

		static std::vector<ServerInfo> OnlineList;
		static std::vector<ServerInfo> OfflineList;
		static std::vector<ServerInfo> FavouriteList;

		static std::vector<unsigned int> VisibleList;
	};
}
