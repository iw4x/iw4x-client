#include <STDInclude.hpp>
#undef getch
#undef ungetch
#include <conio.h>

namespace Components
{
	bool Monitor::IsEnabled()
	{
		static std::optional<bool> flag;

		if (!flag.has_value())
		{
			flag.emplace(Flags::HasFlag("monitor"));
		}

		return flag.value();
	}

	int __stdcall Monitor::EntryPoint(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
	{
		Utils::Hook::Call<void()>(0x4D8220)();  // Dvar_Init
		Utils::Hook::Call<void()>(0x4D2280)();  // SL_Init
		Utils::Hook::Call<void()>(0x47F390)();  // Swap_Init
		Utils::Hook::Call<void()>(0x60AD10)();  // Com_InitDvars
		Utils::Hook::Call<void()>(0x420830)();  // Com_InitHunkMemory
		Utils::Hook::Call<void()>(0x4A62A0)();  // LargeLocalInit
		Utils::Hook::Call<void(unsigned int)>(0x502580)(static_cast<unsigned int>(__rdtsc())); // Netchan_Init
		Game::NET_Init();

		Utils::Time::Interval interval;
		while (!interval.elapsed(15s))
		{
			Utils::Hook::Call<void()>(0x49F0B0)(); // Com_ClientPacketEvent
			//Session::RunFrame();
			Node::RunFrame();
			ServerList::Frame();

			std::this_thread::sleep_for(10ms);
		}

		auto list = ServerList::GetList();
		if (!list)
		{
			printf("1 IW4x player=0|server=0 Returned list was null\n");
			return 1;
		}

		int servers = list->size();
		int players = 0;

		for (unsigned int i = 0; i < list->size(); ++i)
		{
			players += list->at(i).clients;
		}

		printf("0 IW4x player=%d|server=%d Servers successfully parsed\n", players, servers);

		Utils::Hook::Call<void()>(0x430630)();  // LargeLocalReset
		Utils::Hook::Call<void()>(0x4A0460)();  // Hunk_ClearTempMemory
		Utils::Hook::Call<void()>(0x4AB3A0)();  // Hunk_ClearTempMemoryHigh
		Utils::Hook::Call<void()>(0x4B3AD0)();  // SL_Shutdown
		Utils::Hook::Call<void()>(0x502C50)();  // Dvar_Shutdown

		if (*Game::ip_socket && *Game::ip_socket != INVALID_SOCKET)
		{
			closesocket(*Game::ip_socket);
		}

		WSACleanup();
		return 0;
	}

	Monitor::Monitor()
	{
		if (!Monitor::IsEnabled()) return;

		Utils::Hook(0x4513DA, Monitor::EntryPoint, HOOK_JUMP).install()->quick();
	}
}
