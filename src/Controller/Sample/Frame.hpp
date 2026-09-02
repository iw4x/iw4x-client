#pragma once

#include "../Types.hpp"

#include "../Clock.hpp"
#include "Sample.hpp"
#include "../Device/Id.hpp"
#include "../Device/Identity.hpp"

namespace Controller
{
  struct input_frame
  {
    device_id device {};
    Controller::family family {Controller::family::unknown};
    connection link {connection::unknown};
    uint64_t sequence {0};
    latency_span timing {};
    canonical_sample state {};
  };

  std::ostream&
  operator<< (std::ostream&, const input_frame&);
}
