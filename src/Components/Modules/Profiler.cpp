#include "Profiler.hpp"

// The main thread submits the render commands in 0x14001B610 and the
// backend thread (0x140050400) executes them, which also waits for the GPU
// and presents the frame. On x86 the game has empty profiling calls at
// each of these points ("wait cmds" and "end frame", for example), which
// is where we put our zones. x64 compiles those calls out (presumably they
// were empty in the retail build anyway), so we put the zones at the same
// points without them.
//
// Note that most of the waits go through 0x1400470D0, which processes
// worker commands until the predicate it's passed returns true. On x86 it
// also takes whether to block, while x64 always seems to.
//
namespace Components
{
  void Profiler::Wait(predicate_t predicate)
  {
    Utils::Hook::Call<void(predicate_t)>(0x1400470D0)(predicate);
  }

  // Note that on x86 we also have a zone for issuing the render commands,
  // which x64 inlines into R_EndFrame(). We keep the zones for its waits,
  // though.
  //
  void Profiler::WaitForWorkerCommands_Hk(predicate_t predicate)
  {
    ZoneScopedN("Wait for worker commands");

    Wait(predicate);
  }

  // Wait until the backend has taken the previous frame's commands.
  //
  void Profiler::WaitForBackend_Hk(predicate_t predicate)
  {
    ZoneScopedN("Wait for backend");

    Wait(predicate);
  }

  void Profiler::WaitForRenderCommands_Hk(predicate_t predicate)
  {
    ZoneScopedN("Wait for render commands");

    Wait(predicate);
  }

  void Profiler::WaitForRenderCommandsEvent_Hk()
  {
    ZoneScopedN("Wait for render commands");

    Utils::Hook::Call<void()>(0x1402096D0)();
  }

  void Profiler::ExecuteRenderCommands_Hk()
  {
    ZoneScopedN("Execute render commands");

    Utils::Hook::Call<void()>(0x140050320)();
  }

  // The predicate polls the end-of-frame query that the backend issues
  // after each Present(), so this is where we wait for the GPU.
  //
  void Profiler::WaitForGpu_Hk(predicate_t predicate)
  {
    ZoneScopedN("Wait for GPU");

    Wait(predicate);
  }

  // Note that x64 inlines Present() into its caller, which also checks the
  // flags to decide whether to present at all. So we wrap that instead.
  //
  void Profiler::SwapBuffers_Hk(int flags)
  {
    ZoneScopedN("Swap buffers");

    Utils::Hook::Call<void(int)>(0x14004FCF0)(flags);
  }

  void Profiler::WaitForMain_Hk(predicate_t predicate)
  {
    ZoneScopedN("Wait for main thread");

    Wait(predicate);
  }

  void Profiler::WaitForMainEvent_Hk()
  {
    ZoneScopedN("Wait for main thread");

    Utils::Hook::Call<void()>(0x140209770)();
  }

  Profiler::Profiler()
  {
#ifdef TRACY_ENABLE
    // Main thread.
    //
    Utils::Hook(0x14001B730, WaitForWorkerCommands_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x14001B828, WaitForBackend_Hk, HOOK_CALL).install()->quick();

    // Backend thread. Which of the two waits for work and for the main
    // thread it uses depends on a dvar, so we wrap both.
    //
    Utils::Hook(0x1400504C4, WaitForRenderCommands_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400504CB, WaitForRenderCommandsEvent_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400504EA, ExecuteRenderCommands_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x140050541, ExecuteRenderCommands_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400505A7, ExecuteRenderCommands_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400503DB, WaitForGpu_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400503E7, SwapBuffers_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x14001B8CD, SwapBuffers_Hk, HOOK_CALL).install()->quick(); // R_EndFrame
    Utils::Hook(0x140050515, WaitForMain_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x14005051C, WaitForMainEvent_Hk, HOOK_CALL).install()->quick();
#endif
  }
}
