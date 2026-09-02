#pragma once

#include "Types.hpp"

#ifndef IW4X_CONTROLLER_TRACE
#  define IW4X_CONTROLLER_TRACE 0
#endif

#define CONTROLLER_ZONE(name) do {} while (false)

#define CONTROLLER_FRAME_MARK() do {} while (false)

namespace Controller
{
  namespace trace
  {
    inline constexpr bool enabled {IW4X_CONTROLLER_TRACE != 0};
  }
}
