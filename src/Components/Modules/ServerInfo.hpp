namespace Components
{
	class ServerInfo : public Component
	{
	public:
		ServerInfo();
		~ServerInfo();
		const char* GetName() { return "ServerInfo"; };

	private:
		struct Container
		{
			struct Player
			{
				int Ping;
				int Score;
				std::string Name;
			};

			unsigned int CurrentPlayer;
			std::vector<Player> PlayerList;
			Network::Address Target;
		};

		static Container PlayerContainer;

		static void ServerStatus();

		static unsigned int GetPlayerCount();
		static const char* GetPlayerText(unsigned int index, int column);
		static void SelectPlayer(unsigned int index);

		static void DrawScoreboardInfo(void* a1);
		static void DrawScoreboardStub();
	};
}
