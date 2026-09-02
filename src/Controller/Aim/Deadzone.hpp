#pragma once

#include "../Types.hpp"

#include "Types.hpp"
#include "../Sample/Axis.hpp"

namespace Controller
{
  namespace aim
  {
    struct deadzone_params
    {
      magnitude inner {0.0f};
      magnitude outer {0.0f};
      magnitude anti {0.0f};
    };

    bool
    validate (const deadzone_params&, std::string& why) noexcept;

    stick_vector
    apply (const deadzone_params&, stick_vector) noexcept;
  }
}
