#include <STDInclude.hpp>
#include "Threading.hpp"

namespace Components
{
	namespace FrameTime
	{
		void NetSleep(int ms)
		{
			if (ms < 0) ms = 0;

			fd_set fdr;
			FD_ZERO(&fdr);

			auto maxfd = INVALID_SOCKET;
			if (*Game::ip_socket != INVALID_SOCKET)
			{
				FD_SET(*Game::ip_socket, &fdr);
				maxfd = *Game::ip_socket;
			}

			if (maxfd == INVALID_SOCKET)
			{
				// On Windows, select fails if no sockets
				Game::Sys_Sleep(ms);
				return;
			}

			timeval tv = { ms / 1000, (ms % 1000) * 1000 };
			const auto result = ::select(maxfd + 1, &fdr, nullptr, nullptr, &tv);

			if (result == SOCKET_ERROR)
			{
				Logger::Warning(Game::CON_CHANNEL_SYSTEM, "WinAPI: select failed: {}\n", Game::NET_ErrorString());
				return;
			}

			if (result > 0)
			{
				if (Dedicated::IsRunning())
				{
					Game::Com_ServerPacketEvent();
				}
				else
				{
					Game::Com_ClientPacketEvent();
				}
			}
		}

		void SVFrameWaitFunc()
		{
			if (*Game::sv_timeResidual < 50)
			{
				NetSleep(50 - *Game::sv_timeResidual);
			}
		}

		void __declspec(naked) SVFrameWaitStub()
		{
			__asm
			{
				pushad
				call SVFrameWaitFunc
				popad

				push 4CD420h
				ret
			}
		}

		int ComTimeVal(int minMsec)
		{
			const auto timeVal = Game::Sys_Milliseconds() - *Game::com_frameTime;
			return (timeVal >= minMsec ? 0 : minMsec - timeVal);
		}

		int ComFrameWait(int minMsec)
		{
			do
			{
				const auto timeVal = ComTimeVal(minMsec);
				NetSleep(timeVal < 1 ? 0 : timeVal - 1);
			} while (ComTimeVal(minMsec));

			const auto lastTime = *Game::com_frameTime;
			Game::Com_EventLoop();
			*Game::com_frameTime = Game::Sys_Milliseconds();

			return *Game::com_frameTime - lastTime;
		}

		void __declspec(naked) ComFrameWaitStub()
		{
			__asm
			{
				push ecx
				pushad

				push edi
				call ComFrameWait
				add esp, 4

				mov [esp + 20h], eax
				popad
				pop eax
				mov ecx, eax

				mov edx, 1AD7934h // com_sv_running
				cmp byte ptr [edx + 10h], 0

				push 47DDC1h
				ret
			}
		}
	}

	__declspec(naked) void Threading::FrameEpilogueStub()
	{
		__asm
		{
			pop edi
			pop esi
			pop ebx
			mov esp, ebp
			pop ebp
			retn
		}
	}

	__declspec(naked) void Threading::PacketEventStub()
	{
		__asm
		{
			mov eax, 49F0B0h
			call eax

			mov eax, 458160h
			jmp eax
		}
	}

	Threading::Threading()
	{
		// remove starting of server thread from Com_Init_Try_Block_Function
		Utils::Hook::Nop(0x60BEC0, 5);

		// make server thread function jump to per-frame stuff
		Utils::Hook(0x627049, 0x6271CE, HOOK_JUMP).install()->quick();

		// make SV_WaitServer insta-return
		Utils::Hook::Set<std::uint8_t>(0x4256F0, 0xC3);

		// dvar setting function, unknown stuff related to server thread sync
		Utils::Hook::Set<std::uint8_t>(0x647781, 0xEB);

		Utils::Hook(0x627695, 0x627040, HOOK_CALL).install()->quick();
		Utils::Hook(0x43D1C7, PacketEventStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x6272E3, FrameEpilogueStub, HOOK_JUMP).install()->quick();

		if (Dedicated::IsEnabled())
		{
			Utils::Hook(0x4BAAAD, FrameTime::SVFrameWaitStub, HOOK_CALL).install()->quick();
		}
		else
		{
			Utils::Hook(0x47DD80, FrameTime::ComFrameWaitStub, HOOK_JUMP).install()->quick();
		}
	}
}
