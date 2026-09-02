#include "Clock.hpp"

#include "Types.hpp"

namespace Controller
{
  clock::duration
  clock::
  since_epoch () noexcept
  {
    static const time_point epoch (now ());
    return now () - epoch;
  }
}
