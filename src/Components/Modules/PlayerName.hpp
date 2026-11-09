#pragma once

extern "C"
{
  void PlayerName_ClientCleanName();
  void PlayerName_SV_UserinfoChangedStub();

  void PlayerName_UserInfoCopy(char* buffer, const char* name, int size);
  bool PlayerName_CopyClientNameCheck(char* dest, const char* source, int size);
  void PlayerName_DropClient(Game::client_s* drop);
  char* PlayerName_CleanStrStub(char* string);
}

namespace Components
{
  class PlayerName : public Component
  {
  public:
    PlayerName();

    static void UserInfoCopy(char* buffer, const char* name, int size);

    static int GetClientName(int localClientNum, int index, char* buf, int size);

  private:
    friend bool (::PlayerName_CopyClientNameCheck)(char* dest, const char* source, int size);
    friend void (::PlayerName_DropClient)(Game::client_s* drop);
    friend char* (::PlayerName_CleanStrStub)(char* string);

    static Dvar::Var sv_allowColoredNames;

    static char* CleanStrStub(char* string);

    static bool IsBadChar(int c);
    static bool CopyClientNameCheck(char* dest, const char* source, int size);
    static void DropClient(Game::client_s* drop);
  };
}
