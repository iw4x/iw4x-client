#pragma once

#include "Types.hpp"

namespace Controller
{
  struct clock
  {
    using base       = std::chrono::steady_clock;
    using time_point = base::time_point;
    using duration   = base::duration;

    static_assert (base::is_steady,
                   "the subsystem clock must be monotonic; latency spans "
                   "assume acquisition never appears later than consumption");

    static time_point
    now () noexcept {return base::now ();}

    static duration
    since_epoch () noexcept;
  };

  using timestamp = clock::time_point;

  using seconds      = std::chrono::duration<float>;
  using milliseconds = std::chrono::duration<float, std::milli>;

  struct latency_span
  {
    timestamp acquired {};
    timestamp consumed {};

    clock::duration
    latency () const noexcept {return consumed - acquired;}
  };
}
