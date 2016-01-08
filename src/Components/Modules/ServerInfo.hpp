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

			int CurrentPlayer;
			std::vector<Player> PlayerList;
			Network::Address Target;
		};

		static Container PlayerContainer;

		static void ServerStatus();

		static int GetPlayerCount();
		static const char* GetPlayerText(int index, int column);
		static void SelectPlayer(int index);
	};
}
