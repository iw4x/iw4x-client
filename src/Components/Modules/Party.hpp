namespace Components
{
	class Party : public Component
	{
	public:
		Party();
		~Party();
		const char* GetName() { return "Party"; };

		static Network::Address Target();
		static void Connect(Network::Address target);
		static const char* GetLobbyInfo(SteamID lobby, std::string key);
		static void RemoveLobby(SteamID lobby);

		static bool PlaylistAwaiting();
		static void PlaylistContinue();
		static void PlaylistError(std::string error);

	private:
		class JoinContainer
		{
		public:
			Network::Address Target;
			std::string Challenge;
			DWORD JoinTime;
			bool Valid;

			// Party-specific stuff
			DWORD RequestTime;
			bool AwaitingPlaylist;
		};

		static JoinContainer Container;
		static std::map<uint64_t, Network::Address> LobbyMap;

		static SteamID GenerateLobbyId();

		static Game::dvar_t* RegisterMinPlayers(const char* name, int value, int min, int max, Game::dvar_flag flag, const char* description);

		static void ConnectError(std::string message);

		static DWORD UIDvarIntStub(char* dvar);
	};
}
