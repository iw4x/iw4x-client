#include "Threading.hpp"

namespace Components
{
	namespace FrameTime
	{
		// Accumulate the fractional milliseconds lost to integer division (e.g., 85
		// FPS is 11.76ms, but engine gives us 11ms). Once it crosses the 1ms
		// threshold, we'll nudge the next frame's wait.
		//
		static double res (0.0);

		void NetSleep(int n)
		{
			if (n < 0)
				n = 0;

			fd_set fs;
			FD_ZERO(&fs);

			// Check if there is actually a socket to listen on before we bother with
			// select().
			//
			auto s (*Game::ip_socket);
			auto m (INVALID_SOCKET);

			if (s != INVALID_SOCKET)
			{
				FD_SET(s, &fs);
				m = s;
			}

			// Windows select() is notoriously finicky and fails if there are no
			// sockets in the set. If we are ~~maiden~~socket-less, just fall back to
			// a vanilla system sleep.
			//
			if (m == INVALID_SOCKET)
			{
				Game::Sys_Sleep(n);
				return;
			}

			timeval tv {n / 1000, (n % 1000) * 1000};
			auto r (::select(m + 1, &fs, nullptr, nullptr, &tv));

			if (r == SOCKET_ERROR)
			{
				Logger::Warning(Game::CON_CHANNEL_SYSTEM, "WinAPI: select failed: {}\n", Game::NET_ErrorString());
				return;
			}

			// If we caught some activity, process packets immediately to stay
			// responsive instead of waiting for the next cycle.
			//
			if (r > 0)
			{
				if (Dedicated::IsRunning())
					Game::Com_ServerPacketEvent();
				else
					Game::Com_ClientPacketEvent();
			}
		}

		void SVFrameWaitFunc()
		{
			// Yield if we still have a significant chunk of time left in the server's
			// time residual.
			//
			if (*Game::sv_timeResidual < 50)
				NetSleep(50 - *Game::sv_timeResidual);
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

		int ComTimeVal(int m)
		{
			auto d (Game::Sys_Milliseconds() - *Game::com_frameTime);
			return (d >= m ? 0 : m - d);
		}

		int ComFrameWait(int m)
		{
			static Dvar::Var v ("com_maxfps");
			auto f (v.get<int>());
			auto t (0);

			// 'm' is usually 1000/fps. At 85 FPS this is 11ms which is actually ~90
			// FPS. We recalculate using floats and track the drift so we don't
			// consistently run too fast.
			//
			if (f > 0)
			{
				// Use integer division for the base. That is, for exact caps like 250
				// (1000/250 = 4), we never want to accidentally truncate to 3 due to
				// floating point.
				//
				auto b (1000 / f);
				auto i (1000.0 / f);

				res += (i - b);

				// Swallow the accumulated drift once it hits 1ms. We use a while-loop
				// here just in case some massive lag spike means we have multiple
				// milliseconds to "repay" to the clock.
				//
				while (res >= 1.0f)
				{
					b += 1;
					res -= 1.0f;
				}
				t = *Game::com_frameTime + b;
			}
			else
			{
				// For uncapped FPS, we don't care about the drift and just want to get
				// back to work as soon as possible. (wait time is effectively 0)
				//
				t = *Game::com_frameTime + m;
			}

			// Select/Sleep resolution is often too coarse (>1ms) to hit high targets
			// like 250 FPS reliably. To solve this, we sleep for the bulk of the time
			// but spin for the final 2ms to wake up exactly when we need to.
			//
			for (;;)
			{
				auto n (Game::Sys_Milliseconds());
				auto r (t - n);

				if (r <= 0)
					break;

				// Give up the timeslice if we have a comfortable margin, otherwise
				// spin-wait with NetSleep(0) to keep processing packets while we burn
				// the remaining time.
				//
				if (r > 2)
					NetSleep(r - 2);
				else
					NetSleep(0);
			}

			auto l (*Game::com_frameTime);
			Game::Com_EventLoop();
			*Game::com_frameTime = Game::Sys_Milliseconds();

			return (*Game::com_frameTime - l);
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

				mov edx, 1AD7934h // sv_running
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
		// Force the OS scheduler to 1ms so our hybrid sleep/spin logic doesn't get
		// stuck in 15ms "quantum" naps.
		//
		timeBeginPeriod(1);

		// Keep the engine threading model intact.
		//
		// It may be tempting to make the server frame run through the main frame.
		// And yes, that does make the control flow look simpler, but only by
		// removing an engine invariant that the rest of the code still assumes: the
		// server side is driven by an engine-owned thread.
		//
		// Once that boundary is removed, server code begins to execute as if it
		// were main-frame code. Some of it will appear to work, which is what makes
		// this kind of change dangerous. That is, failures are no longer confined
		// to the wait path and can instead show up later as state being touched
		// from the wrong side of the engine.
		//
		// So while this module may change how the existing threads wait, it must
		// not decide that the engine no longer gets a server thread.
		//
		// ... Actually, ZoneBuilder is the one known exception. It currently
		// depends on the collapsed path, so we preserve that behavior there only.
		// Treat this as a compatibility exception.
		//
		if (ZoneBuilder::IsEnabled() || Dedicated::IsEnabled())
		{
			// Stop the engine from spawning its own server thread so we can maintain
			// control over the threading model ourselves.
			//
			Utils::Hook::Nop(0x60BEC0, 5);
			Utils::Hook(0x627049, 0x6271CE, HOOK_JUMP).install()->quick();

			// Disable the blocking server wait so we can drive the cycle.
			//
			Utils::Hook::Set<std::uint8_t>(0x4256F0, 0xC3);

			// This dvar sync logic is known to conflict with our manual threading
			// approach, so we'll just bypass it.
			//
			Utils::Hook::Set<std::uint8_t>(0x647781, 0xEB);

			Utils::Hook(0x627695, 0x627040, HOOK_CALL).install()->quick();
			Utils::Hook(0x43D1C7, PacketEventStub, HOOK_JUMP).install()->quick();
			Utils::Hook(0x6272E3, FrameEpilogueStub, HOOK_JUMP).install()->quick();
		}

		// Hijack the appropriate waiter based on whether we are running a dedicated
		// server or a client.
		//
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
