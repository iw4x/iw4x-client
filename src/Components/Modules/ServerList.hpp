#pragma once

namespace Components
{
	class ServerList : public Component
	{
	public:
		typedef int(SortCallback)(const void*, const void*);

		struct ServerInfo
		{
			Network::Address addr;
			std::string hostname;
			std::string mapname;
			std::string gametype;
			std::string mod;
			std::string version;
			std::size_t hash;
			int clients;
			int bots;
			int maxClients;
			bool password;
			int ping;
			int matchType;
			int securityLevel;
			bool hardcore;
			bool svRunning;
			bool aimassist;
			bool voice;
		};

		ServerList();
		
		void preDestroy() override;

		static void Refresh([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info);
		static void RefreshVisibleList([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info);
		static void RefreshVisibleListInternal([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info, bool refresh = false);
		static void UpdateVisibleList([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info);
		static void InsertRequest(Network::Address address);
		static void Insert(const Network::Address& address, const Utils::InfoString& info);

		static ServerInfo* GetCurrentServer();

		static bool IsFavouriteList();
		static bool IsOfflineList();
		static bool IsOnlineList();

		static void Frame();
		static std::vector<ServerInfo>* GetList();

		static void UpdateVisibleInfo();

		static bool GetMasterServer(const char* ip, int port, Game::netadr_t& address);
		static bool UseMasterServer;

	private:
		enum class Column : int
		{
			Password,
			Matchtype,
			AimAssist,
			VoiceChat,
			Hostname,
			Mapname,
			Players,
			Gametype,
			Mod,
			Ping,

			Count
		};

		static constexpr auto* FavouriteFile = "players/favourites.json";

#pragma pack(push, 1)
		union MasterEntry
		{
			char token[7];
			struct
			{
				uint32_t ip;
				uint16_t port;
			};

			[[nodiscard]] bool IsEndToken() const noexcept
			{
				// End of transmission or file token
				return (token[0] == 'E' && token[1] == 'O' && (token[2] == 'T' || token[2] == 'F'));
			}

			[[nodiscard]] bool HasSeparator() const noexcept
			{
				return (token[6] == '\\');
			}
		};
#pragma pack(pop)

		class Container
		{
		public:
			class ServerContainer
			{
			public:
				bool sent;
				int sendTime;
				std::string challenge;
				Network::Address target;
			};

			bool awatingList;
			int awaitTime;

			int sentCount;
			int sendCount;

			Network::Address host;
			std::vector<ServerContainer> servers;
			std::recursive_mutex mutex;
		};

		static unsigned int GetServerCount();
		static const char* GetServerText(unsigned int index, int column);
		static const char* GetServerInfoText(ServerInfo* server, int column, bool sorting = false);
		static void SelectServer(unsigned int index);

		static void UpdateSource();
		static void UpdateGameType();

		static void SortList();

		static void LoadFavourties();
		static void StoreFavourite(const std::string& server);
		static void RemoveFavourite(const std::string& server);

		static ServerInfo* GetServer(unsigned int index);

		static bool CompareVersion(const std::string& version1, const std::string& version2);
		static bool IsServerDuplicate(const std::vector<ServerInfo>* list, const ServerInfo& server);

		static int SortKey;
		static bool SortAsc;

		static unsigned int CurrentServer;
		static Container RefreshContainer;

		static std::vector<ServerInfo> OnlineList;
		static std::vector<ServerInfo> OfflineList;
		static std::vector<ServerInfo> FavouriteList;

		static std::vector<unsigned int> VisibleList;

		static Dvar::Var UIServerSelected;
		static Dvar::Var UIServerSelectedMap;
		static Dvar::Var NETServerQueryLimit;
		static Dvar::Var NETServerFrames;

		static bool IsServerListOpen();
	};
}

template <>
struct std::hash<Components::ServerList::ServerInfo>
{
	std::size_t operator()(const Components::ServerList::ServerInfo& x) const noexcept
	{
		std::size_t hash = 0;

		hash ^= std::hash<std::string>()(x.hostname);
		hash ^= std::hash<std::string>()(x.mapname);
		hash ^= std::hash<std::string>()(x.mod);
		hash ^= std::hash<std::uint32_t>()(*reinterpret_cast<const std::uint32_t*>(&x.addr.getIP().bytes[0]));
		hash ^= x.clients;

		return hash;
	}
};
