#include <Utils/InfoString.hpp>

#include "Dedicated.hpp"
#include "CardTitles.hpp"
#include "ClanTags.hpp"
#include "Events.hpp"
#include "Party.hpp"
#include "ServerCommands.hpp"

namespace Components
{
  SteamID Dedicated::PlayerGuids[18][2];

  Dvar::Var Dedicated::SVLanOnly;
  Dvar::Var Dedicated::SVMOTD;
  Dvar::Var Dedicated::COMLogFilter;

  const Game::dvar_t* Dedicated::com_dedicated;

  bool Dedicated::IsEnabled()
  {
    static std::optional<bool> flag;

    if (!flag.has_value())
    {
      flag.emplace(Flags::HasFlag("dedicated"));
    }

    return flag.value();
  }

  bool Dedicated::IsRunning()
  {
    assert(*Game::sv_running);
    return *Game::sv_running && (*Game::sv_running)->current.enabled;
  }

  void Dedicated::InitDedicatedServer()
  {
    static const char* fastfiles[7] =
    {
      "code_post_gfx_mp",
      "localized_code_post_gfx_mp",
      "ui_mp",
      "localized_ui_mp",
      "common_mp",
      "localized_common_mp",
      "patch_mp"
    };

    std::memcpy(reinterpret_cast<void*>(0x148CCA7E0), &fastfiles, sizeof(fastfiles));
    Game::R_LoadGraphicsAssets();

    Utils::Hook::Call<void()>(0x1400FBD40)(); // CL_InitRenderer
  }

  void Dedicated::PostInitialization()
  {
    Command::Execute("exec autoexec.cfg"); // Can be used by mods / server owners at will : Currently shows an error message on a default setup
    Command::Execute("onlinegame 1");
    Command::Execute("exec default_xboxlive.cfg");
    Command::Execute("xblive_rankedmatch 1");
    Command::Execute("xblive_privatematch 1");
    Command::Execute("xblive_privateserver 0");
    Command::Execute("xstartprivatematch");
    //Command::Execute("xstartlobby");
    Command::Execute("cl_maxpackets 125");
    Command::Execute("snaps 30");
    Command::Execute("com_maxfps 125");

    Game::Com_AddStartupCommands();
  }

  void Dedicated::PostInitialization_Hk()
  {
    PostInitialization();

    Utils::Hook::Call<void()>(0x1401F3BE0)(); // Com_EventLoop
  }

  void Dedicated::Com_ClampMsec(const int msec)
  {
    if (msec > 500 && msec < 500000)
    {
      Game::Com_PrintWarning(Game::CON_CHANNEL_SYSTEM, "Hitch warning: %i msec frame time\n", msec);
    }
  }

  void Dedicated::TransmitGuids()
  {
    std::string list = Utils::String::VA("%c", 20);

    for (std::size_t i = 0; i < Game::MAX_CLIENTS; ++i)
    {
      if (Game::svs_clients[i].header.state >= 3)
      {
        list.append(Utils::String::VA(" %llX", Game::svs_clients[i].steamID));

        Utils::InfoString info(Game::svs_clients[i].userinfo);
        list.append(Utils::String::VA(" %llX", std::strtoull(info.get("realsteamId").data(), nullptr, 16)));
      }
      else
      {
        list.append(" 0 0");
      }
    }

    Game::SV_GameSendServerCommand(-1, Game::SV_CMD_CAN_IGNORE, list.data());
  }

  void Dedicated::TimeWrapStub(Game::errorParm_t code, const char* message)
  {
    Scheduler::Once([]
    {
      std::string mapname = (*Game::sv_mapname)->current.string;

      if (!Party::IsEnabled()) // Time wrapping should not occur in party servers, but yeah...
      {
        if (mapname.empty()) mapname = "mp_rust";
        Command::Execute(std::format("map {}", mapname), true);
      }
    }, Scheduler::Pipeline::SERVER);

    Game::Com_Error(code, message);
  }

  void Dedicated::Heartbeat()
  {
    // Do not send a heartbeat if sv_lanOnly is set to true
    if (SVLanOnly.get<bool>())
    {
      return;
    }

    const auto masterPort = (*Game::com_masterPort)->current.unsignedInt;
    const auto* masterServerName = (*Game::com_masterServerName)->current.string;

    Network::Address master(Utils::String::VA("%s:%u", masterServerName, masterPort));

    Logger::Print("Sending heartbeat to master: {}:{}\n", masterServerName, masterPort);
    Network::SendCommand(master, "heartbeat", "IW4");
  }

  Dedicated::Dedicated()
  {
    COMLogFilter = Dvar::Register<bool>("com_logFilter", true,
      Game::DVAR_LATCH, "Removes ~95% of unneeded lines from the log");

    if (IsEnabled())
    {
      // Make sure all callbacks are handled
      Scheduler::Loop(Steam::SteamAPI_RunCallbacks, Scheduler::Pipeline::SERVER);

      // Note that on x86 we also nop the 'dvar set' and 'sending splash open'
      // lines here. x64 compiles them out, so there is nothing left to silence.
      //
      Utils::Hook(0x1401F5633, InitDedicatedServer, HOOK_CALL).install()->quick(); // Com_Init_Try_Block_Function

      // steam sometimes doesn't like the server
      //
      // On x86 we return from Steam_CheckConsistency() right away. x64 inlines
      // it into Steam_Frame(), so instead we turn the jnz in front of it into a
      // jmp ('nop; jmp') and always skip it.
      //
      Utils::Hook::Set<WORD>(0x14024BC13, 0xE990);

      Utils::Hook::Set<BYTE>(0x14010E2F0, 0xC3); // self-registration on party

      // other party stuff?
      //
      // Note that x64 also inlines this into PartyHost_StartParty() twice: once
      // for the start itself and once more in what looks like the host key
      // handler, which it seems to have inlined there as well. So we jump over
      // both copies, the second one with a short jmp that we write over its
      // first instruction.
      //
      Utils::Hook::Set<BYTE>(0x14010EE40, 0xC3);
      Utils::Hook::Set<BYTE>(0x1401123E4, 0xEB);
      Utils::Hook::Set<BYTE>(0x140112610, 0xEB);
      Utils::Hook::Set<BYTE>(0x140112611, 0x5C);

      Utils::Hook::Set<BYTE>(0x1401B4DC0, 0xC3); // upnp stuff

      Utils::Hook::Set<BYTE>(0x1400FAD9F, 0x04); // make CL_Frame do client packets, even for game state 9
      Utils::Hook::Set<BYTE>(0x1402C0140, 0xC3); // init sound system (1)
      Utils::Hook::Set<BYTE>(0x140032930, 0xC3); // start render thread
      Utils::Hook::Set<BYTE>(0x1400FBD40, 0xC3); // R_Init caller
      Utils::Hook::Set<BYTE>(0x1402445A0, 0xC3); // init sound system (2)
      Utils::Hook::Set<BYTE>(0x1400F1750, 0xC3); // Com_Frame audio processor?
      Utils::Hook::Set<BYTE>(0x140101C10, 0xC3); // called from Com_Frame, seems to do renderer stuff
      Utils::Hook::Set<BYTE>(0x1400F8190, 0xC3); // CL_CheckForResend, which tries to connect to the local server constantly

      // Note that the x64 R_Shutdown() doesn't have the bsp checks that we nop
      // on x86, and it compiles out the warnings about removed subsystems that
      // we nop there as well.
      //
      Utils::Hook::Set<BYTE>(0x1402A8D20, 0xC3); // some mixer-related function called on shutdown
      Utils::Hook::Set<BYTE>(0x1401F4960, 0);    // masterServerName flags

      Utils::Hook::Nop(0x1400FD65E, 2);          // some check preventing proper game functioning
      Utils::Hook::Nop(0x1402366FB, 6);          // cl->messageAcknowledge > cl->gamestateMessageNum check in SV_ExecuteClientMessage
      Utils::Hook::Nop(0x1400FD682, 5);          // some deinit renderer function
      Utils::Hook::Nop(0x1402A42FE, 5);          // function detecting video card, causes Direct3DCreate9 to be called

      // recommended settings check
      //
      // On x86 this is one run of code that we just nop. x64 interleaves the
      // check with restoring registers, so we can't nop all of it. Instead we
      // nop the call and jump over the rest of the check.
      //
      Utils::Hook::Nop(0x1401F53A5, 5);
      Utils::Hook::Set<BYTE>(0x1401F53BC, 0xEB);

      Utils::Hook::Nop(0x1402A4355, 5);          // Disable splash screen

      // do not trigger host migration, even if the server is a 'bad host'
      // Note that x64 inlines the bad host test into its caller, so this is
      // where we jump over it.
      //
      Utils::Hook::Set<BYTE>(0x14023A4EF, 0xEB);

      // isHost script call return 0
      Utils::Hook::Nop(0x14019863B, 4);

      // r_loadForRenderer default to 0
      Utils::Hook::Set<BYTE>(0x14002E80B, 0);

      // disable cheat protection on onlinegame
      Utils::Hook::Set<BYTE>(0x1400FAF69, 0x80);

      // some d3d9 call on error
      Utils::Hook::Set<BYTE>(0x140033AD0, 0xC3);

      // stop saving a config_mp.cfg
      // Note that x64 inlines Com_WriteConfiguration() into Com_Frame(), so we
      // always take the jump over it.
      //
      Utils::Hook::Set<WORD>(0x1401F3EF8, 0xE990);

      // don't load the config
      Utils::Hook::Nop(0x1401F51BF, 5);

      // Intercept time wrapping
      Utils::Hook(0x14023A714, TimeWrapStub, HOOK_CALL).install()->quick();

      Events::OnDvarInit([]
      {
        SVMOTD = Dvar::Register<const char*>("sv_motd", "", Game::DVAR_NONE, "A custom message of the day for servers");
        SVLanOnly = Dvar::Register<bool>("sv_lanOnly", false, Game::DVAR_NONE, "Don't act as node");

        static const char* g_dedicatedEnumNames[] =
        {
          "listen server",
          "dedicated LAN server",
          "dedicated internet server",
          nullptr,
        };

        // IW5MP Dedicated Server adds another flag. That flag should not exist on this version of IW4
        com_dedicated = Game::Dvar_RegisterEnum("dedicated", g_dedicatedEnumNames, 2, Game::DVAR_ROM, "True if this is a dedicated server");
        // Dedicated only behaviour from IW5MP Dedicated Server.
        if (com_dedicated->current.integer != 1 && com_dedicated->current.integer != 2)
        {
          Game::DvarValue value;
          value.integer = 0;
          Game::Dvar_SetVariant(const_cast<Game::dvar_t*>(com_dedicated), value, Game::DVAR_SOURCE_INTERNAL);
        }
      });

      // Post initialization point
      Utils::Hook(0x1401F6776, PostInitialization_Hk, HOOK_CALL).install()->quick();

      Utils::Hook(0x1401F407C, Dedicated_Com_ClampMsec_Stub, HOOK_JUMP).install()->quick(); // Com_Frame_Try_Block_Function
      Utils::Hook::Nop(0x1401F407C + 5, 1);

      // Transmit custom data
      Scheduler::Loop([]
      {
        CardTitles::SendCustomTitlesToClients();
        ClanTags::SendClanTagsToClients();
      }, Scheduler::Pipeline::SERVER, 10s);

      // Heartbeats
      Scheduler::Once(Heartbeat, Scheduler::Pipeline::SERVER);
      Scheduler::Loop(Heartbeat, Scheduler::Pipeline::SERVER, 2min);
    }
    else
    {
      ZeroMemory(PlayerGuids, sizeof(PlayerGuids));

      // Intercept server commands
      // Note that on x86 we also tell Steam that we played with each player.
      // That goes through the Steam proxy, which the x64 build doesn't have (see
      // premake5.lua for why).
      //
      ServerCommands::OnCommand(20, [](const Command::Params* params)
      {
        for (int client = 0; client < 18; client++)
        {
          PlayerGuids[client][0].bits = std::strtoull(params->get(2 * client + 1), nullptr, 16);
          PlayerGuids[client][1].bits = std::strtoull(params->get(2 * client + 2), nullptr, 16);
        }

        return true;
      });
    }

    Scheduler::Loop([]
    {
      if (IsRunning())
      {
        TransmitGuids();
      }
    }, Scheduler::Pipeline::SERVER, 15s);
  }
}

void Dedicated_Com_ClampMsec(int msec)
{
  Components::Dedicated::Com_ClampMsec(msec);
}
