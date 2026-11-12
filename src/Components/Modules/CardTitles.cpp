#include "CardTitles.hpp"
#include "Events.hpp"
#include "ServerCommands.hpp"

namespace Components
{
  char CardTitles::CustomTitles[Game::MAX_CLIENTS][18];
  Dvar::Var CardTitles::CustomTitle;

  int CardTitles::GetPlayerCardClientInfo(int lookupResult, Game::PlayerCardData* data)
  {
    auto result = lookupResult;

    const auto* username = Dvar::Name.get<const char*>();
    if (std::strcmp(data->name, username) == 0)
    {
      result += 0xFE000000;
    }
    else
    {
      for (std::size_t i = 0; i < Game::MAX_CLIENTS; ++i)
      {
        if (!std::strcmp(data->name, Game::cgClientInfo[i].name))
        {
          // Since a 4 byte integer is overkill for a row num: We can use it to store the customprefix + clientNum and use a 2 byte integer for the row number
          result += 0xFF000000;
          result += static_cast<int>(i) * 0x10000;
          break;
        }
      }
    }

    return result;
  }

  const char* CardTitles::TableLookupByRowHook(const Game::StringTable* table, int row, int column)
  {
    std::uint8_t prefix = (row >> (8 * 3)) & 0xFF;
    std::uint8_t data = (row >> (8 * 2)) & 0xFF;

    // Note that the game looks assets up ignoring case, so the table name
    // may differ in case from the one we expect.
    //
    if (data < Game::MAX_CLIENTS && prefix != 0x00 && Game::I_stricmp(table->name, "mp/cardTitleTable.csv") == 0)
    {
      // Column 1 = CardTitle
      if (column == 1)
      {
        if (prefix == 0xFE)
        {
          if (!CustomTitle.get<std::string>().empty())
          {
            // 0xFF in front of the title to skip localization. Or else it will wait for a couple of seconds for the asset of type localize
            return Utils::String::VA("\x15%s", CustomTitle.get<const char*>());
          }
        }
        else if (prefix == 0xFF)
        {
          if (CustomTitles[data][0] != '\0')
          {
            return Utils::String::VA("\x15%s", CustomTitles[data]);
          }
        }
      }

      // If the title was changed it already returned at this point so...
      // Remove prefix and data to make being readable to the normal lookuprequest
      row &= 0xFFFF;
    }

    return Game::StringTable_GetColumnValueForRow(table, row, column);
  }

  void CardTitles::SendCustomTitlesToClients()
  {
    std::string list;

    for (std::size_t i = 0; i < Game::MAX_CLIENTS; ++i)
    {
      char playerTitle[18]{};

      if (Game::svs_clients[i].userinfo[0] != '\0')
      {
        strncpy_s(playerTitle, Game::Info_ValueForKey(Game::svs_clients[i].userinfo, "customTitle"), _TRUNCATE);
      }
      else
      {
        playerTitle[0] = '\0';
      }

      list.append(std::format("\\{}\\{}", i, playerTitle));
    }

    Game::SV_GameSendServerCommand(-1, Game::SV_CMD_CAN_IGNORE, Utils::String::Format("{:c} customTitles \"{}\"", 21, list));
  }

  void CardTitles::ParseCustomTitles(const char* msg)
  {
    for (std::size_t i = 0; i < Game::MAX_CLIENTS; ++i)
    {
      const auto index = std::to_string(i);
      const auto* playerTitle = Game::Info_ValueForKey(msg, index.data());

      if (playerTitle[0] == '\0')
      {
        CustomTitles[i][0] = '\0';
      }
      else
      {
        Game::I_strncpyz(CustomTitles[i], playerTitle, sizeof(CustomTitles[0]) / sizeof(char));
      }
    }
  }

  CardTitles::CardTitles()
  {
    Events::OnDvarInit([]
    {
      CustomTitle = Dvar::Register<const char*>("customTitle", "", Game::DVAR_USERINFO | Game::DVAR_ARCHIVE, "Custom card title");
    });

    std::memset(&CustomTitles, 0, sizeof(char[Game::MAX_CLIENTS][18]));

    ServerCommands::OnCommand(21, [](const Command::Params* params)
    {
      if (std::strcmp(params->get(1), "customTitles") == 0)
      {
        if (params->size() == 3)
        {
          ParseCustomTitles(params->get(2));
          return true;
        }
      }

      return false;
    });

    // Tag the card title row in the player card expression with whose card
    // it is.
    //
    Utils::Hook(0x14025006A, CardTitles_GetPlayerCardClientInfoStub, HOOK_JUMP).install()->quick();

    // Note that x64 inlines the tableLookupByRow expression into the
    // expression evaluator, leaving its string table lookup as the place to
    // substitute the title.
    //
    Utils::Hook(0x14025540F, TableLookupByRowHook, HOOK_CALL).install()->quick();
  }
}

int CardTitles_GetPlayerCardClientInfo(int lookupResult, Game::PlayerCardData* data)
{
  return Components::CardTitles::GetPlayerCardClientInfo(lookupResult, data);
}
