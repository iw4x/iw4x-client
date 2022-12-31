#pragma once

namespace Components
{
	class Bots : public Component
	{
	public:
		Bots();

	private:
		using botData = std::pair< std::string, std::string>;
		static std::vector<botData> BotNames;

		static Dvar::Var SVRandomBotNames;

		static int BuildConnectString(char* buffer, const char* connectString, int num, int, int protocol, int checksum, int statVer, int statStuff, int port);

		static void Spawn(unsigned int count);

		static void GScr_isTestClient(Game::scr_entref_t entref);
		static void AddMethods();

		static void BotAiAction(Game::client_t* cl);
		static void SV_BotUserMove_Hk();

		static void G_SelectWeaponIndex(int clientNum, int iWeaponIndex);
		static void G_SelectWeaponIndex_Hk();
	};
}
