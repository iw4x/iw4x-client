#pragma once

extern "C"
{
  void Elevators_PM_TraceStub();

  void Elevators_PM_Trace(Game::trace_t* results);
}

namespace Components
{
  class Elevators : public Component
  {
  public:
    Elevators();

  private:
    friend void (::Elevators_PM_Trace)(Game::trace_t* results);

    enum ElevatorSettings { DISABLED, ENABLED, EASY };
    static Dvar::Var BG_Elevators;

    static int PM_CorrectAllSolid(Game::pmove_s* move, Game::pml_t* pml, Game::trace_t* trace);
    static void PM_Trace_Hk(Game::trace_t* results);
  };
}
