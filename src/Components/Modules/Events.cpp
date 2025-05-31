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

	Events::ClientCmdCallback Events::ClientCmdButtonsTasks_;
	Events::ClientCmdCallback Events::ClientKeyMoveTasks_;

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

		Utils::Hook::Call<void(int)>(0x4AA430)(clientNum); // ClientDisconnect
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

		Utils::Hook::Call<void(Game::client_s*)>(0x401950)(cl); // SV_UserinfoChanged
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

		Utils::Hook::Call<void()>(0x467CC0)(); // LiveSteam_Client_SteamDisconnect
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

		Utils::Hook::Call<void(unsigned char)>(0x421EE0)(sys); // Scr_ShutdownSystem
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

		Utils::Hook::Call<void()>(0x404CA0)(); // CL_InitOnceForAllClients
	}

	void Events::CL_CmdButtons(Game::usercmd_s* cmd)
	{
		for (const auto& func : ClientCmdButtonsTasks_)
		{
			func(cmd);
		}
	}

	void __declspec(naked) Events::CL_CmdButtons_Stub()
	{
		static const DWORD CL_CmdButtons_t = 0x5A6510;

		__asm
		{
			call CL_CmdButtons_t

			pushad

			push esi
			call CL_CmdButtons
			add esp, 0x4

			popad
			ret
		}
	}

	void __declspec(naked) Events::CL_Disconnect_Stub()
	{
		static const DWORD original = 0x40354D;

		__asm
		{
			// original code first
			je end

			push [esp + 4]
			call original
			add esp, 4

			// Once all is done, call our hook
			pushad
			push bx
			call CL_Disconnect_Hk
			pop bx
			popad

			end:
				retn
		}
	}
	
	void Events::CL_Disconnect_Hk(bool wasConnected)
	{
		CL_DisconnectedTask_.access([&](CLDisconnectCallback& tasks)
		{
			for (const auto& func : tasks)
			{
				func(wasConnected);
			}
		});
	}

	void Events::CL_KeyMove(Game::usercmd_s* cmd)
	{
		for (const auto& func : ClientKeyMoveTasks_)
		{
			func(cmd);
		}
	}

	void __declspec(naked) Events::CL_KeyMove_Stub()
	{
		static const DWORD CL_KeyMove_t = 0x5A5F40;

		__asm
		{
			call CL_KeyMove_t

			pushad

			push esi
			call CL_KeyMove
			add esp, 0x4

			popad
			ret
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

		Utils::Hook::Call<void()>(0x474320)(); // SV_InitGameMode
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

		Utils::Hook::Call<void()>(0x60AD10)(); // Com_InitDvars
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

	void Events::NetworkStart()
	{
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
		Utils::Hook::Call<void(int)>(0x4A57D0)(localClientNum); // CAll UI_INIT

		UIInitTasks_.access([](Callback& tasks)
		{
			for (const auto& func : tasks)
			{
				func();
			}
		});
	}

	__declspec(naked) void Events::NET_OpenSocks_Hk()
	{
		__asm
		{
			mov eax, 64D900h
			call eax
			jmp NetworkStart
		}
	}

	Events::Events()
	{
		Utils::Hook(0x625235, ClientDisconnect_Hk, HOOK_CALL).install()->quick(); // SV_FreeClient

		Utils::Hook(0x4612BD, SV_UserinfoChanged_Hk, HOOK_CALL).install()->quick(); // SV_DirectConnect

		Utils::Hook(0x403582, SteamDisconnect_Hk, HOOK_CALL).install()->quick(); // CL_Disconnect

		Utils::Hook(0x47548B, Scr_ShutdownSystem_Hk, HOOK_CALL).install()->quick(); // G_LoadGame
		Utils::Hook(0x4D06BA, Scr_ShutdownSystem_Hk, HOOK_CALL).install()->quick(); // G_ShutdownGame

		Utils::Hook(0x60BE5B, CL_InitOnceForAllClients_HK, HOOK_CALL).install()->quick(); // Com_Init_Try_Block_Function

		Utils::Hook(0x5A6D84, CL_CmdButtons_Stub, HOOK_CALL).install()->quick();

		Utils::Hook(0x5A6D8B, CL_KeyMove_Stub, HOOK_CALL).install()->quick();

		Utils::Hook(0x4236A5, CL_InitCGame_Hk, HOOK_CALL).install()->quick(); // CL_InitCGame

		Utils::Hook(0x60BB3A, Com_InitDvars_Hk, HOOK_CALL).install()->quick(); // Com_Init_Try_Block_Function

		Utils::Hook(0x4D3665, SV_Init_Hk, HOOK_CALL).install()->quick(); // SV_Init

		Utils::Hook(0x4FD4D4, NET_OpenSocks_Hk, HOOK_JUMP).install()->quick(); // NET_OpenIP

		Utils::Hook(0x4B5422, UI_Init_Hk, HOOK_CALL).install()->quick();

		Utils::Hook(0x403547, CL_Disconnect_Stub, HOOK_JUMP).install()->quick(); // CL_Disconnect but at the very end, whether we were connected or not
	}
}
