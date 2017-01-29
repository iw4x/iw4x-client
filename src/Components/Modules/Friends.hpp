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
		static void UpdateHostname(Network::Address server, std::string hostname);

		static void SetPresence(std::string key, std::string value);
		static void ClearPresence(std::string key);

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
			Network::Address server;
			std::string serverName;
			bool online;
			bool playing;
			int experience;
			int prestige;
		};

		static unsigned int CurrentFriend;
		static std::recursive_mutex Mutex;
		static std::vector<Friend> FriendsList;

		static void DisconnectStub();
		static void ClearServer();
		static void SetServer();

		static bool IsClientInParty(int controller, int clientNum);

		static void UpdateUserInfo(SteamID user);
		static void UpdateState();

		static void SortList();
		static void SortIndividualList(std::vector<Friend>* list);

		static unsigned int GetFriendCount();
		static const char* GetFriendText(unsigned int index, int column);
		static void SelectFriend(unsigned int index);

		static void ParsePresence(std::vector<std::string> params, bool sort);

		static void FriendsResponse(std::vector<std::string> params);
		static void NameResponse(std::vector<std::string> params);
		static void PresenceResponse(std::vector<std::string> params);
		static void InfoResponse(std::vector<std::string> params);
	};
}
