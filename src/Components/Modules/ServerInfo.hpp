#pragma once

namespace Components
{
	class ServerInfo : public Component
	{
	public:
		ServerInfo();
		~ServerInfo();

		static Utils::InfoString GetHostInfo();
		static Utils::InfoString GetInfo();

	private:
		class Container
		{
		public:
			class Player
			{
			public:
				int ping;
				int score;
				std::string name;
			};

			unsigned int currentPlayer;
			std::vector<Player> playerList;
			Network::Address target;
		};

		static Container PlayerContainer;

		static void ServerStatus([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info);

		static unsigned int GetPlayerCount();
		static const char* GetPlayerText(unsigned int index, int column);
		static void SelectPlayer(unsigned int index);

		static void DrawScoreboardInfo(int localClientNum);
		static void DrawScoreboardStub();
	};
}
