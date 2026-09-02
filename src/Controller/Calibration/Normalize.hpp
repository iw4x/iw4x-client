#pragma once

#include "../Types.hpp"

#include "Profile.hpp"
#include "../Sample/Sample.hpp"

namespace Controller
{
  namespace calibration
  {
    void
    apply (const profile&, const raw_sample&, canonical_sample&) noexcept;
  }
}
