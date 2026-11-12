#pragma once

extern "C"
{
  void CardTitles_GetPlayerCardClientInfoStub();

  int CardTitles_GetPlayerCardClientInfo(int lookupResult, Game::PlayerCardData* data);
}

namespace Components
{
  class CardTitles : public Component
  {
  public:
    AssertOffset(Game::PlayerCardData, Game::PlayerCardData::name, 0x1C);

    static void SendCustomTitlesToClients();

    CardTitles();

  private:
    friend int (::CardTitles_GetPlayerCardClientInfo)(int lookupResult, Game::PlayerCardData* data);

    static Dvar::Var CustomTitle;
    static char CustomTitles[Game::MAX_CLIENTS][18];

    static int GetPlayerCardClientInfo(int lookupResult, Game::PlayerCardData* data);
    static const char* TableLookupByRowHook(const Game::StringTable* table, int row, int column);

    static void ParseCustomTitles(const char* msg);
  };
}
