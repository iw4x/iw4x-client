#pragma once

namespace Components
{
	class Bots : public Component
	{
	public:
		Bots();
		~Bots();
		static unsigned int GetClientNum(Game::client_s*);
		static bool IsValidClientNum(unsigned int);

	private:
		static std::vector<std::string> BotNames;

		static void BuildConnectString(char* buffer, const char* connectString, int num, int, int protocol, int checksum, int statVer, int statStuff, int port);

		static void Spawn(unsigned int count);

		static void AddMethods();
	};
}
