#pragma once

extern "C"
{
  void Profiler_WaitStub(void* predicate, int block);
}

namespace Components
{
  class Profiler : public Component
  {
  public:
    Profiler();

  private:
    static void IssueRenderCommands_Hk();
    static void WaitForWorkerCommands_Hk(int block);
    static void WaitForBackend_Hk(int block);

    static void WaitForRenderCommands_Hk(int block);
    static void WaitForRenderCommandsEvent_Hk();
    static void ExecuteRenderCommands_Hk();
    static void WaitForGpu_Hk(int block);
    static void SwapBuffers_Hk();
    static void WaitForMain_Hk(int block);
    static void WaitForMainEvent_Hk();
  };
}
