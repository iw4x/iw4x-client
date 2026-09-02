#pragma once

#include "../Types.hpp"

#include "Profile.hpp"

namespace Controller
{
  namespace calibration
  {
    bool
    validate (const profile&, std::string& why) noexcept;
  }
}
