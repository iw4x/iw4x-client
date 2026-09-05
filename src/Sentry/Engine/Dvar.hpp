#pragma once

#include "../Types.hpp"

#include "../Options.hpp"

namespace Sentry
{
  namespace engine
  {
    struct status_dvar
    {
      const char* name;
      const char* description;
      std::string (*read) (const options&);
    };

    std::span<const status_dvar>
    status_dvars () noexcept;

    void
    publish_status (const options&);
  }
}
