#pragma once

namespace Components
{
	class Bots : public Component
	{
	public:
		Bots();

		enum testClientKeyFlag
		{
			NUM_0 = 0x8,
			NUM_1 = 0x20,
			NUM_2 = 0x10000,
			NUM_3 = 0x20000,
			NUM_4 = 0x100000,
			NUM_5 = 0x200000,
			NUM_6 = 0x400000,
			NUM_7 = 0x800000,
			NUM_8 = 0x1000000,
			NUM_9 = 0x2000000
		};

	private:
		static std::vector<std::string> BotNames;

		static int BuildConnectString(char* buffer, const char* connectString, int num, int, int protocol, int checksum, int statVer, int statStuff, int port);

		static void Spawn(unsigned int count);

		static void AddMethods();

		static void BotAiAction(Game::client_t* cl);
		static void SV_UpdateBots_Hk();
	};
}
