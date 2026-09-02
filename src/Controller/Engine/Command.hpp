#pragma once

#include "../Types.hpp"

#include "../Context.hpp"

namespace Controller
{
  class runtime;
}

namespace Controller
{
  namespace engine
  {
    void
    register_commands (const context&, runtime&);
  }
}
