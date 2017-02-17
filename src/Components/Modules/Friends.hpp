#pragma once

namespace Components
{
	class Friends : public Component
	{
	public:
		Friends();
		~Friends();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "Friends"; };
#endif

		static void UpdateFriends();
		static void UpdateRank();
		static void UpdateServer(Network::Address server, std::string hostname, std::string mapname);
		static void UpdateName();

		static void SetPresence(std::string key, std::string value);
		static void ClearPresence(std::string key);

		static void RequestPresence(SteamID user);
		static std::string GetPresence(SteamID user, std::string key);

		static void AddFriend(SteamID user);

	private:
#pragma pack(push, 4)
		struct FriendRichPresenceUpdate
		{
			SteamID m_steamIDFriend;	// friend who's rich presence has changed
			int32_t m_nAppID;			// the appID of the game (should always be the current game)
		};
#pragma pack(pop)

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

		static bool TriggerSort;
		static bool TriggerUpdate;
		static int InitialState;
		static unsigned int CurrentFriend;
		static std::recursive_mutex Mutex;
		static std::vector<Friend> FriendsList;

		static void DisconnectStub();
		static void ClearServer();
		static void SetServer();

		static bool IsClientInParty(int controller, int clientNum);

		static void UpdateUserInfo(SteamID user);
		static void UpdateState(bool force = false);

		static void SortList(bool force = false);
		static void SortIndividualList(std::vector<Friend>* list);

		static unsigned int GetFriendCount();
		static const char* GetFriendText(unsigned int index, int column);
		static void SelectFriend(unsigned int index);

		static void UpdateTimeStamp();

		static bool IsOnline(unsigned __int64 timeStamp);

		static void StoreFriendsList();
	};
}
