#pragma once

extern "C"
{
  void Bots_SV_BotUserMove_Hk();
  void Bots_G_SelectWeaponIndex_Hk();

  void Bots_BotAiAction(Game::client_s* cl);
  void Bots_G_SelectWeaponIndex(int clientNum, unsigned int iWeaponIndex);
}

namespace Components
{
  class Bots : public Component
  {
  public:
    Bots();

    static void SV_DirectConnect_Full_Check();

  private:
    friend void (::Bots_BotAiAction)(Game::client_s* cl);
    friend void (::Bots_G_SelectWeaponIndex)(int clientNum, unsigned int iWeaponIndex);

    using botData = std::pair<std::string, std::string>;

    static Game::dvar_t** aim_automelee_range;
    static Game::dvar_t** perk_extendedMeleeRange;

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

    static void BotAiAction(Game::client_s* cl);

    static void G_SelectWeaponIndex(int clientNum, unsigned int iWeaponIndex);

    static bool Player_UpdateActivate_stub(int);

    static int SV_GetClientPing_Hk(int clientNum);

    static bool IsFull();

    static void CleanBotArray();

    static void AddServerCommands();
  };
}
