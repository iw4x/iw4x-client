#pragma once

#include "../Types.hpp"

namespace Controller
{
  namespace aim
  {
    enum class curve_kind : uint8_t
    {
      linear,
      power,
    };

    const char*
    to_string (curve_kind) noexcept;

    struct response_curve
    {
      curve_kind kind {curve_kind::linear};
      float exponent {1.0f};
    };

    bool
    validate (const response_curve&, std::string& why) noexcept;

    float
    evaluate (const response_curve&, float t) noexcept;
  }
}
