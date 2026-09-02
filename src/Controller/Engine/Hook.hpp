#pragma once

#include "../Types.hpp"

namespace Controller
{
  class runtime;
}

namespace Controller
{
  namespace engine
  {
    void
    install (runtime&);

    void
    install_protocol ();

    void
    note_mouse_move (int dx, int dy) noexcept;
  }
}
