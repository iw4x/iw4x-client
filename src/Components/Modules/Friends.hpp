#pragma once

namespace Components
{
	class Friends : public Component
	{
	public:
		Friends();
		~Friends();

		static void UpdateFriends();
		static void UpdateRank();
		static void UpdateServer(Network::Address server, const std::string& hostname, const std::string& mapname);
		static void UpdateName();

		static void SetPresence(const std::string& key, const std::string& value);
		static void ClearPresence(const std::string& key);

		static void RequestPresence(SteamID user);
		static std::string GetPresence(SteamID user, const std::string& key);

		static int GetGame(SteamID user);

		static bool IsInvisible();

		static Dvar::Var UIStreamFriendly;
		static Dvar::Var CLAnonymous;
		static Dvar::Var CLNotifyFriendState;

	private:
#pragma pack(push, 4)
		struct FriendRichPresenceUpdate
		{
			SteamID m_steamIDFriend;	// friend who's rich presence has changed
			int32_t m_nAppID;			// the appID of the game (should always be the current game)
		};
#pragma pack(pop)

		struct AvatarImageLoaded
		{
			SteamID m_steamID; // steamid the avatar has been loaded for
			int m_iImage; // the image index of the now loaded image
			int m_iWide; // width of the loaded image
			int m_iTall; // height of the loaded image
		};

		struct PersonaStateChange
		{
			SteamID m_ulSteamID;	// steamID of the friend who changed
			int m_nChangeFlags;		// what's changed
		};

		class Friend
		{
		public:
			SteamID userId;
			SteamID guid;
			std::string name;
			std::string playerName;
			std::string cleanName;
			Network::Address server;
			std::string serverName;
			std::string mapname;
			bool online;
			unsigned int lastTime;
			int experience;
			int prestige;
		};

		static bool LoggedOn;
		static bool TriggerSort;
		static bool TriggerUpdate;
		static int InitialState;
		static unsigned int CurrentFriend;
		static std::recursive_mutex Mutex;
		static std::vector<Friend> FriendsList;

		static void ClearServer();
		static void SetServer();

		static bool IsClientInParty(int controller, int clientNum);

		static void UpdateUserInfo(SteamID user);
		static void UpdateState();

		static void SortList(bool force = false);
		static void SortIndividualList(std::vector<Friend>* list);

		static unsigned int GetFriendCount();
		static const char* GetFriendText(unsigned int index, int column);
		static void SelectFriend(unsigned int index);

		static void UpdateTimeStamp();

		static bool IsOnline(unsigned __int64 timeStamp);

		static void StoreFriendsList();

		static void SetRawPresence(const char* key, const char* value);
		static std::vector<int> GetAppIdList();

		static Game::Material* CreateAvatar(SteamID user);
	};
}
