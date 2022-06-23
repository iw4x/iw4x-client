#include <STDInclude.hpp>

namespace Components
{
	void FrameTime::NetSleep(int msec)
	{
		if (msec < 0) msec = 0;

		fd_set fdr;
		FD_ZERO(&fdr);

		SOCKET highestfd = INVALID_SOCKET;
		if (*Game::ip_socket != INVALID_SOCKET)
		{
			FD_SET(*Game::ip_socket, &fdr);
			highestfd = *Game::ip_socket;
		}

		if (highestfd == INVALID_SOCKET)
		{
			// windows ain't happy when select is called without valid FDs
			std::this_thread::sleep_for(std::chrono::milliseconds(msec));
			return;
		}

		timeval timeout;
		timeout.tv_sec = msec / 1000;
		timeout.tv_usec = (msec % 1000) * 1000;

		int retval = select(highestfd + 1, &fdr, nullptr, nullptr, &timeout);

		if (retval == SOCKET_ERROR)
		{
			Logger::Warning(Game::CON_CHANNEL_SYSTEM, "Select() syscall failed: {}\n", Game::NET_ErrorString());
		}
		else if (retval > 0)
		{
			// process packets
			if (Dvar::Var(0x1AD7934).get<bool>()) // com_sv_running
			{
				Utils::Hook::Call<void()>(0x458160)();
			}
			else
			{
				Utils::Hook::Call<void()>(0x49F0B0)();
			}
		}
	}

	void FrameTime::SVFrameWaitFunc()
	{
		int sv_residualTime = *reinterpret_cast<int*>(0x2089E14);

		if (sv_residualTime < 50)
		{
			FrameTime::NetSleep(50 - sv_residualTime);
		}
	}

	void __declspec(naked) FrameTime::SVFrameWaitStub()
	{
		__asm
		{
			pushad
			call FrameTime::SVFrameWaitFunc
			popad

			push 4CD420h
			retn
		}
	}

	int FrameTime::ComTimeVal(int minMsec)
	{
		int timeVal = Game::Sys_Milliseconds() - *reinterpret_cast<uint32_t*>(0x1AD8F3C); // com_frameTime
		return (timeVal >= minMsec ? 0 : minMsec - timeVal);
	}

	uint32_t FrameTime::ComFrameWait(int minMsec)
	{
		int timeVal;

		do
		{
			timeVal = FrameTime::ComTimeVal(minMsec);
			FrameTime::NetSleep(timeVal < 1 ? 0 : timeVal - 1);
		} while (FrameTime::ComTimeVal(minMsec));

		uint32_t lastTime = *Game::com_frameTime;
		Utils::Hook::Call<void()>(0x43D140)(); // Com_EventLoop
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
			call FrameTime::ComFrameWait
			add esp, 4

			mov [esp + 20h], eax
			popad
			pop eax
			mov ecx, eax

			mov edx, 1AD7934h // com_sv_running
			cmp byte ptr[edx + 10h], 0

			push 47DDC1h
			retn
		}
	}

	FrameTime::FrameTime()
	{
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
