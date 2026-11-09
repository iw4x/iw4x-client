#pragma once

extern "C"
{
  void Elevators_PM_CorrectAllSolidStub();

  int Elevators_PM_CorrectAllSolid(Game::pmove_s* move, Game::pml_t* pml, Game::trace_t* trace);
}

namespace Components
{
  class Elevators : public Component
  {
  public:
    Elevators();

  private:
    friend int (::Elevators_PM_CorrectAllSolid)(Game::pmove_s* move, Game::pml_t* pml, Game::trace_t* trace);

    enum ElevatorSettings { DISABLED, ENABLED, EASY };
    static Dvar::Var BG_Elevators;

    static int PM_CorrectAllSolid(Game::pmove_s* move, Game::pml_t* pml, Game::trace_t* trace);
    static void PM_Trace_Hk(Game::pmove_s*, Game::trace_t*, const float*, const float*, const Game::Bounds*, int, int);
  };
}
