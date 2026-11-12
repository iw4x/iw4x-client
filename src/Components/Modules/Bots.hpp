#pragma once

extern "C"
{
  void Bots_SV_BotUserMoveStub();

  bool Bots_BotAiAction(Game::client_s* cl);
}

namespace Components
{
  class Bots : public Component
  {
  public:
    Bots();

    static void SV_DirectConnect_Full_Check();

  private:
    friend bool (::Bots_BotAiAction)(Game::client_s* cl);

    using botData = std::pair<std::string, std::string>;

    static const Game::dvar_t* sv_randomBotNames;
    static const Game::dvar_t* sv_replaceBots;

    static std::size_t BotDataIndex;

    static std::vector<botData> RemoteBotNames;
    static bool AwaitingRemoteBotNames;

    static void UpdateBotNames();

    static std::vector<botData> LoadBotNames();
    static int BuildConnectString(char* buffer, const char* connectString, int num, int, int protocol, int checksum, int statVer, int statStuff, int port);

    static void Spawn(unsigned int count);

    static void GScr_isTestClient(Game::scr_entref_t entref);
    static bool BG_HasPerk(const unsigned int* perks, const unsigned int perkIndex);
    static void AddScriptMethods();

    static bool BotAiAction(Game::client_s* cl);

    static void G_SelectWeaponIndex(int clientNum, unsigned int iWeaponIndex);

    static void(*G_SelectWeaponIndex_Original)(int clientNum, unsigned int iWeaponIndex);
    static void G_SelectWeaponIndex_Hk(int clientNum, unsigned int iWeaponIndex);

    static bool Player_UpdateActivate_stub(int);

    static int SV_GetClientPing_Hk(int clientNum);

    static bool IsFull();

    static void CleanBotArray();

    static void AddServerCommands();
  };
}
