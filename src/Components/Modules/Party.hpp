namespace Components
{
	class Party : public Component
	{
	public:
		Party();
		~Party();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() { return "Party"; };
#endif

		static Network::Address Target();
		static void Connect(Network::Address target);
		static const char* GetLobbyInfo(SteamID lobby, std::string key);
		static void RemoveLobby(SteamID lobby);

		static bool PlaylistAwaiting();
		static void PlaylistContinue();
		static void PlaylistError(std::string error);

		static void ConnectError(std::string message);

	private:
		class JoinContainer
		{
		public:
			Network::Address target;
			std::string challenge;
			DWORD joinTime;
			bool valid;
			int matchType;

			Utils::InfoString info;

			// Party-specific stuff
			DWORD requestTime;
			bool awaitingPlaylist;		
		};

		static JoinContainer Container;
		static std::map<uint64_t, Network::Address> LobbyMap;

		static SteamID GenerateLobbyId();

		static Game::dvar_t* RegisterMinPlayers(const char* name, int value, int min, int max, Game::dvar_flag flag, const char* description);

		static DWORD UIDvarIntStub(char* dvar);
	};
}
