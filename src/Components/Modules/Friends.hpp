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

	private:
		struct FriendRichPresenceUpdate
		{
			SteamID m_steamIDFriend;	// friend who's rich presence has changed
			int32_t m_nAppID;			// the appID of the game (should always be the current game)
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
			std::string name;
			Network::Address server;
			std::string statusName;
			bool online;
		};

		static unsigned int CurrentFriend;
		static std::recursive_mutex Mutex;
		static std::vector<Friend> FriendsList;

		static void UpdateUserInfo(SteamID user);

		static unsigned int GetFriendCount();
		static const char* GetFriendText(unsigned int index, int column);
		static void SelectFriend(unsigned int index);
	};
}
