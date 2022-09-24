#include <STDInclude.hpp>

namespace Components
{
	void FrameTime::NetSleep(int mSec)
	{
		if (mSec < 0) mSec = 0;

		fd_set fdr;
		FD_ZERO(&fdr);

		auto highestfd = INVALID_SOCKET;
		if (*Game::ip_socket != INVALID_SOCKET)
		{
			FD_SET(*Game::ip_socket, &fdr);
			highestfd = *Game::ip_socket;
		}

		if (highestfd == INVALID_SOCKET)
		{
			// windows ain't happy when select is called without valid FDs
			std::this_thread::sleep_for(std::chrono::milliseconds(mSec));
			return;
		}

		timeval timeout;
		timeout.tv_sec = mSec / 1000;
		timeout.tv_usec = (mSec % 1000) * 1000;

		const auto retVal = select(highestfd + 1, &fdr, nullptr, nullptr, &timeout);

		if (retVal == SOCKET_ERROR)
		{
			Logger::Warning(Game::CON_CHANNEL_SYSTEM, "Select() syscall failed: {}\n", Game::NET_ErrorString());
		}
		else if (retVal > 0)
		{
			if ((*Game::com_sv_running)->current.enabled)
			{
				Game::Com_ServerPacketEvent();
			}
			else
			{
				Game::Com_ClientPacketEvent();
			}
		}
	}

	void FrameTime::SVFrameWaitFunc()
	{
		int sv_residualTime = *reinterpret_cast<int*>(0x2089E14);

		if (sv_residualTime < 50)
		{
			NetSleep(50 - sv_residualTime);
		}
	}

	void __declspec(naked) FrameTime::SVFrameWaitStub()
	{
		__asm
		{
			pushad
			call SVFrameWaitFunc
			popad

			push 4CD420h
			retn
		}
	}

	int FrameTime::ComTimeVal(int minMsec)
	{
		const auto timeVal = Game::Sys_Milliseconds() - *Game::com_frameTime;
		return (timeVal >= minMsec ? 0 : minMsec - timeVal);
	}

	int FrameTime::ComFrameWait(int minMsec)
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

	void __declspec(naked) FrameTime::ComFrameWaitStub()
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
			retn
		}
	}

	FrameTime::FrameTime()
	{
		if (Dedicated::IsEnabled())
		{
			Utils::Hook(0x4BAAAD, SVFrameWaitStub, HOOK_CALL).install()->quick();
		}
		else
		{
			Utils::Hook(0x47DD80, ComFrameWaitStub, HOOK_JUMP).install()->quick();
		}
	}
}
