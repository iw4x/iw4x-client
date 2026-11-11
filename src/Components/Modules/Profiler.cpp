#include "Profiler.hpp"

// Render commands are submitted by the main thread in 0x509200 and
// executed by the backend thread (0x536F70), which also waits for the
// GPU and presents the frame. The game has empty profiling calls at
// each of these points ("wait cmds", "wait renderer", "exec cmds",
// "end frame", and "wait for main"), which we use for our zones.
//
// Note that most waits go through 0x52CA10, which processes worker
// commands until the predicate passed in esi returns true. We use
// Profiler_WaitStub() to set esi before calling it.
//
namespace Components
{
  void Profiler::IssueRenderCommands_Hk()
  {
    ZoneScopedN("Issue render commands");

    Utils::Hook::Call<void()>(0x509200)();
  }

  void Profiler::WaitForWorkerCommands_Hk(int block)
  {
    ZoneScopedN("Wait for worker commands");

    Profiler_WaitStub(reinterpret_cast<void*>(0x52CC60), block);
  }

  // Wait until the backend has taken the previous frame's commands.
  //
  void Profiler::WaitForBackend_Hk(int block)
  {
    ZoneScopedN("Wait for backend");

    Profiler_WaitStub(reinterpret_cast<void*>(0x44D050), block);
  }

  void Profiler::WaitForRenderCommands_Hk(int block)
  {
    ZoneScopedN("Wait for render commands");

    Profiler_WaitStub(reinterpret_cast<void*>(0x4662A0), block);
  }

  void Profiler::WaitForRenderCommandsEvent_Hk()
  {
    ZoneScopedN("Wait for render commands");

    Utils::Hook::Call<void()>(0x42ED10)();
  }

  void Profiler::ExecuteRenderCommands_Hk()
  {
    ZoneScopedN("Execute render commands");

    Utils::Hook::Call<void()>(0x536E90)();
  }

  // The predicate polls the end-of-frame query that the backend issues
  // after each Present(), so this is where we wait for the GPU.
  //
  void Profiler::WaitForGpu_Hk(int block)
  {
    ZoneScopedN("Wait for GPU");

    Profiler_WaitStub(reinterpret_cast<void*>(0x536A30), block);
  }

  void Profiler::SwapBuffers_Hk()
  {
    ZoneScopedN("Swap buffers");

    Utils::Hook::Call<void()>(0x536A80)();
  }

  void Profiler::WaitForMain_Hk(int block)
  {
    ZoneScopedN("Wait for main thread");

    Profiler_WaitStub(reinterpret_cast<void*>(0x4FFAE0), block);
  }

  void Profiler::WaitForMainEvent_Hk()
  {
    ZoneScopedN("Wait for main thread");

    Utils::Hook::Call<void()>(0x4D2300)();
  }

  Profiler::Profiler()
  {
#ifdef TRACY_ENABLE
    // Main thread.
    //
    Utils::Hook(0x50933B, IssueRenderCommands_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x509232, WaitForWorkerCommands_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x5092A3, WaitForBackend_Hk, HOOK_CALL).install()->quick();

    // Backend thread. Which of the two waits for work and for the main
    // thread it uses depends on a dvar, so we wrap both.
    //
    Utils::Hook(0x537001, WaitForRenderCommands_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x53700B, WaitForRenderCommandsEvent_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x537027, ExecuteRenderCommands_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x537088, ExecuteRenderCommands_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x5370EF, ExecuteRenderCommands_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x536F3A, WaitForGpu_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x536B00, SwapBuffers_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x53705D, WaitForMain_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x537067, WaitForMainEvent_Hk, HOOK_CALL).install()->quick();
#endif
  }
}
