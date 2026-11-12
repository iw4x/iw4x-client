#include "ClanTags.hpp"
#include "Events.hpp"
#include "PlayerName.hpp"
#include "ServerCommands.hpp"

namespace Components
{
  const Game::dvar_t* ClanTags::ClanName;
  void (*ClanTags::PlayerCards_SetCachedPlayerData_Original)(int, int, int);

  // bgs_t and clientState_s do not have this
  char ClanTags::ClientState[Game::MAX_CLIENTS][MAX_CLAN_NAME_LENGTH];

  const char* ClanTags::GetClanTagWithName(int clientNum, const char* playerName)
  {
    AssertIn(clientNum, Game::MAX_CLIENTS);

    if (ClientState[clientNum][0] == '\0')
    {
      return playerName;
    }

    return Utils::String::VA("[%s^7]%s", ClientState[clientNum], playerName);
  }

  void ClanTags::SendClanTagsToClients()
  {
    std::string list;

    for (std::size_t i = 0; i < Game::MAX_CLIENTS; ++i)
    {
      list.append(std::format("\\{}\\{}", i, ClientState[i]));
    }

    Game::SV_GameSendServerCommand(-1, Game::SV_CMD_CAN_IGNORE, Utils::String::Format("{:c} clanNames \"{}\"", 22, list));
  }

  void ClanTags::ParseClanTags(const char* infoString)
  {
    for (std::size_t i = 0; i < Game::MAX_CLIENTS; ++i)
    {
      const auto index = std::to_string(i);
      const auto* clanTag = Game::Info_ValueForKey(infoString, index.data());

      if (clanTag[0] == '\0')
      {
        ClientState[i][0] = '\0';
      }
      else
      {
        Game::I_strncpyz(ClientState[i], clanTag, sizeof(ClientState[0]) / sizeof(char));
      }
    }
  }

  int ClanTags::CL_FilterChar(unsigned char input)
  {
    if (input == '^')
    {
      return ' ';
    }
    if (input < ' ')
    {
      return -1;
    }

    if (input == 188 || input == 189)
    {
      return -1;
    }

    return input;
  }

  void ClanTags::CL_SanitizeClanName()
  {
    char saneNameBuf[MAX_CLAN_NAME_LENGTH]{};
    auto* saneName = saneNameBuf;

    assert(ClanName);
    const auto* currentName = ClanName->current.string;
    if (currentName)
    {
      auto nameLen = std::strlen(currentName);
      for (std::size_t i = 0; (i < nameLen) && (i < sizeof(saneNameBuf)); ++i)
      {
        auto curChar = CL_FilterChar(static_cast<unsigned char>(currentName[i]));
        if (curChar > 0)
        {
          *saneName++ = curChar & 0xFF;
        }
      }

      saneNameBuf[sizeof(saneNameBuf) - 1] = '\0';
      Game::Dvar_SetString(ClanName, saneNameBuf);
    }
  }

  char* ClanTags::GamerProfile_GetClanName(int controllerIndex)
  {
    AssertIn(controllerIndex, Game::MAX_LOCAL_CLIENTS);
    assert(ClanName);

    CL_SanitizeClanName();
    Game::I_strncpyz(Game::gamerSettings[controllerIndex].exeConfig.clanPrefix, ClanName->current.string, sizeof(Game::GamerSettingExeConfig::clanPrefix));

    return Game::gamerSettings[controllerIndex].exeConfig.clanPrefix;
  }

  void ClanTags::Dvar_InfoString_Stub(char* s, const char* key, const char* value)
  {
    Utils::Hook::Call<void(char*, const char*, const char*)>(0x14028AB00)(s, key, value); // Info_SetValueForKey

    // Set 'clanAbbrev' in the info string
    Utils::Hook::Call<void(char*, const char*, const char*)>(0x14028AB00)(s, "clanAbbrev", GamerProfile_GetClanName(0)); // Info_SetValueForKey
  }

  void ClanTags::ClientUserinfoChanged(const char* s, int clientNum)
  {
    AssertIn(clientNum, Game::MAX_CLIENTS);

    const auto* clanAbbrev = Game::Info_ValueForKey(s, "clanAbbrev");

    if (clanAbbrev[0] == '\0')
    {
      ClientState[clientNum][0] = '\0';
    }
    else
    {
      Game::I_strncpyz(ClientState[clientNum], clanAbbrev, sizeof(ClientState[0]) / sizeof(char));
    }
  }

  // s1 is always an empty string
  int ClanTags::PartyClient_Frame_Stub(const char* s0, [[maybe_unused]] const char* s1)
  {
    return Utils::Hook::Call<int(const char*, const char*)>(0x14028A5C0)(s0, GamerProfile_GetClanName(0)); // I_strcmp
  }

  // clanAbbrev is always an empty string
  void ClanTags::Party_UpdateClanName_Stub(Game::PartyData* party, [[maybe_unused]] const char* clanAbbrev)
  {
    Utils::Hook::Call<void(Game::PartyData*, const char*)>(0x14010C2E0)(party, GamerProfile_GetClanName(0)); // Party_UpdateClanName
  }

  void ClanTags::PlayerCards_SetCachedPlayerData_Stub(int localClientNum, int clientNum, int cardIndex)
  {
    PlayerCards_SetCachedPlayerData_Original(localClientNum, clientNum, cardIndex);

    // Note that the game fills the card, leaving its clan empty, only for
    // these arguments and only if the client info is valid.
    //
    if (localClientNum != 0 ||
        clientNum < 0 || clientNum >= static_cast<int>(Game::MAX_CLIENTS) ||
        cardIndex < 0 || cardIndex >= static_cast<int>(Game::MAX_CLIENTS) ||
        !Game::cgClientInfo[clientNum].infoValid)
    {
      return;
    }

    Game::PlayerCardData& data (Game::playerCardCache[localClientNum * Game::MAX_CLIENTS + cardIndex]);
    Game::I_strncpyz(data.clanAbbrev, ClientState[clientNum], sizeof(Game::PlayerCardData::clanAbbrev));
  }

  Game::PlayerCardData* ClanTags::PlayerCards_GetLiveProfileDataForClient_Stub(const unsigned int clientIndex)
  {
    Game::PlayerCardData* result = Utils::Hook::Call<Game::PlayerCardData*(unsigned int)>(0x1401E63C0)(clientIndex);
    Game::I_strncpyz(result->clanAbbrev, GamerProfile_GetClanName(static_cast<int>(clientIndex)), sizeof(Game::PlayerCardData::clanAbbrev));

    return result;
  }

  Game::PlayerCardData* ClanTags::PlayerCards_GetLiveProfileDataForController_Stub(const unsigned int controllerIndex)
  {
    Game::PlayerCardData* result = Utils::Hook::Call<Game::PlayerCardData*(unsigned int)>(0x1401E64A0)(controllerIndex);
    AssertIn(controllerIndex, Game::MAX_LOCAL_CLIENTS);
    Game::I_strncpyz(result->clanAbbrev, GamerProfile_GetClanName(static_cast<int>(controllerIndex)), sizeof(Game::PlayerCardData::clanAbbrev));

    return result;
  }

  ClanTags::ClanTags()
  {
    Events::OnDvarInit([]
    {
      ClanName = Game::Dvar_RegisterString("clanName", "", Game::DVAR_ARCHIVE, "Your clan abbreviation");
    });

    std::memset(&ClientState, 0, sizeof(char[Game::MAX_CLIENTS][MAX_CLAN_NAME_LENGTH]));

    ServerCommands::OnCommand(22, [](const Command::Params* params)
    {
      if (std::strcmp(params->get(1), "clanNames") == 0)
      {
        if (params->size() == 3)
        {
          ParseClanTags(params->get(2));
          return true;
        }
      }

      return false;
    });

    Utils::Hook(0x1401FC061, Dvar_InfoString_Stub, HOOK_CALL).install()->quick();

    // Note that x64 inlines a copy of ClientUserinfoChanged into
    // ClientConnect.
    //
    Utils::Hook(0x1401969AA, ClanTags_ClientUserinfoChanged_Stub, HOOK_CALL).install()->quick();
    Utils::Hook(0x140196068, ClanTags_ClientConnectUserinfo_Stub, HOOK_CALL).install()->quick();

    // clanName before playerName
    Utils::Hook(0x1400E3D62, ClanTags_DrawPlayerNameOnScoreboard, HOOK_JUMP).install()->quick();

    Utils::Hook(0x140106448, PartyClient_Frame_Stub, HOOK_CALL).install()->quick();
    Utils::Hook(0x14010646B, Party_UpdateClanName_Stub, HOOK_CALL).install()->quick();

    // clanName in the PlayerCard (GetPlayerCardClientData)
    PlayerCards_SetCachedPlayerData_Original = Utils::Hook::Detour(0x1401E67C0, 6, PlayerCards_SetCachedPlayerData_Stub);
    Utils::Hook(0x14024FFFD, PlayerCards_GetLiveProfileDataForClient_Stub, HOOK_CALL).install()->quick();
    Utils::Hook(0x140250009, PlayerCards_GetLiveProfileDataForController_Stub, HOOK_CALL).install()->quick();

    // clanName in CG_Obituary
    Utils::Hook(0x1400AF0D4, PlayerName::GetClientName, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400AF12C, PlayerName::GetClientName, HOOK_CALL).install()->quick();
  }
}

void ClanTags_ClientUserinfoChanged(const char* s, int clientNum)
{
  Components::ClanTags::ClientUserinfoChanged(s, clientNum);
}

const char* ClanTags_GetClanTagWithName(int clientNum, const char* playerName)
{
  return Components::ClanTags::GetClanTagWithName(clientNum, playerName);
}
