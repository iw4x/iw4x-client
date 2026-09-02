#include "Frame.hpp"

#include "../Types.hpp"

namespace Controller
{
  std::ostream&
  operator<< (std::ostream& os, const input_frame& f)
  {
    os << f.device << ' ' << f.family << " #" << f.sequence;

    if (f.timing.consumed != timestamp {})
    {
      auto ms (std::chrono::duration_cast<milliseconds> (f.timing.latency ()));
      os << ' ' << ms.count () << "ms";
    }

    return os;
  }
}
