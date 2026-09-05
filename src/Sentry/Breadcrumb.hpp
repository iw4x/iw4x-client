#pragma once

#include "Types.hpp"

namespace Sentry
{
  enum class trail : uint8_t
  {
    lifecycle,
    state,
    error,
    engine,
  };

  struct trail_kind
  {
    const char* type;
    const char* category;
  };

  const trail_kind&
  kind_of (trail) noexcept;

  void
  record (trail, sentry_level_t, const std::string& message);
}
