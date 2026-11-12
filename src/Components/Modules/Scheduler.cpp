#include "Scheduler.hpp"

constexpr bool COND_CONTINUE = false;
constexpr bool COND_END = true;

namespace Components
{
  std::jthread Scheduler::Thread;
  volatile bool Scheduler::Kill = false;
  Scheduler::TaskPipeline Scheduler::Pipelines[static_cast<std::underlying_type_t<Pipeline>>(Pipeline::COUNT)];

  void Scheduler::TaskPipeline::add(Task&& task)
  {
    newCallbacks_.access([&task](taskList& tasks)
    {
      tasks.emplace_back(std::move(task));
    });
  }

  void Scheduler::TaskPipeline::execute()
  {
    callbacks_.access([&](taskList& tasks)
    {
      this->mergeCallbacks();

      for (auto i = tasks.begin(); i != tasks.end();)
      {
        const auto now = std::chrono::high_resolution_clock::now();
        const auto diff = now - i->lastCall;

        if (diff < i->interval)
        {
          ++i;
          continue;
        }

        i->lastCall = now;

        const auto res = i->handler();
        if (res == COND_END)
        {
          i = tasks.erase(i);
        }
        else
        {
          ++i;
        }
      }
    });
  }

  void Scheduler::TaskPipeline::mergeCallbacks()
  {
    callbacks_.access([&](taskList& tasks)
    {
      newCallbacks_.access([&](taskList& new_tasks)
      {
        tasks.insert(tasks.end(), std::move_iterator<taskList::iterator>(new_tasks.begin()), std::move_iterator<taskList::iterator>(new_tasks.end()));
        new_tasks = {};
      });
    });
  }

  void Scheduler::Execute(Pipeline type)
  {
    assert(type < Pipeline::COUNT);
    const auto index = static_cast<std::underlying_type_t<Pipeline>>(type);
    Pipelines[index].execute();
  }

  void Scheduler::ScrPlace_EndFrame_Hk()
  {
    Utils::Hook::Call<void()>(0x1400F2B60)();

    ZoneScopedN("Renderer pipeline");

    Execute(Pipeline::RENDERER);
  }

  void Scheduler::ServerFrame_Hk()
  {
    Utils::Hook::Call<void()>(0x140169310)();

    ZoneScopedN("Server pipeline");

    Execute(Pipeline::SERVER);
  }

  void Scheduler::ClientFrame_Hk(const int localClientNum)
  {
    Utils::Hook::Call<void(int)>(0x1400F8190)(localClientNum);

    ZoneScopedN("Client pipeline");

    Execute(Pipeline::CLIENT);
  }

  void Scheduler::MainFrame_Hk()
  {
    Utils::Hook::Call<void()>(0x1401F3E80)();

    {
      ZoneScopedN("Main pipeline");

      Execute(Pipeline::MAIN);
    }

    // Com_Frame() calls MainFrame_Hk() exactly once per iteration, which
    // gives us one frame mark per game frame.
    //
    FrameMark;
  }

  void Scheduler::SysSetBlockSystemHotkeys_Hk(int block)
  {
    {
      ZoneScopedN("Quit pipeline");

      Execute(Pipeline::QUIT);
    }

    Utils::Hook::Call<void(int)>(0x1402A99B0)(block);
  }

  void Scheduler::Schedule(const std::function<bool()>& callback, const Pipeline type,
    const std::chrono::milliseconds delay)
  {
    assert(type < Pipeline::COUNT);

    Task task;
    task.handler = callback;
    task.interval = delay;
    task.lastCall = std::chrono::high_resolution_clock::now();

    const auto index = static_cast<std::underlying_type_t<Pipeline>>(type);
    Pipelines[index].add(std::move(task));
  }

  void Scheduler::Loop(const std::function<void()>& callback, const Pipeline type,
    const std::chrono::milliseconds delay)
  {
    Schedule([callback]
    {
      callback();
      return COND_CONTINUE;
    }, type, delay);
  }

  void Scheduler::Once(const std::function<void()>& callback, const Pipeline type,
    const std::chrono::milliseconds delay)
  {
    Schedule([callback]
    {
      callback();
      return COND_END;
    }, type, delay);
  }

  void Scheduler::OnGameInitialized(const std::function<void()>& callback, const Pipeline type,
    const std::chrono::milliseconds delay)
  {
    Schedule([=]
    {
      if (Game::Sys_IsDatabaseReady2())
      {
        Once(callback, type, delay);
        return COND_END;
      }

      return COND_CONTINUE;
    }, Pipeline::MAIN); // Once Com_Frame_Try_Block_Function is called we know the game is 'ready'
  }

  void Scheduler::OnGameShutdown(const std::function<void()>& callback)
  {
    Schedule([callback]
    {
      callback();
      return COND_END;
    }, Pipeline::QUIT, 0ms);
  }

  Scheduler::Scheduler()
  {
    Thread = Utils::Thread::CreateNamedThread("Async Scheduler", []
    {
      while (!Kill)
      {
        {
          ZoneScopedN("Async pipeline");

          Execute(Pipeline::ASYNC);
        }

        std::this_thread::sleep_for(10ms);
      }
    });

    // SCR_UpdateFrame, which x64 inlines.
    //
    Utils::Hook(0x140101D11, ScrPlace_EndFrame_Hk, HOOK_CALL).install()->quick();

    // Hook G_Glass_Update so we may fix TLS issues
    Utils::Hook(0x14019EFB4, ServerFrame_Hk, HOOK_CALL).install()->quick();

    // CL_CheckForResend. Note that x64 inlines CL_CheckTimeout, which x86
    // hooks and which comes right after it.
    //
    Utils::Hook(0x1400FAC24, ClientFrame_Hk, HOOK_CALL).install()->quick();

    // Com_Frame_Try_Block_Function
    Utils::Hook(0x1401F3E04, MainFrame_Hk, HOOK_CALL).install()->quick();

    // Sys_Quit
    Utils::Hook(0x1402A3DB1, SysSetBlockSystemHotkeys_Hk, HOOK_CALL).install()->quick();
  }
}
