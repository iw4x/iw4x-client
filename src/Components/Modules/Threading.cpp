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
      auto r (::select(static_cast<int>(m + 1), &fs, nullptr, nullptr, &tv));

      // Note that x64 compiles out NET_ErrorString(), so we print the error
      // code instead of its name.
      //
      if (r == SOCKET_ERROR)
      {
        Logger::Warning(Game::CON_CHANNEL_SYSTEM, "WinAPI: select failed: {}\n", WSAGetLastError());
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

    // The server thread's frame, which we run where SV_Frame() would wake
    // the thread for it.
    //
    // Note that on x86 we jump into the thread's function at the frame and
    // return from it at the end of the frame. x64 has the same frame, so we
    // run it as the thread does instead.
    //
    void ServerThreadFrame()
    {
      // Set and clear the flag the game keeps while the server runs its
      // frame.
      //
      const auto SV_EnterFrame(reinterpret_cast<void (*)()>(0x1402387C0));
      const auto SV_LeaveFrame(reinterpret_cast<void (*)()>(0x1402386D0));

      auto& sv_time (*reinterpret_cast<int*>(0x14340CB00));
      auto& sv_nextFrameTime (*reinterpret_cast<int*>(0x1464FBD8C));
      auto& sv_packetEvents (*reinterpret_cast<bool*>(0x1464FBD90));

      // The time the last frame ended at, which the thread keeps from one
      // frame to the next.
      //
      static auto end (Game::Sys_Milliseconds());

      SV_EnterFrame();
      Utils::Hook::Call<void()>(0x14023A2B0)();

      if (auto d (Game::Sys_Milliseconds() - end); d > 0)
      {
        auto& n (*reinterpret_cast<int*>(0x1406CBD84));
        n = std::max(n - (d + 2) / 3, 0);
      }

      for (;;)
      {
        Utils::Hook::Call<void()>(0x140231BE0)();
        Utils::Hook::Call<void()>(0x14023BB30)();

        sv_time += 50;

        if (!Utils::Hook::Call<bool()>(0x14023C750)())
          Utils::Hook::Call<void(int)>(0x14019EA50)(sv_time); // G_RunFrame

        Utils::Hook::Call<void()>(0x14023B4B0)();

        end = Game::Sys_Milliseconds();

        if (sv_time - sv_nextFrameTime >= 0)
          break;

        SV_LeaveFrame();

        Utils::Hook::Call<void()>(0x1402096B0)();
        Utils::Hook::Call<void()>(0x140208FA0)();

        if (sv_packetEvents)
        {
          SV_EnterFrame();
          Game::Com_ServerPacketEvent();
          SV_LeaveFrame();
        }

        Utils::Hook::Call<void()>(0x140209050)();

        SV_EnterFrame();
      }

      *reinterpret_cast<int*>(0x1406CBD80) = 0;
      Utils::Hook::Call<void()>(0x1402090D0)();

      SV_LeaveFrame();
    }

    // Com_EventLoop() ends on releasing the buffer of the client's packets.
    // We then hand the server its own, which its thread would otherwise.
    //
    void Com_EventLoop_End(void* buffer)
    {
      Utils::Hook::Call<void(void*)>(0x14027CDA0)(buffer);
      Game::Com_ServerPacketEvent();
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
    // ... Actually (#2), the dedicated server path is another legacy with the 
    // same assumption. 
    //
    // Note that x64 has no ZoneBuilder, so the dedicated server is the only
    // exception here.
    //
    if (Dedicated::IsEnabled())
    {
      // Stop the engine from spawning its own server thread so we can maintain
      // control over the threading model ourselves.
      //
      Utils::Hook::Nop(0x1401F5658, 5);

      // Disable the blocking server wait so we can drive the cycle.
      //
      // Note that x64 also inlines it into SV_Frame(), where we jump over it.
      //
      Utils::Hook::Set<std::uint8_t>(0x14023BF10, 0xC3);
      Utils::Hook::Set<std::uint8_t>(0x14023ABAB, 0xEB);

      // This dvar sync logic is known to conflict with our manual threading
      // approach, so we'll just bypass it.
      //
      Utils::Hook::Set<std::uint8_t>(0x14028611E, 0xEB);

      Utils::Hook(0x14023AB77, FrameTime::ServerThreadFrame, HOOK_CALL).install()->quick();
      Utils::Hook(0x1401F3DB2, FrameTime::Com_EventLoop_End, HOOK_CALL).install()->quick();
    }

    // Hijack the appropriate waiter based on whether we are running a dedicated
    // server or a client.
    //
    // Note that x64 inlines the server's into SV_Frame() and the client's into
    // Com_Frame().
    //
    if (Dedicated::IsEnabled())
    {
      Utils::Hook(0x14023AA94, Threading_SVFrameWaitStub, HOOK_JUMP).install()->quick();
    }
    else
    {
      Utils::Hook(0x1401F4050, Threading_ComFrameWaitStub, HOOK_JUMP).install()->quick();
    }
  }
}

void Threading_SVFrameWaitFunc()
{
  Components::FrameTime::SVFrameWaitFunc();
}

int Threading_ComFrameWait(int m)
{
  return Components::FrameTime::ComFrameWait(m);
}
