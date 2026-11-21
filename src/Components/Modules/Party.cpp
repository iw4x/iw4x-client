#include <Utils/InfoString.hpp>

#include "Party.hpp"
#include "Auth.hpp"
#include "Download.hpp"
#include "FastFiles.hpp"
#include "Controller.hpp"
#include "Maps.hpp"
#include "ModList.hpp"
#include "Node.hpp"
#include "ServerList.hpp"
#include "Stats.hpp"
#include "TextRenderer.hpp"
#include "Voice.hpp"
#include "Events.hpp"

#include <version.hpp>

#define CL_MOD_LOADING

namespace Components
{
  class JoinContainer
  {
  public:
    enum MatchType : int32_t {
      NO_MATCH = 0,
      PARTY_LOBBY = 1,
      DEDICATED_MATCH = 2,
      PRIVATE_PARTY = 3,

      COUNT
    };

    Network::Address target;
    std::string challenge;
    std::string motd;
    DWORD joinTime;
    bool valid;
    bool downloadOnly;
    MatchType matchType;

    Utils::InfoString info;

    // Party-specific stuff
    DWORD requestTime;
    bool awaitingPlaylist;
  };

  static JoinContainer Container;
  std::map<std::uint64_t, Network::Address> Party::LobbyMap;

  Dvar::Var Party::PartyEnable;
  Dvar::Var Party::ServerVersion;

  SteamID Party::GenerateLobbyId()
  {
    SteamID id;

    id.accountID = Game::Sys_Milliseconds();
    id.universe = 1;
    id.accountType = 8;
    id.accountInstance = 0x40000;

    return id;
  }

  Network::Address Party::Target()
  {
    return Container.target;
  }

  void Party::Connect(Network::Address target, bool downloadOnly)
  {
    // We send our stats to the server while connecting and the game refuses
    // to read them once we have started, so we need them before we connect.
    // They may not be loaded yet if, say, we were told to connect on startup.
    // In this case, read them first and connect once they are in.
    //
    // Note that if we are asked to connect again while waiting, then we only
    // keep the last target.
    //
    static std::optional<std::pair<Network::Address, bool>> pending;

    if (!Game::LiveStorage_DoWeHaveStats(0))
    {
      const auto waiting = pending.has_value();
      pending = std::make_pair(target, downloadOnly);

      if (waiting)
        return;

      Game::LiveStorage_ReadStats(0);
      Command::Execute("openmenu popup_reconnectingtoparty");

      Scheduler::Schedule([start = Game::Sys_Milliseconds()]
      {
        if (Game::LiveStorage_DoWeHaveStats(0))
        {
          const auto [target, downloadOnly] = *pending;
          pending.reset();
          Connect(target, downloadOnly);
          return true;
        }

        // Give up if they don't come in time rather than join without them.
        //
        if (Game::Sys_Milliseconds() - start > 10'000)
        {
          pending.reset();
          ConnectError("Unable to read your stats.");
          return true;
        }

        return false;
      }, Scheduler::Pipeline::MAIN);

      return;
    }

    Node::Add(target);

    Container.valid = true;
    Container.awaitingPlaylist = false;
    Container.joinTime = Game::Sys_Milliseconds();
    Container.target = target;
    Container.challenge = Utils::Cryptography::Rand::GenerateChallenge();
    Container.downloadOnly = downloadOnly;

    Network::SendCommand(Container.target, "getinfo", Container.challenge);

    Command::Execute("openmenu popup_reconnectingtoparty");
  }

  const char* Party::GetLobbyInfo(SteamID lobby, const std::string& key)
  {
    if (LobbyMap.contains(lobby.bits))
    {
      Network::Address address = LobbyMap[lobby.bits];

      if (key == "addr"s)
      {
        return Utils::String::VA("%d", address.getIP().full);
      }

      if (key == "port"s)
      {
        return Utils::String::VA("%d", address.getPort());
      }
    }

    return "212";
  }

  void Party::RemoveLobby(SteamID lobby)
  {
    LobbyMap.erase(lobby.bits);
  }

  void Party::ConnectError(const std::string& message)
  {
    Command::Execute("closemenu popup_reconnectingtoparty");
    Dvar::Var("partyend_reason").set(message);
    Command::Execute("openmenu menu_xboxlive_partyended");
  }

  std::string Party::GetMotd()
  {
    return Container.motd;
  }

  std::string Party::GetHostName()
  {
    return Container.info.get("hostname");
  }

  int Party::GetMaxClients()
  {
    const auto value = Container.info.get("sv_maxclients");
    return std::strtol(value.data(), nullptr, 10);
  }

  bool Party::PlaylistAwaiting()
  {
    return Container.awaitingPlaylist;
  }

  void Party::PlaylistContinue()
  {
    Dvar::Var("xblive_privateserver").set(false);

    // Ensure we can join
    *Game::g_lobbyCreateInProgress = false;

    Container.awaitingPlaylist = false;

    SteamID id = GenerateLobbyId();

    // Temporary workaround
    // TODO: Patch the 127.0.0.1 -> loopback mapping in the party code
    if (Container.target.isLoopback())
    {
      if (*Game::numIP)
      {
        Container.target.setIP(*Game::localIP);
        Container.target.setType(Game::netadrtype_t::NA_IP);

        Logger::Print("Trying to connect to party with loopback address, using a local ip instead: {}\n", Container.target.getString());
      }
      else
      {
        Logger::Print("Trying to connect to party with loopback address, but no local ip was found.\n");
      }
    }

    LobbyMap[id.bits] = Container.target;

    Game::Steam_JoinLobby(id, 0);
  }

  void Party::PlaylistError(const std::string& error)
  {
    Container.valid = false;
    Container.awaitingPlaylist = false;

    ConnectError(error);
  }

  // Note that x86 hands the menus a zeroed spot of the game's memory for
  // onlinegame, which reads as a dvar that is false. On x64 we keep our own.
  //
  const Game::dvar_t* Party::UIDvarIntStub(const char* dvar)
  {
    static const Game::dvar_t offline {};

    if (!_stricmp(dvar, "onlinegame") && !Stats::IsMaxLevel())
    {
      return &offline;
    }

    return Game::Dvar_FindVar(dvar);
  }

  int Party::PartyMigrate_HandlePacket()
  {
    return 0;
  }

  bool Party::IsInLobby()
  {
    return (!Dedicated::IsRunning() && PartyEnable.get<bool>() && Dvar::Var("party_host").get<bool>());
  }

  bool Party::IsInUserMapLobby()
  {
    return (IsInLobby() && Maps::IsUserMap((*Game::ui_mapname)->current.string));
  }

  bool Party::IsEnabled()
  {
    return PartyEnable.get<bool>();
  }

  void SV_SpawnServer_Com_SyncThreads_Hook()
  {
    Game::Com_SyncThreads(); // Com_SyncThreads

    // Whenever the game starts a server,
    // RMsg_SendMessages so that everybody gets the PartyGo message!
    // (Otherwise Com_Try_Block doesn't send it until we're done loading, which times out some people)
    Game::RMesg_SendMessages();
  }



  Party::Party()
  {

    Events::OnDvarInit([]
    {
      ServerVersion = Dvar::Register<const char*>("sv_version", "", Game::DVAR_SERVERINFO | Game::DVAR_INIT, "Server version");
    });

    PartyEnable = Dvar::Register<bool>("party_enable", Dedicated::IsEnabled(), Game::DVAR_NONE, "Enable party system");
    Dvar::Register<bool>("xblive_privatematch", true, Game::DVAR_INIT, "private match");

    // Kill the party migrate handler - it's not necessary and has apparently been used in the past for trickery?
    Utils::Hook(0x1401152E0, PartyMigrate_HandlePacket, HOOK_JUMP).install()->quick();

    // various changes to SV_DirectConnect-y stuff to allow non-party joinees
    //
    // Note that x86 writes the jmp opcode over the first byte of the jge here
    // and so jumps by a displacement that starts with its second byte. What
    // it means to do, and what we do, is to always take the jump.
    //
    Utils::Hook::Set<WORD>(0x140235FD6, 0xE990);
    Utils::Hook::Set<BYTE>(0x140236098, 0xEB);

    // Note that in an online game x64 SV_UserinfoChanged() drops the client
    // the session doesn't hold, where x86 registers it with the session. So
    // we skip the drop where x86 skips the registration.
    //
    Utils::Hook::Nop(0x140238096, 2);
    Utils::Hook::Set<BYTE>(0x14023810C, 0xEB);

    // disable configstring checksum matching (it's unreliable at most)
    Utils::Hook::Set<BYTE>(0x140239CEE, 0xEB); // SV_SpawnServer
    Utils::Hook::Set<BYTE>(0x1400FFCE8, 0xEB); // CL_ParseGamestate
    Utils::Hook::Set<BYTE>(0x1400FFCDB, 0xEB); // CL_ParseGamestate

    // AnonymousAddRequest
    //
    // Note that x64 compares the 64-bit lobby id in one go, and so we skip
    // one jump where x86 handles each half.
    //
    Utils::Hook::Set<BYTE>(0x14010E5D6, 0xEB);
    Utils::Hook::Nop(0x14010E5FB, 2);

    // HandleClientHandshake
    //
    // As above. Here x64 also jumps when the Steam check fails, where x86
    // jumps when it passes.
    //
    Utils::Hook::Nop(0x14010D285, 2);
    Utils::Hook::Nop(0x14010D293, 2);

    // Allow local connections
    Utils::Hook::Set<BYTE>(0x140112ACE, 0xEB);

    // LobbyID mismatch
    //
    // As in AnonymousAddRequest.
    //
    Utils::Hook::Nop(0x14010FD84, 6);

    // causes 'does current Steam lobby match' calls in Steam_JoinLobby to be ignored
    Utils::Hook::Set<BYTE>(0x14024C3C9, 0xEB);

    // function checking party heartbeat timeouts, cause random issues
    //
    // Note that x64 inlines PartyHost_TimeoutMembers(), whose call we nop on
    // x86, into PartyHost_Frame(). So we skip the timeout there.
    //
    Utils::Hook::Set<BYTE>(0x140110214, 0xEB); // PartyHost_TimeoutMembers

    // Steam_JoinLobby call causes migration
    Utils::Hook::Nop(0x140107310, 5);
    Utils::Hook::Set<BYTE>(0x14010731F, 0xEB);

    // Allow xpartygo in public lobbies
    //
    // Note that x64 jumps to the same early return from both checks, which
    // we skip.
    //
    Utils::Hook::Nop(0x1400FBEF1, 2);
    Utils::Hook::Nop(0x1400FBEF7, 2);

    // Always open lobby menu when connecting
    // It's not possible to entirely patch it via code
    //Utils::Hook::Set<BYTE>(0x5B1698, 0xEB);
    //Utils::Hook::Nop(0x5029F2, 6);
    //Utils::Hook::SetString(0x70573C, "menu_xboxlive_lobby");

    // Disallow selecting team in private match
    //Utils::Hook::Nop(0x5B2BD8, 6);

    // Force teams, even if not private match
    Utils::Hook::Set<BYTE>(0x140112B72, 0xEB);

    // Force xblive_privatematch 0 and rename it
    //
    // Note that x64 already registers it as 0.
    //
    Utils::Hook::PlaceString(0x1402A22A3, 7, 0x1403A7E68, "xblive_privateserver");

    // Remove migration shutdown, it causes crashes and will be destroyed when erroring anyways
    //
    // Note that x86 also nops the instructions that compute where to store,
    // which x64 shares with the rest of the function.
    //
    Utils::Hook::Nop(0x1400FD52C, 11);

    // Enable XP Bar
    Utils::Hook(0x140251859, UIDvarIntStub, HOOK_CALL).install()->quick();

    // Set NAT to open
    Utils::Hook::Set<int>(0x140426830, 1);

    // Disable host migration
    //
    // Note that x86 also registers party_hostmigration as false, which x64
    // compiles down to true. So we also skip the migration where the host
    // times out, which x86 skips because of that.
    //
    Utils::Hook::Set<BYTE>(0x14010EC66, 0xEB);
    Utils::Hook::Nop(0x140109A69, 5); // PartyMigrate_Frame
    Utils::Hook::Set<BYTE>(0x14010688D, 0xEB); // PartyClient_HostTimedOut

    // Patch playlist stuff for non-party behavior
    //
    // Note that x64 reaches the xblive_privatematch dvar from the next
    // instruction, and so we keep our pointer where it can reach it.
    //
    auto** partyEnable (static_cast<Game::dvar_t**>(Utils::Hook::AllocateNear(0x140000000, sizeof(Game::dvar_t*))));
    *partyEnable = PartyEnable.get<Game::dvar_t*>();

    for (const uintptr_t place: {0x140105528, 0x140108E96, 0x1401050B3})
    {
      Utils::Hook::Displace(place, 7, 0x146784ED0, reinterpret_cast<uintptr_t>(partyEnable));
    }

    // Invert corresponding jumps
    Utils::Hook::Xor<BYTE>(0x140105533, 1);
    Utils::Hook::Xor<BYTE>(0x140108EA4, 1);
    Utils::Hook::Xor<BYTE>(0x1401050BE, 1);

    // Set ui_maxclients to sv_maxclients
    for (const uintptr_t place: {0x140238258, 0x140238928, 0x14019DC89})
    {
      Utils::Hook::Displace(place, 7, 0x140385AC0, 0x1403750E0);
    }

    // Unlatch maxclient dvars
    for (const uintptr_t place: {0x140238254, 0x140238933, 0x14019DC94, 0x14023827D, 0x140238955, 0x14019DCB6})
    {
      Utils::Hook::Xor<DWORD>(place, Game::DVAR_LATCH);
    }

    // Synchronize / Send network messages when the server starts so that clients can load the game with us
    // Otherwise they timeout while the host load
    //
    // Note that x64 inlines the function whose call to Com_SyncThreads() we
    // hook on x86 into PartyHost_StartMatch().
    //
    Utils::Hook(0x140111FAC, SV_SpawnServer_Com_SyncThreads_Hook, HOOK_CALL).install()->quick();

    Command::Add("connect", [](const Command::Params* params)
    {
      if (params->size() < 2)
      {
        return;
      }

      if (Game::CL_IsCgameInitialized())
      {
        Command::Execute("disconnect", false);
        Command::Execute(Utils::String::VA("%s", params->join(0).data()), false);
      }
      else
      {
        Connect(Network::Address(params->get(1)));
      }
    });

    Command::Add("reconnect", []()
    {
      Connect(Container.target);
    });

    if (!Dedicated::IsEnabled())
    {
      Scheduler::Loop([]
      {
        if (Container.valid)
        {
          if ((Game::Sys_Milliseconds() - Container.joinTime) > 10'000)
          {
            Container.valid = false;
            ConnectError("Server connection timed out.");
          }
        }

        if (Container.awaitingPlaylist)
        {
          if ((Game::Sys_Milliseconds() - Container.requestTime) > 5'000)
          {
            Container.awaitingPlaylist = false;
            ConnectError("Playlist request timed out.");
          }
        }

      }, Scheduler::Pipeline::CLIENT);
    }

    // Basic info handler
    Network::OnClientPacket("getInfo", [](const Network::Address& address, [[maybe_unused]] const std::string& data)
    {
      auto botCount = 0;
      auto effectiveClientCount = 0;
      auto maxClientCount = *Game::svs_clientCount;
      const auto securityLevel = Dvar::Var("sv_securityLevel").get<int>();
      const auto* password = *Game::g_password ? (*Game::g_password)->current.string : "";

      if (maxClientCount)
      {
        for (int i = 0; i < maxClientCount; ++i)
        {
          if (Game::svs_clients[i].header.state < Game::CS_ACTIVE) continue;
          if (!Game::svs_clients[i].gentity || !Game::svs_clients[i].gentity->client) continue;

          const auto* client = Game::svs_clients[i].gentity->client;
          const auto team = client->sess.cs.team;
          if (Game::svs_clients[i].bIsTestClient || team == Game::TEAM_SPECTATOR)
          {
            ++botCount;
          }
          else
          {
            ++effectiveClientCount;
          }
        }
      }
      else
      {
        maxClientCount = *Game::party_maxplayers ? (*Game::party_maxplayers)->current.integer : 18;
        effectiveClientCount = Game::PartyHost_CountMembers(Game::g_lobbyData);

        for (std::size_t i = 0; i < std::min (static_cast<std::size_t> (maxClientCount), Game::MAX_CLIENTS); ++i)
        {
          if (Game::svs_clients[i].header.state < Game::CS_ACTIVE)
            continue;

          if (!Game::svs_clients[i].gentity ||
              !Game::svs_clients[i].gentity->client)
            continue;

          const auto client = Game::svs_clients[i].gentity->client;
          const auto team = client->sess.cs.team;

          if (Game::svs_clients[i].bIsTestClient || team == Game::TEAM_SPECTATOR)
          {
            ++botCount;
            --effectiveClientCount;
          }
        }

        if (effectiveClientCount < 0)
          effectiveClientCount = 0;
      }

      Utils::InfoString info;
      info.set("challenge", Utils::ParseChallenge(data));
      info.set("gamename", "IW4");
      info.set("hostname", (*Game::sv_hostname)->current.string);
      info.set("gametype", (*Game::sv_gametype)->current.string);
      info.set("fs_game", (*Game::fs_gameDirVar)->current.string);
      info.set("xuid", Utils::String::VA("%llX", Steam::SteamUser()->GetSteamID().bits));
      info.set("clients", std::to_string(effectiveClientCount));
      info.set("bots", std::to_string(botCount));
      info.set("sv_maxclients", std::to_string(maxClientCount));
      info.set("protocol", std::to_string(PROTOCOL));
      info.set("version", REVISION_STR);
      info.set("checksum", std::to_string(Game::Sys_Milliseconds()));
      info.set("mapname", Dvar::Var("mapname").get<std::string>());
      info.set("isPrivate", *password ? "1" : "0");
      info.set("hc", (Dvar::Var("g_hardcore").get<bool>() ? "1" : "0"));
      info.set("securityLevel", std::to_string(securityLevel));
      info.set("sv_running", (Dedicated::IsRunning() ? "1" : "0"));
      info.set("aimAssist", (Controller::sv_allowAimAssist.get<bool>() ? "1" : "0"));
      info.set("voiceChat", (Voice::SV_VoiceEnabled() ? "1" : "0"));

      // Ensure mapname is set
      if (info.get("mapname").empty() || IsInLobby())
      {
        info.set("mapname", Dvar::Var("ui_mapname").get<const char*>());
      }

      if (Maps::GetUserMap()->isValid())
      {
        info.set("usermaphash", Utils::String::VA("%i", Maps::GetUserMap()->getHash()));
      }
      else if (IsInUserMapLobby())
      {
        info.set("usermaphash", Utils::String::VA("%i", Maps::GetUsermapHash(info.get("mapname"))));
      }

      if (Dedicated::IsEnabled())
      {
        info.set("sv_motd", Dedicated::SVMOTD.get<std::string>());
      }

      // Set matchtype
      // 0 - No match, connecting not possible
      // PARTY_LOBBY - Party, use Steam_JoinLobby to connect
      // DEDICATED_MATCH - Match, use CL_ConnectFromParty to connect
      // PRIVATE_PARTY - Private party, use Steam_JoinLobby to connect (same as party? Kept separated for now)
      bool partyHost = Dvar::Var("party_host").get<bool>();
      if (partyHost) // Party hosting
      {
        if (PartyEnable.get<bool>())
        {
          // Public party
          info.set("matchtype", std::to_string(JoinContainer::MatchType::PARTY_LOBBY));
        }
        else
        {
          // Private party
          info.set("matchtype", std::to_string(JoinContainer::MatchType::PRIVATE_PARTY));
        }
      }
      else if (Dvar::Var("sv_running").get<bool>()) // Match hosting
      {
        info.set("matchtype",  std::to_string(JoinContainer::MatchType::DEDICATED_MATCH));
      }
      else
      {
        info.set("matchtype",  std::to_string(JoinContainer::MatchType::NO_MATCH));
      }

      info.set("wwwDownload", (Download::SV_wwwDownload.get<bool>() ? "1" : "0"));
      info.set("wwwUrl", Download::SV_wwwBaseUrl.get<std::string>());

      Network::SendCommand(address, "infoResponse", info.build());
    });

    Network::OnClientPacket("infoResponse", [](const Network::Address& address, [[maybe_unused]] const std::string& data)
    {
      const Utils::InfoString info(data);

      // Handle connection
      if (Container.valid)
      {
        if (Container.target == address)
        {
          // Invalidate handler for future packets
          Container.valid = false;
          Container.info = info;

          Container.matchType = static_cast<JoinContainer::MatchType>(std::strtol(info.get("matchtype").data(), nullptr, 10));
          auto securityLevel = std::strtoul(info.get("securityLevel").data(), nullptr, 10);
          bool isUsermap = !info.get("usermaphash").empty();
          auto usermapHash = std::strtoul(info.get("usermaphash").data(), nullptr, 10);
#ifdef CL_MOD_LOADING
          std::string mod = (*Game::fs_gameDirVar)->current.string;
#endif
          // set fast server stuff here so its updated when we go to download stuff
          if (info.get("wwwDownload") == "1"s)
          {
            Download::SV_wwwDownload.set(true);
            Download::SV_wwwBaseUrl.set(info.get("wwwUrl"));
          }
          else
          {
            Download::SV_wwwDownload.set(false);
            Download::SV_wwwBaseUrl.set("");
          }

          if (info.get("challenge") != Container.challenge)
          {
            ConnectError("Invalid join response: Challenge mismatch.");
          }
          else if (securityLevel > Auth::GetSecurityLevel())
          {
            Command::Execute("closemenu popup_reconnectingtoparty");
            Auth::IncreaseSecurityLevel(securityLevel, "reconnect");
          }
          else if (Container.matchType == JoinContainer::MatchType::NO_MATCH)
          {
            ConnectError("Server is not hosting a match.");
          }
          else if (Container.matchType >= JoinContainer::MatchType::COUNT || Container.matchType < JoinContainer::MatchType::NO_MATCH)
          {
            ConnectError("Invalid join response: Unknown matchtype");
          }
          else if (Container.info.get("mapname").empty() || Container.info.get("gametype").empty())
          {
            ConnectError("Invalid map or gametype.");
          }
          else if (Container.info.get("isPrivate") == "1"s && Dvar::Var("password").get<std::string>().empty())
          {
            ConnectError("A password is required to join this server! Set it at the bottom of the serverlist.");
          }
          else if (isUsermap && !Download::IsValidMapName(info.get("mapname")))
          {
            ConnectError("Invalid join response: Bad map name.");
          }
          else if (isUsermap && usermapHash != Maps::GetUsermapHash(info.get("mapname")))
          {
            // Make sure the usermap doesn't take the name of a zone that ships
            // with the game. See FastFiles::GetZoneLocation() for details.
            //
            if (FastFiles::IsInstalledZone(info.get("mapname")))
            {
              ConnectError("Invalid join response: Usermap name collides with an installed zone.");
              return;
            }

            Command::Execute("closemenu popup_reconnectingtoparty");
            Download::InitiateMapDownload(info.get("mapname"), info.get("isPrivate") == "1");
          }
#ifdef CL_MOD_LOADING
          else if (!info.get("fs_game").empty() && !Download::IsValidModName(info.get("fs_game")))
          {
            ConnectError("Invalid join response: Bad mod name.");
          }
          else if (!info.get("fs_game").empty() && Utils::String::ToLower(mod) != Utils::String::ToLower(info.get("fs_game")))
          {
            Command::Execute("closemenu popup_reconnectingtoparty");
            Download::InitiateClientDownload(info.get("fs_game"), info.get("isPrivate") == "1"s, false, Container.downloadOnly);
          }
          else if ((*Game::fs_gameDirVar)->current.string[0] != '\0' && info.get("fs_game").empty())
          {
            Game::Dvar_SetString(*Game::fs_gameDirVar, "");

            if (ModList::cl_modVidRestart.get<bool>())
            {
              Command::Execute("vid_restart", false);
            }

            Command::Execute("reconnect", false);
          }
#endif
          else
          {
            // Report a missing DLC pack as a failed join rather than fail
            // with the CheckMapInstalled() error below: that drops us back to
            // the main menu and its message is lost on the way.
            //
            if (const auto message = Maps::GetMissingDlcError(Container.info.get("mapname")))
            {
              ConnectError(*message);
              return;
            }

            if (!Maps::CheckMapInstalled(Container.info.get("mapname"), true)) return;

            Container.motd = TextRenderer::StripMaterialTextIcons(info.get("sv_motd"));

            switch(Container.matchType)
            {
              case JoinContainer::MatchType::DEDICATED_MATCH:
                {
                  int clients;
                  int maxClients;
                  std::string version;

                  try
                  {
                    clients = std::stoi(Container.info.get("clients"));
                    maxClients = std::stoi(Container.info.get("sv_maxclients"));
                    version = Container.info.get("version");
                  }
                  catch ([[maybe_unused]] const std::exception& ex)
                  {
                    ConnectError("Invalid info string");
                    return;
                  }

                  if (clients >= maxClients)
                  {
                    ConnectError("@EXE_SERVERISFULL");
                  }
                  else
                  {
                    Dvar::Var("xblive_privateserver").set(true);
                    ServerVersion.set(version);
                    Game::Menus_CloseAll(Game::uiContext);

                    Game::_XSESSION_INFO hostInfo;
                    Game::CL_ConnectFromParty(0, &hostInfo, *Container.target.get(), 0, 0, Container.info.get("mapname").data(), Container.info.get("gametype").data());
                  }
                }

                break;

              case JoinContainer::MatchType::PARTY_LOBBY:
                {
                  // Send playlist request
                  Container.requestTime = Game::Sys_Milliseconds();
                  Container.awaitingPlaylist = true;
                  Network::SendCommand(Container.target, "getplaylist", Dvar::Var("password").get<std::string>());

                  // This is not a safe method
                  // TODO: Fix actual error!
                  if (Game::CL_IsCgameInitialized())
                  {
                    Command::Execute("disconnect", true);
                  }
                }

                break;



              case JoinContainer::MatchType::PRIVATE_PARTY:
                {
                  PlaylistContinue();
                }
                break;

              // Rejected above.
              //
              case JoinContainer::MatchType::NO_MATCH:
              case JoinContainer::MatchType::COUNT:
                AssertUnreachable;
                break;
            }
          }
        }
      }

      // Note that on x86 we also tell our Steam friends about the server,
      // which the x64 build doesn't have (see premake5.lua for details).
      //
      ServerList::Insert(address, info);
    });
  }
}
