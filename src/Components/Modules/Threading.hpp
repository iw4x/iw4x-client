#pragma once

extern "C"
{
  void Threading_FrameEpilogueStub();
  void Threading_PacketEventStub();
  void Threading_SVFrameWaitStub();
  void Threading_ComFrameWaitStub();

  void Threading_SVFrameWaitFunc();
  int Threading_ComFrameWait(int m);
}

namespace Components
{
  class Threading : public Component
  {
  public:
    Threading();
  };
}
