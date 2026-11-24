#pragma once

namespace Components
{
  class Profiler : public Component
  {
  public:
    Profiler();

  private:
    using predicate_t = int (*)();

    static void Wait(predicate_t predicate);

    static void WaitForWorkerCommands_Hk(predicate_t predicate);
    static void WaitForBackend_Hk(predicate_t predicate);

    static void WaitForRenderCommands_Hk(predicate_t predicate);
    static void WaitForRenderCommandsEvent_Hk();
    static void ExecuteRenderCommands_Hk();
    static void WaitForGpu_Hk(predicate_t predicate);
    static void SwapBuffers_Hk(int flags);
    static void WaitForMain_Hk(predicate_t predicate);
    static void WaitForMainEvent_Hk();
  };
}
