#include "Events.hpp"

namespace Components
{
  Utils::Concurrency::Container<Events::ClientCallback> Events::ClientDisconnectTasks_;
  Utils::Concurrency::Container<Events::ClientConnectCallback> Events::ClientConnectTasks_;
  Utils::Concurrency::Container<Events::Callback> Events::SteamDisconnectTasks_;
  Utils::Concurrency::Container<Events::Callback> Events::ShutdownSystemTasks_;
  Utils::Concurrency::Container<Events::Callback> Events::ClientInitTasks_;
  Utils::Concurrency::Container<Events::Callback> Events::ServerInitTasks_;
  Utils::Concurrency::Container<Events::Callback> Events::DvarInitTasks_;
  Utils::Concurrency::Container<Events::CLDisconnectCallback> Events::CL_DisconnectedTask_;
  Utils::Concurrency::Container<Events::Callback> Events::NetworkInitTasks_;
  Utils::Concurrency::Container<Events::Callback> Events::CGameInitTasks_;
  Utils::Concurrency::Container<Events::Callback> Events::UIInitTasks_;
  Utils::Concurrency::Container<Events::Callback> Events::DatabaseLoadedTasks_;

  Events::ClientCmdCallback Events::ClientCmdButtonsTasks_;
  Events::ClientCmdCallback Events::ClientKeyMoveTasks_;

  void (*Events::CL_Disconnect_Original)(int localClientNum);
  void (*Events::NET_Config_Original)(int enableNetworking);

  void Events::OnClientDisconnect(const std::function<void(int clientNum)>& callback)
  {
    ClientDisconnectTasks_.access([&callback](ClientCallback& tasks)
    {
      tasks.emplace_back(callback);
    });
  }

  void Events::OnClientConnect(const std::function<void(Game::client_s* cl)>& callback)
  {
    ClientConnectTasks_.access([&callback](ClientConnectCallback& tasks)
    {
      tasks.emplace_back(callback);
    });
  }

  void Events::OnSteamDisconnect(const std::function<void()>& callback)
  {
    SteamDisconnectTasks_.access([&callback](Callback& tasks)
    {
      tasks.emplace_back(callback);
    });
  }

  void Events::OnCLDisconnected(const std::function<void(bool)>& callback)
  {
    CL_DisconnectedTask_.access([&callback](CLDisconnectCallback& tasks)
    {
      tasks.emplace_back(callback);
    });
  }

  void Events::OnVMShutdown(const std::function<void()>& callback)
  {
    ShutdownSystemTasks_.access([&callback](Callback& tasks)
    {
      tasks.emplace_back(callback);
    });
  }

  void Events::OnClientInit(const std::function<void()>& callback)
  {
    ClientInitTasks_.access([&callback](Callback& tasks)
    {
      tasks.emplace_back(callback);
    });
  }

  void Events::AfterUIInit(const std::function<void()>& callback)
  {
    UIInitTasks_.access([&callback](Callback& tasks)
    {
      tasks.emplace_back(callback);
    });
  }

  void Events::AfterDatabaseLoaded(const std::function<void()>& callback)
  {
    DatabaseLoadedTasks_.access([&callback](Callback& tasks)
    {
      tasks.emplace_back(callback);
    });
  }

  void Events::OnClientCmdButtons(const std::function<void(Game::usercmd_s*)>& callback)
  {
    ClientCmdButtonsTasks_.emplace_back(callback);
  }

  void Events::OnClientKeyMove(const std::function<void(Game::usercmd_s*)>& callback)
  {
    ClientKeyMoveTasks_.emplace_back(callback);
  }

  void Events::OnSVInit(const std::function<void()>& callback)
  {
    ServerInitTasks_.access([&callback](Callback& tasks)
    {
      tasks.emplace_back(callback);
    });
  }

  void Events::OnDvarInit(const std::function<void()>& callback)
  {
    DvarInitTasks_.access([&callback](Callback& tasks)
    {
        tasks.emplace_back(callback);
    });
  }

  void Events::OnNetworkInit(const std::function<void()>& callback)
  {
    NetworkInitTasks_.access([&callback](Callback& tasks)
    {
      tasks.emplace_back(callback);
    });
  }

  void Events::OnCGameInit(const std::function<void()>& callback)
  {
    CGameInitTasks_.access([&callback](Callback& tasks)
    {
      tasks.emplace_back(callback);
    });
  }

  /*
   * Should be called when a client drops from the server
   * but not "between levels" (Quake-III-Arena)
   */
  void Events::ClientDisconnect_Hk(const int clientNum)
  {
    ClientDisconnectTasks_.access([&clientNum](ClientCallback& tasks)
    {
      for (const auto& func : tasks)
      {
        func(clientNum);
      }
    });

    Utils::Hook::Call<void(int)>(0x1401961A0)(clientNum); // ClientDisconnect
  }

  void Events::SV_UserinfoChanged_Hk(Game::client_s* cl)
  {
    ClientConnectTasks_.access([&cl](ClientConnectCallback& tasks)
    {
      for (const auto& func : tasks)
      {
        func(cl);
      }
    });

    Utils::Hook::Call<void(Game::client_s*)>(0x140237DE0)(cl); // SV_UserinfoChanged
  }

  void Events::SteamDisconnect_Hk()
  {
    SteamDisconnectTasks_.access([](Callback& tasks)
    {
      for (const auto& func : tasks)
      {
        func();
      }
    });

    Utils::Hook::Call<void()>(0x14024A220)(); // LiveSteam_Client_SteamDisconnect
  }

  void Events::Scr_ShutdownSystem_Hk(unsigned char sys)
  {
    ShutdownSystemTasks_.access([](Callback& tasks)
    {
      for (const auto& func : tasks)
      {
        func();
      }
    });

    Utils::Hook::Call<void(unsigned char)>(0x140229310)(sys); // Scr_ShutdownSystem
  }

  void Events::CL_InitOnceForAllClients_HK()
  {
    ClientInitTasks_.access([](Callback& tasks)
    {
      for (const auto& func : tasks)
      {
        func();
      }

      tasks = {}; // Only called once. Clear
    });

    Utils::Hook::Call<void()>(0x1400FAEE0)(); // CL_InitOnceForAllClients
  }

  void Events::CL_CmdButtons_Hk(int localClientNum, Game::usercmd_s* cmd)
  {
    Utils::Hook::Call<void(int, Game::usercmd_s*)>(0x1400F5B70)(localClientNum, cmd); // CL_CmdButtons

    for (const auto& func : ClientCmdButtonsTasks_)
    {
      func(cmd);
    }
  }

  void Events::CL_Disconnect_Hk(int localClientNum)
  {
    // Since we hook the start of CL_Disconnect() (see below), we have to work
    // out beforehand what it's going to do. It does nothing unless the client
    // is running, and it resets the connection state on the way out, so we
    // take both now.
    //
    const bool running(Game::clientUIActives[0].isRunning);
    const bool wasConnected(Game::clientUIActives[0].connectionState >= Game::CA_CONNECTED);

    CL_Disconnect_Original(localClientNum);

    if (!running)
    {
      return;
    }

    CL_DisconnectedTask_.access([&](CLDisconnectCallback& tasks)
    {
      for (const auto& func : tasks)
      {
        func(wasConnected);
      }
    });
  }

  void Events::CL_KeyMove_Hk(int localClientNum, Game::usercmd_s* cmd)
  {
    Utils::Hook::Call<void(int, Game::usercmd_s*)>(0x1400F6DE0)(localClientNum, cmd); // CL_KeyMove

    for (const auto& func : ClientKeyMoveTasks_)
    {
      func(cmd);
    }
  }

  void Events::SV_Init_Hk()
  {
    ServerInitTasks_.access([](Callback& tasks)
    {
      for (const auto& func : tasks)
      {
        func();
      }

      tasks = {}; // Only called once. Clear
    });

    Utils::Hook::Call<void()>(0x1402319F0)(); // SV_InitGameMode
  }

  void Events::Com_InitDvars_Hk()
  {
    DvarInitTasks_.access([](Callback& tasks)
    {
      for (const auto& func : tasks)
      {
        func();
      }

      tasks = {}; // Only called once. Clear
    });

    Utils::Hook::Call<void()>(0x1401F48C0)(); // Com_InitDvars
  }

  int Events::CL_InitCGame_Hk()
  {
    CGameInitTasks_.access([](Callback& tasks)
    {
      for (const auto& func : tasks)
      {
        func();
      }
    });

    return Game::Sys_Milliseconds();
  }

  void Events::NET_Config_Hk(int enableNetworking)
  {
    NET_Config_Original(enableNetworking);

    // On x86 we run these right after NET_OpenIP() opens the sockets. x64
    // inlines NET_OpenIP() into NET_Config(), so we run them after the whole
    // of NET_Config() instead, and only if it actually got us an IP socket.
    //
    if (*Game::ip_socket == 0)
    {
      return;
    }

    NetworkInitTasks_.access([](Callback& tasks)
    {
      for (const auto& func : tasks)
      {
        func();
      }

      tasks = {}; // Only called once. Clear
    });
  }

  void Events::UI_Init_Hk(int localClientNum)
  {
    Utils::Hook::Call<void(int)>(0x14026D960)(localClientNum); // UI_Init

    UIInitTasks_.access([](Callback& tasks)
    {
      for (const auto& func : tasks)
      {
        func();
      }
    });
  }

  // DB_PostLoadXZone() calls this once it has linked the assets of the zones
  // the database thread finished loading, which is when FastFiles::Ready()
  // turns true.
  //
  void Events::Sys_DatabaseCompleted2_Hk()
  {
    Utils::Hook::Call<void()>(0x140208A10)(); // Sys_DatabaseCompleted2

    DatabaseLoadedTasks_.access([](Callback& tasks)
    {
      for (const auto& func : tasks)
      {
        func();
      }
    });
  }

  Events::Events()
  {
    // Note that x64 also inlines SV_FreeClient() into one of its callers, so
    // there are two calls to hook where x86 has one.
    //
    Utils::Hook(0x140236926, ClientDisconnect_Hk, HOOK_CALL).install()->quick(); // SV_FreeClient
    Utils::Hook(0x140236A4B, ClientDisconnect_Hk, HOOK_CALL).install()->quick(); // SV_FreeClient

    Utils::Hook(0x14023639E, SV_UserinfoChanged_Hk, HOOK_CALL).install()->quick(); // SV_DirectConnect

    Utils::Hook(0x1400F8E3F, SteamDisconnect_Hk, HOOK_CALL).install()->quick(); // CL_Disconnect

    Utils::Hook(0x1401A0EE6, Scr_ShutdownSystem_Hk, HOOK_CALL).install()->quick(); // G_LoadGame
    Utils::Hook(0x14019F41E, Scr_ShutdownSystem_Hk, HOOK_CALL).install()->quick(); // G_ShutdownGame

    Utils::Hook(0x1401F55FA, CL_InitOnceForAllClients_HK, HOOK_CALL).install()->quick(); // Com_Init_Try_Block_Function

    Utils::Hook(0x1400F7BA0, CL_CmdButtons_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400F7BAA, CL_KeyMove_Hk, HOOK_CALL).install()->quick();

    Utils::Hook(0x1400F4D5D, CL_InitCGame_Hk, HOOK_CALL).install()->quick(); // CL_InitCGame

    Utils::Hook(0x1401F50A5, Com_InitDvars_Hk, HOOK_CALL).install()->quick(); // Com_Init_Try_Block_Function

    Utils::Hook(0x1402387DB, SV_Init_Hk, HOOK_CALL).install()->quick(); // SV_Init

    // DB_PostLoadXZone() completes in one of two places: after it has linked
    // the assets the database thread copied over, and (as a tail call) when
    // there was nothing to link.
    //
    Utils::Hook(0x14012F337, Sys_DatabaseCompleted2_Hk, HOOK_CALL).install()->quick(); // DB_PostLoadXZone
    Utils::Hook(0x14012F1D1, Sys_DatabaseCompleted2_Hk, HOOK_JUMP).install()->quick(); // DB_PostLoadXZone

    NET_Config_Original = Utils::Hook::Detour(0x1402A5C50, 5, &NET_Config_Hk); // NET_OpenIP

    Utils::Hook(0x140101F06, UI_Init_Hk, HOOK_CALL).install()->quick();

    // CL_Disconnect but at the very end, whether we were connected or not
    //
    // On x86 we hook its end. x64 has no single place there that we could get
    // at, so we detour the whole function (it starts with 'push rdi; sub rsp,
    // 20h', six bytes) and run our tasks after it returns. This is a bit
    // roundabout, since we then have to guess what it did (see
    // CL_Disconnect_Hk()), but it seems to be the simplest option.
    //
    CL_Disconnect_Original = Utils::Hook::Detour(0x1400F8E00, 6, &CL_Disconnect_Hk);
  }
}
